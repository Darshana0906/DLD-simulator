#include "CircuitAdapter.h"
#include "../CircuitScene.h"
#include "../elements/Input.h"
#include "../elements/Switch.h"
#include "../elements/GND.h"
#include "../elements/Const1.h"
#include "../elements/LED.h"
#include "../elements/Point.h"
#include "../elements/Wire.h"
#include "../gates/Gate.h"
#include "../gates/AndGate.h"
#include "../gates/OrGate.h"
#include "../gates/NotGate.h"
#include "../gates/NandGate.h"
#include "../gates/NorGate.h"
#include "../gates/XorGate.h"
#include "../gates/XnorGate.h"
#include <QGraphicsItem>

CircuitAdapter::CircuitAdapter(CircuitScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene) {
    if (m_scene) {
        syncSceneItems();
    }
}

void CircuitAdapter::setScene(CircuitScene *scene) {
    m_scene = scene;
    if (m_scene) {
        syncSceneItems();
    }
}

// =========================================================================
// MEMBER 1: QGraphicsItem* <---> NodeID (int) Mapping
// =========================================================================

int CircuitAdapter::registerItem(QGraphicsItem *item, int preferredId) {
    if (!item) return -1;

    auto it = m_itemToId.find(item);
    if (it != m_itemToId.end()) {
        return it->second;
    }

    int id = (preferredId >= 0) ? preferredId : m_nextId++;
    if (id >= m_nextId) {
        m_nextId = id + 1;
    }

    m_itemToId[item] = id;
    m_idToItem[id] = item;

    // Automatically instantiate corresponding Node based on item type
    if (GND *gnd = dynamic_cast<GND*>(item)) {
        m_nodes[id] = std::make_unique<InputNode>(id, LogicState::LOW, NodeType::INPUT_GND);
    } else if (Const1 *c1 = dynamic_cast<Const1*>(item)) {
        m_nodes[id] = std::make_unique<InputNode>(id, LogicState::HIGH, NodeType::INPUT_CONST1);
    } else if (Switch *sw = dynamic_cast<Switch*>(item)) {
        LogicState initVal = sw->getValue() ? LogicState::HIGH : LogicState::LOW;
        m_nodes[id] = std::make_unique<InputNode>(id, initVal, NodeType::INPUT_SWITCH);

        // Bind user click callback so Switch automatically triggers the adapter
        sw->setOnToggleCallback([this](Switch *s, bool val) {
            onSwitchToggled(s, val);
        });
    } else if (Gate *gate = dynamic_cast<AndGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_AND, gate->getInputPins().size());
    } else if (Gate *gate = dynamic_cast<OrGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_OR, gate->getInputPins().size());
    } else if (dynamic_cast<NotGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_NOT, 1);
    } else if (Gate *gate = dynamic_cast<NandGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_NAND, gate->getInputPins().size());
    } else if (Gate *gate = dynamic_cast<NorGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_NOR, gate->getInputPins().size());
    } else if (Gate *gate = dynamic_cast<XorGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_XOR, gate->getInputPins().size());
    } else if (Gate *gate = dynamic_cast<XnorGate*>(item)) {
        m_nodes[id] = std::make_unique<GateNode>(id, NodeType::GATE_XNOR, gate->getInputPins().size());
    } else if (dynamic_cast<LED*>(item)) {
        m_nodes[id] = std::make_unique<OutputNode>(id);
    }

    m_dag.addNode(id);
    return id;
}

int CircuitAdapter::getNodeId(QGraphicsItem *item) const {
    auto it = m_itemToId.find(item);
    if (it != m_itemToId.end()) {
        return it->second;
    }
    return -1;
}

QGraphicsItem* CircuitAdapter::getItem(int id) const {
    auto it = m_idToItem.find(id);
    if (it != m_idToItem.end()) {
        return it->second;
    }
    return nullptr;
}

