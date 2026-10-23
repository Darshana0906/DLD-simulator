#ifndef NODE_H
#define NODE_H

#include "LogicTypes.h"
#include <vector>
#include <cstddef>

// Base class for all pure C++ logic nodes in the simulation graph
class Node {
public:
    explicit Node(int id, NodeType type);
    virtual ~Node() = default;

    int getId() const { return m_id; }
    NodeType getType() const { return m_type; }

    virtual void evaluate() = 0;

    LogicState getInput(size_t index) const;
    void setInput(size_t index, LogicState state);

    LogicState getOutput(size_t index = 0) const;
    void setOutput(size_t index, LogicState state);

    size_t inputCount() const { return m_inputs.size(); }
    void setInputCount(size_t count) { m_inputs.resize(count, LogicState::UNDEFINED); }
    size_t outputCount() const { return m_outputs.size(); }

protected:
    int m_id;
    NodeType m_type;
    std::vector<LogicState> m_inputs;
    std::vector<LogicState> m_outputs;
};

// =========================================================================
// InputNode: Represents GND (fixed LOW), Const1 (fixed HIGH), and Switch (0/1)
// =========================================================================
class InputNode : public Node {
public:
    explicit InputNode(int id, LogicState initialState = LogicState::LOW, NodeType inputType = NodeType::INPUT_SWITCH);

    void evaluate() override;

    void setState(LogicState state);
    LogicState getState() const;

    bool isDynamic() const { return m_type == NodeType::INPUT_SWITCH; }
    bool isConstant() const { return !isDynamic(); }

private:
    LogicState m_state;
};

// =========================================================================
// GateNode: Represents logic gates (AND, OR, NOT, NAND, NOR, XOR, XNOR)
// =========================================================================
class GateNode : public Node {
public:
    explicit GateNode(int id, NodeType gateType, size_t inputCount = 2);

    void evaluate() override;

    static LogicState evaluateGate(NodeType type, const std::vector<LogicState>& inputs);
};

// =========================================================================
// OutputNode: Represents output indicators (e.g. LED, bulb)
// =========================================================================
class OutputNode : public Node {
public:
    explicit OutputNode(int id);

    void evaluate() override;

    LogicState getState() const;
    bool isOn() const { return getState() == LogicState::HIGH; }
};

#endif // NODE_H

