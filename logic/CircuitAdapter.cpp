#include "CircuitAdapter.h"
#include "../CircuitScene.h"
#include "../elements/Input.h"
#include "../elements/Switch.h"
#include "../elements/GND.h"
#include "../elements/Const1.h"
#include "../elements/LED.h"
#include <QGraphicsItem>

CircuitAdapter::CircuitAdapter(CircuitScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene) {
    if (m_scene) {
        syncInputsFromScene();
    }
}

void CircuitAdapter::setScene(CircuitScene *scene) {
    m_scene = scene;
    if (m_scene) {
        syncInputsFromScene();
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

    // Automatically instantiate corresponding InputNode if it is an Input item
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
    }

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
        m_idToItem.erase(id);
        m_nodes.erase(id);
        m_itemToId.erase(it);
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

// =========================================================================
// MEMBER 2: Wire Extraction & Graph Edge Connectivity (Stub for Member 2)
// =========================================================================

void CircuitAdapter::extractWires() {
    // Member 2 will populate directed edges between Gate/Input/Output pins here
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

    // Immediately step simulation and push updated outputs to visuals
    stepSimulation();
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

        stepSimulation();
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

    // Default basic hook for Member 4: update any LEDs if their logic node is set
    for (QGraphicsItem *item : m_scene->items()) {
        if (LED *led = dynamic_cast<LED*>(item)) {
            int id = getNodeId(led);
            if (id >= 0) {
                if (Node *node = getNode(id)) {
                    led->setState(node->getOutput(0) == LogicState::HIGH);
                }
            }
        }
    }
}

// =========================================================================
// SIMULATION ENGINE TRIGGER
// =========================================================================

void CircuitAdapter::stepSimulation() {
    // 1. Evaluate all registered nodes
    for (auto &pair : m_nodes) {
        if (pair.second) {
            pair.second->evaluate();
        }
    }

    // 2. Read back outputs to Qt visuals (LEDs, wires)
    syncOutputsToScene();

    // 3. Emit update notification
    emit simulationStepped();
}