void CircuitAdapter::unregisterItem(QGraphicsItem *item) {
    if (!item) return;

    auto it = m_itemToId.find(item);
    if (it != m_itemToId.end()) {
        int id = it->second;

        // Clean up edges and wires connected to this node
        std::vector<Wire*> wiresToDisconnect;
        for (auto &pair : m_wireToEdge) {
            const Edge &edge = pair.second;
            if (edge.sourceNodeId == id || edge.targetNodeId == id) {
                wiresToDisconnect.push_back(pair.first);
                // Reset input pin on target node if it wasn't the one deleted
                if (edge.targetNodeId != id) {
                    if (Node *targetNode = getNode(edge.targetNodeId)) {
                        targetNode->setInput(static_cast<size_t>(edge.targetPinIdx), LogicState::UNDEFINED);
                    }
                }
            }
        }

        for (Wire *w : wiresToDisconnect) {
            if (w) {
                w->setLogicValue(-1);
            }
            m_wireToEdge.erase(w);
        }

        m_edges.erase(
            std::remove_if(m_edges.begin(), m_edges.end(), [id](const Edge &e) {
                return e.sourceNodeId == id || e.targetNodeId == id;
            }),
            m_edges.end()
        );

        m_dag.removeNode(id);
        m_idToItem.erase(id);
        m_nodes.erase(id);
        m_itemToId.erase(it);

        emit simulationInvalidated();
    }
}

bool CircuitAdapter::hasItem(QGraphicsItem *item) const {
    return m_itemToId.find(item) != m_itemToId.end();
}

Node* CircuitAdapter::getNode(int id) const {
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return it->second.get();
    }
    return nullptr;
}

GateNode* CircuitAdapter::getGateNode(int id) const {
    return getNodeAs<GateNode>(id);
}

GateNode* CircuitAdapter::getGateNode(Gate *gateItem) const {
    int id = getNodeId(gateItem);
    return (id >= 0) ? getGateNode(id) : nullptr;
}

OutputNode* CircuitAdapter::getOutputNode(int id) const {
    return getNodeAs<OutputNode>(id);
}

OutputNode* CircuitAdapter::getOutputNode(LED *ledItem) const {
    int id = getNodeId(ledItem);
    return (id >= 0) ? getOutputNode(id) : nullptr;
}

bool CircuitAdapter::getPinInfo(Point *pin, int &nodeId, bool &isOutput, int &pinIndex) const {
    if (!pin) return false;
    QGraphicsItem *owner = pin->parentItem() ? pin->parentItem() : pin->topLevelItem();
    if (!owner) return false;

    nodeId = getNodeId(owner);
    if (nodeId < 0) return false;

    if (Gate *gate = dynamic_cast<Gate*>(owner)) {
        if (pin == gate->getOutputPin()) {
            isOutput = true;
            pinIndex = 0;
            return true;
        }
        const auto &inPins = gate->getInputPins();
        for (int i = 0; i < inPins.size(); ++i) {
            if (inPins[i] == pin) {
                isOutput = false;
                pinIndex = i;
                return true;
            }
        }
    } else if (Input *inp = dynamic_cast<Input*>(owner)) {
        if (pin == inp->getOutputPoint()) {
            isOutput = true;
            pinIndex = 0;
            return true;
        }
    } else if (LED *led = dynamic_cast<LED*>(owner)) {
        if (pin == led->getInputPoint()) {
            isOutput = false;
            pinIndex = 0;
            return true;
        }
    }
    return false;
}

Point* CircuitAdapter::getPin(int nodeId, bool isOutput, int pinIndex) const {
    QGraphicsItem *item = getItem(nodeId);
    if (!item) return nullptr;

    if (Gate *gate = dynamic_cast<Gate*>(item)) {
        if (isOutput) {
            return (pinIndex == 0) ? gate->getOutputPin() : nullptr;
        } else {
            const auto &inPins = gate->getInputPins();
            if (pinIndex >= 0 && pinIndex < inPins.size()) {
                return inPins[pinIndex];
            }
        }
    } else if (Input *inp = dynamic_cast<Input*>(item)) {
        if (isOutput && pinIndex == 0) {
            return inp->getOutputPoint();
        }
    } else if (LED *led = dynamic_cast<LED*>(item)) {
        if (!isOutput && pinIndex == 0) {
            return led->getInputPoint();
        }
    }
    return nullptr;
}

void CircuitAdapter::clear() {
    m_wireToEdge.clear();
    m_edges.clear();
    m_dag.clear();
    m_nodes.clear();
    m_idToItem.clear();
    m_itemToId.clear();
    m_nextId = 0;
    emit simulationInvalidated();
}

void CircuitAdapter::syncSceneItems() {
    if (!m_scene) return;

    for (QGraphicsItem *item : m_scene->items()) {
        if (dynamic_cast<Gate*>(item) || dynamic_cast<Input*>(item) || dynamic_cast<LED*>(item)) {
            registerItem(item);
        }
    }
    syncInputsFromScene();
}

// =========================================================================
// MEMBER 2: Wire Extraction & Graph Edge Connectivity
// =========================================================================

