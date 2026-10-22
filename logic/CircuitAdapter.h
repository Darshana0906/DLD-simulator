#ifndef CIRCUITADAPTER_H
#define CIRCUITADAPTER_H

#include <QObject>
#include <unordered_map>
#include <memory>
#include <vector>

#include "LogicTypes.h"
#include "Node.h"
#include "CircuitDAG.h"


class CircuitScene;
class QGraphicsItem;
class Switch;
class Input;
class GND;
class Const1;
class Wire;
class LED;

class Gate;
class Point;

class CircuitAdapter : public QObject {
    Q_OBJECT
public:
    explicit CircuitAdapter(CircuitScene *scene = nullptr, QObject *parent = nullptr);
    ~CircuitAdapter() override = default;

    void setScene(CircuitScene *scene);
    CircuitScene* getScene() const { return m_scene; }

    // =========================================================================
    // MEMBER 1: QGraphicsItem* <---> NodeID (int) Mapping
    // =========================================================================
    int registerItem(QGraphicsItem *item, int preferredId = -1);
    int getNodeId(QGraphicsItem *item) const;
    QGraphicsItem* getItem(int id) const;
    const std::unordered_map<int, QGraphicsItem*>& getItems() const { return m_idToItem; }
    void unregisterItem(QGraphicsItem *item);
    bool hasItem(QGraphicsItem *item) const;
    size_t getNodeCount() const { return m_nodes.size(); }

    // Node access
    Node* getNode(int id) const;
    template <typename T>
    T* getNodeAs(int id) const {
        return dynamic_cast<T*>(getNode(id));
    }

    GateNode* getGateNode(int id) const;
    GateNode* getGateNode(Gate *gateItem) const;
    OutputNode* getOutputNode(int id) const;
    OutputNode* getOutputNode(LED *ledItem) const;

    // Pin resolution (bridges visual Point* pins to NodeID and pinIndex)
    bool getPinInfo(Point *pin, int &nodeId, bool &isOutput, int &pinIndex) const;
    Point* getPin(int nodeId, bool isOutput, int pinIndex) const;

    // Reset all internal mappings, DAG nodes, and edges
    void clear();

    // Synchronize all components from scene (Gates, Inputs, LEDs)
    void syncSceneItems();

    // =========================================================================
    // MEMBER 2: Wire Extraction & Graph Edge Connectivity
    // =========================================================================

    // Scan all Wire items in the scene and build directed Edges.
    // Call this after components are placed and wires are drawn.
    void extractWires();

    // Called automatically by CircuitScene whenever a wire connection is completed.
    void onWireConnected(Wire *wire);

    // Called automatically by CircuitScene when a wire is deleted.
    void onWireRemoved(Wire *wire);

    // Read-only access to the edge list (used by simulation engine)
    const std::vector<Edge>& getEdges() const { return m_edges; }

    // =========================================================================
    // MEMBER 3 (YOUR WORK): Push User Inputs (Clicks & Constants) into Logic
    // =========================================================================
    // 1. Scans all inputs (Switch, GND, Const1) in the scene, registers them,
    //    creates InputNodes, sets their initial states, and binds click callbacks.
    void syncInputsFromScene();

    // 2. Direct event-driven handler called whenever a Switch is toggled by click.
    void onSwitchToggled(Switch *switchItem, bool newState);

    // 3. Programmatic input setter by NodeID or Item
    void setInputValue(int nodeId, LogicState state);
    void setInputValue(Input *inputItem, LogicState state);

    // 4. Retrieve logical state of an input
    LogicState getInputValue(int nodeId) const;
    InputNode* getInputNode(int nodeId) const;
    InputNode* getInputNode(Input *inputItem) const;

    // =========================================================================
    // MEMBER 4: Logic Outputs Readback to LEDs & Wires
    // =========================================================================
    void syncOutputsToScene();
    void resetOutputsToNeutral();

    // =========================================================================
    // SIMULATION ENGINE TRIGGER (DAG Topological Evaluation)
    // =========================================================================
    // Evaluates current logic states using topological sort and notifies outputs
    void stepSimulation();

    // Access to DAG representation and Topological Sort
    const CircuitDAG& getDAG() const { return m_dag; }
    CircuitDAG& getDAG() { return m_dag; }
    bool hasCycle() const { return m_dag.hasCycle(); }
    std::vector<int> getTopologicalOrder() const { return m_dag.getTopologicalOrder(); }

signals:
    void inputChanged(int nodeId, LogicState newState);
    void simulationStepped();
    void simulationInvalidated();
    void cycleDetected(const std::vector<int> &cycleNodeIds);

private:
    CircuitScene *m_scene = nullptr;
    int m_nextId = 0;

    // Mapping between Qt GUI items and Logic Node IDs
    std::unordered_map<QGraphicsItem*, int>            m_itemToId;
    std::unordered_map<int, QGraphicsItem*>            m_idToItem;

    // Logic Nodes owned by the adapter
    std::unordered_map<int, std::unique_ptr<Node>>     m_nodes;

    // Directed Acyclic Graph structure for topological sorting & simulation
    CircuitDAG                                         m_dag;

    // Wire-to-Edge mapping: each completed wire is stored as a directed Edge
    std::unordered_map<Wire*, Edge>                    m_wireToEdge;
    std::vector<Edge>                                  m_edges;

    // Helper: resolve a pin to its Edge endpoint
    bool resolvePinToEndpoint(Point *pin, int &nodeId, int &pinIndex, bool &isOutput) const;
};

#endif // CIRCUITADAPTER_H

