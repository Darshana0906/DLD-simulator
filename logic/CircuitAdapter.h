#ifndef CIRCUITADAPTER_H
#define CIRCUITADAPTER_H

#include <QObject>
#include <unordered_map>
#include <memory>
#include <vector>

#include "LogicTypes.h"
#include "Node.h"

class CircuitScene;
class QGraphicsItem;
class Switch;
class Input;
class GND;
class Const1;
class Wire;
class LED;

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
    void unregisterItem(QGraphicsItem *item);
    bool hasItem(QGraphicsItem *item) const;

    // Node access
    Node* getNode(int id) const;
    template <typename T>
    T* getNodeAs(int id) const {
        return dynamic_cast<T*>(getNode(id));
    }

    // =========================================================================
    // MEMBER 2: Wire Extraction & Graph Edge Connectivity
    // =========================================================================
    void extractWires();

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

    // =========================================================================
    // SIMULATION ENGINE TRIGGER
    // =========================================================================
    // Evaluates current logic states and notifies outputs
    void stepSimulation();

signals:
    void inputChanged(int nodeId, LogicState newState);
    void simulationStepped();

private:
    CircuitScene *m_scene = nullptr;
    int m_nextId = 0;

    // Mapping between Qt GUI items and Logic Node IDs
    std::unordered_map<QGraphicsItem*, int> m_itemToId;
    std::unordered_map<int, QGraphicsItem*> m_idToItem;

    // Logic Nodes owned by the adapter
    std::unordered_map<int, std::unique_ptr<Node>> m_nodes;
};

#endif // CIRCUITADAPTER_H