// Helper: resolve a visual Point* pin to (nodeId, pinIndex, isOutput)
bool CircuitAdapter::resolvePinToEndpoint(Point *pin, int &nodeId, int &pinIndex, bool &isOutput) const {
    return getPinInfo(pin, nodeId, isOutput, pinIndex);
}

// Build an Edge from a single wire and register it in the adapter.
// Always returns { sourceNodeId -> targetNodeId } with the Output pin as source.
static Edge buildEdge(Point *startPin, Point *endPin, CircuitAdapter *adapter) {
    int startNodeId = -1, endNodeId = -1;
    int startPinIdx = 0, endPinIdx = 0;
    bool startIsOutput = false, endIsOutput = false;

    if (!adapter->getPinInfo(startPin, startNodeId, startIsOutput, startPinIdx))
        return {};
    if (!adapter->getPinInfo(endPin, endNodeId, endIsOutput, endPinIdx))
        return {};

    if (startIsOutput == endIsOutput)
        return {}; // Invalid connection: Output-to-Output or Input-to-Input

    Edge edge;
    if (startIsOutput) {
        // start = source, end = target
        edge.sourceNodeId = startNodeId;
        edge.sourcePinIdx = startPinIdx;
        edge.targetNodeId = endNodeId;
        edge.targetPinIdx = endPinIdx;
    } else {
        // end = source, start = target (user drew wire backwards – normalize it)
        edge.sourceNodeId = endNodeId;
        edge.sourcePinIdx = endPinIdx;
        edge.targetNodeId = startNodeId;
        edge.targetPinIdx = startPinIdx;
    }
    return edge;
}

void CircuitAdapter::extractWires() {
    if (!m_scene) return;

    m_wireToEdge.clear();
    m_edges.clear();

    std::vector<int> nodeIds;
    nodeIds.reserve(m_nodes.size());
    for (const auto &pair : m_nodes) {
        nodeIds.push_back(pair.first);
    }

    for (QGraphicsItem *item : m_scene->items()) {
        Wire *wire = dynamic_cast<Wire*>(item);
        if (!wire || !wire->getStartPoint() || !wire->getEndPoint()) continue;

        Edge edge = buildEdge(wire->getStartPoint(), wire->getEndPoint(), this);
        if (!edge.isValid()) continue;

        m_wireToEdge[wire] = edge;
        m_edges.push_back(edge);
    }

    // Rebuild the DAG with all nodes and extracted edges
    m_dag.build(nodeIds, m_edges);
}

void CircuitAdapter::onWireConnected(Wire *wire) {
    if (!wire || !wire->getStartPoint() || !wire->getEndPoint()) return;

    Edge edge = buildEdge(wire->getStartPoint(), wire->getEndPoint(), this);
    if (!edge.isValid()) return;

    m_wireToEdge[wire] = edge;
    m_edges.push_back(edge);
    m_dag.addEdge(edge);

    emit simulationInvalidated();
}

void CircuitAdapter::onWireRemoved(Wire *wire) {
    if (!wire) return;

    auto it = m_wireToEdge.find(wire);
    if (it == m_wireToEdge.end()) return;

    const Edge edge = it->second;

    // Reset the input pin on the target node to UNDEFINED
    if (Node *targetNode = getNode(edge.targetNodeId)) {
        targetNode->setInput(static_cast<size_t>(edge.targetPinIdx), LogicState::UNDEFINED);
        targetNode->evaluate();
    }

    // If target node is an LED, reset its visual state to unstimulated (-1) immediately
    if (QGraphicsItem *targetItem = getItem(edge.targetNodeId)) {
        if (LED *led = dynamic_cast<LED*>(targetItem)) {
            led->setLogicValue(-1);
        }
    }

    // Remove from flat edge list
    m_edges.erase(
        std::remove(m_edges.begin(), m_edges.end(), edge),
        m_edges.end()
    );

    m_wireToEdge.erase(it);
    m_dag.removeEdge(edge);

    emit simulationInvalidated();
}

// =========================================================================
// MEMBER 3 (YOUR WORK): Push User Inputs (Clicks & Constants) into Logic
// =========================================================================

void CircuitAdapter::syncInputsFromScene() {
    if (!m_scene) return;

    for (QGraphicsItem *item : m_scene->items()) {
        if (Input *inputItem = dynamic_cast<Input*>(item)) {
            int id = registerItem(inputItem);

            // Synchronize the logical node's state with the Qt element's current state
            if (InputNode *node = getInputNode(id)) {
                node->setState(inputItem->getLogicState());
            }

            // Ensure switch has the click callback wired
            if (Switch *sw = dynamic_cast<Switch*>(inputItem)) {
                sw->setOnToggleCallback([this](Switch *s, bool val) {
                    onSwitchToggled(s, val);
                });
            }
        }
    }
}

void CircuitAdapter::onSwitchToggled(Switch *switchItem, bool newState) {
    if (!switchItem) return;

    int id = getNodeId(switchItem);
    if (id < 0) {
        id = registerItem(switchItem);
    }

    InputNode *node = getInputNode(id);
    if (node) {
        LogicState state = newState ? LogicState::HIGH : LogicState::LOW;
        node->setState(state);
        emit inputChanged(id, state);
    }

    // Mark simulation as stale — do not change LED/wire visuals until user clicks Simulate
    emit simulationInvalidated();
}

void CircuitAdapter::setInputValue(int nodeId, LogicState state) {
    InputNode *node = getInputNode(nodeId);
    if (node) {
        node->setState(state);
        emit inputChanged(nodeId, state);

        // Update corresponding Qt item if it is a Switch
        if (QGraphicsItem *item = getItem(nodeId)) {
            if (Switch *sw = dynamic_cast<Switch*>(item)) {
                sw->setValue(state == LogicState::HIGH);
            }
        }

        emit simulationInvalidated();
    }
}

void CircuitAdapter::setInputValue(Input *inputItem, LogicState state) {
    if (!inputItem) return;
    int id = getNodeId(inputItem);
    if (id >= 0) {
        setInputValue(id, state);
    }
}

LogicState CircuitAdapter::getInputValue(int nodeId) const {
    if (InputNode *node = getInputNode(nodeId)) {
        return node->getState();
    }
    return LogicState::UNDEFINED;
}

InputNode* CircuitAdapter::getInputNode(int nodeId) const {
    auto it = m_nodes.find(nodeId);
    if (it != m_nodes.end()) {
        return dynamic_cast<InputNode*>(it->second.get());
    }
    return nullptr;
}

InputNode* CircuitAdapter::getInputNode(Input *inputItem) const {
    int id = getNodeId(inputItem);
    if (id >= 0) {
        return getInputNode(id);
    }
    return nullptr;
}

// =========================================================================
// MEMBER 4: Logic Outputs Readback to LEDs & Wires (Stub for Member 4)
// =========================================================================

void CircuitAdapter::syncOutputsToScene() {
    if (!m_scene) return;

    // 1. Update LEDs
    for (QGraphicsItem *item : m_scene->items()) {
        if (LED *led = dynamic_cast<LED*>(item)) {
            int id = getNodeId(led);
            if (id >= 0) {
                if (Node *node = getNode(id)) {
                    led->setLogicState(node->getOutput(0));
                }
            } else {
                led->setLogicValue(-1);
            }
        }
    }

    // 2. Update Wires visual signal state based on driving output pin
    for (auto &pair : m_wireToEdge) {
        Wire *wire = pair.first;
        const Edge &edge = pair.second;
        if (wire && edge.isValid()) {
            if (Node *srcNode = getNode(edge.sourceNodeId)) {
                LogicState st = srcNode->getOutput(static_cast<size_t>(edge.sourcePinIdx));
                wire->setLogicValue(st == LogicState::HIGH ? 1 : (st == LogicState::LOW ? 0 : -1));
            }
        }
    }
}

// =========================================================================
// Reset Outputs to Neutral (called when circuit is edited to clear stale results)
// =========================================================================

void CircuitAdapter::resetOutputsToNeutral() {
    if (!m_scene) return;

    // Reset all LEDs to unstimulated (question mark) state
    for (QGraphicsItem *item : m_scene->items()) {
        if (LED *led = dynamic_cast<LED*>(item)) {
            led->setLogicValue(-1);
        }
    }

    // Reset all wire visual states to undefined
    for (auto &pair : m_wireToEdge) {
        Wire *wire = pair.first;
        if (wire) {
            wire->setLogicValue(-1);
        }
    }
}

// =========================================================================
// SIMULATION ENGINE TRIGGER
// =========================================================================

void CircuitAdapter::stepSimulation() {
    // 1. Evaluate logic graph using topological sort on the DAG
    bool stable = m_dag.evaluate(m_nodes);

    if (!stable && m_dag.hasCycle()) {
        std::vector<int> sortedOrder, cycleNodes;
        m_dag.topologicalSort(sortedOrder, cycleNodes);
        emit cycleDetected(cycleNodes);
    }

    // 2. Read back outputs to Qt visuals (LEDs, wires)
    syncOutputsToScene();

    // 3. Emit update notification
    emit simulationStepped();
}

