#include "Node.h"

Node::Node(int id, NodeType type)
    : m_id(id), m_type(type) {}

LogicState Node::getInput(size_t index) const {
    if (index < m_inputs.size()) {
        return m_inputs[index];
    }
    return LogicState::UNDEFINED;
}

void Node::setInput(size_t index, LogicState state) {
    if (index >= m_inputs.size()) {
        m_inputs.resize(index + 1, LogicState::UNDEFINED);
    }
    m_inputs[index] = state;
}

LogicState Node::getOutput(size_t index) const {
    if (index < m_outputs.size()) {
        return m_outputs[index];
    }
    return LogicState::UNDEFINED;
}

void Node::setOutput(size_t index, LogicState state) {
    if (index >= m_outputs.size()) {
        m_outputs.resize(index + 1, LogicState::UNDEFINED);
    }
    m_outputs[index] = state;
}

// =========================================================================
// InputNode Implementation
// =========================================================================

InputNode::InputNode(int id, LogicState initialState, NodeType inputType)
    : Node(id, inputType), m_state(initialState) {
    // Inputs have 0 inputs and 1 output pin
    m_outputs.resize(1, initialState);

    // Enforce fixed values for GND and Const1
    if (inputType == NodeType::INPUT_GND) {
        m_state = LogicState::LOW;
        m_outputs[0] = LogicState::LOW;
    } else if (inputType == NodeType::INPUT_CONST1) {
        m_state = LogicState::HIGH;
        m_outputs[0] = LogicState::HIGH;
    }
}

void InputNode::evaluate() {
    if (m_type == NodeType::INPUT_GND) {
        m_state = LogicState::LOW;
    } else if (m_type == NodeType::INPUT_CONST1) {
        m_state = LogicState::HIGH;
    }
    setOutput(0, m_state);
}

void InputNode::setState(LogicState state) {
    // GND and Const1 cannot have their values overridden
    if (m_type == NodeType::INPUT_GND) {
        m_state = LogicState::LOW;
    } else if (m_type == NodeType::INPUT_CONST1) {
        m_state = LogicState::HIGH;
    } else {
        m_state = state;
    }
    setOutput(0, m_state);
}

LogicState InputNode::getState() const {
    return m_state;
}

// =========================================================================
// GateNode Implementation
// =========================================================================

GateNode::GateNode(int id, NodeType gateType, size_t inputCount)
    : Node(id, gateType) {
    if (gateType == NodeType::GATE_NOT) {
        m_inputs.resize(1, LogicState::UNDEFINED);
    } else {
        m_inputs.resize(inputCount, LogicState::UNDEFINED);
    }
    m_outputs.resize(1, LogicState::UNDEFINED);
}

void GateNode::evaluate() {
    setOutput(0, evaluateGate(m_type, m_inputs));
}

LogicState GateNode::evaluateGate(NodeType type, const std::vector<LogicState>& inputs) {
    if (inputs.empty()) {
        return LogicState::UNDEFINED;
    }

    switch (type) {
    case NodeType::GATE_NOT: {
        LogicState in = inputs[0];
        if (in == LogicState::HIGH) return LogicState::LOW;
        if (in == LogicState::LOW) return LogicState::HIGH;
        return LogicState::UNDEFINED;
    }
    case NodeType::GATE_AND: {
        bool hasUndefined = false;
        for (LogicState in : inputs) {
            if (in == LogicState::LOW) return LogicState::LOW;
            if (in == LogicState::UNDEFINED) hasUndefined = true;
        }
        return hasUndefined ? LogicState::UNDEFINED : LogicState::HIGH;
    }
    case NodeType::GATE_OR: {
        bool hasUndefined = false;
        for (LogicState in : inputs) {
            if (in == LogicState::HIGH) return LogicState::HIGH;
            if (in == LogicState::UNDEFINED) hasUndefined = true;
        }
        return hasUndefined ? LogicState::UNDEFINED : LogicState::LOW;
    }
    case NodeType::GATE_NAND: {
        LogicState andRes = evaluateGate(NodeType::GATE_AND, inputs);
        if (andRes == LogicState::HIGH) return LogicState::LOW;
        if (andRes == LogicState::LOW) return LogicState::HIGH;
        return LogicState::UNDEFINED;
    }
    case NodeType::GATE_NOR: {
        LogicState orRes = evaluateGate(NodeType::GATE_OR, inputs);
        if (orRes == LogicState::HIGH) return LogicState::LOW;
        if (orRes == LogicState::LOW) return LogicState::HIGH;
        return LogicState::UNDEFINED;
    }
    case NodeType::GATE_XOR: {
        int highCount = 0;
        for (LogicState in : inputs) {
            if (in == LogicState::UNDEFINED) return LogicState::UNDEFINED;
            if (in == LogicState::HIGH) highCount++;
        }
        return (highCount % 2 != 0) ? LogicState::HIGH : LogicState::LOW;
    }
    case NodeType::GATE_XNOR: {
        LogicState xorRes = evaluateGate(NodeType::GATE_XOR, inputs);
        if (xorRes == LogicState::HIGH) return LogicState::LOW;
        if (xorRes == LogicState::LOW) return LogicState::HIGH;
        return LogicState::UNDEFINED;
    }
    default:
        return LogicState::UNDEFINED;
    }
}

// =========================================================================
// OutputNode Implementation
// =========================================================================

OutputNode::OutputNode(int id)
    : Node(id, NodeType::OUTPUT_LED) {
    m_inputs.resize(1, LogicState::UNDEFINED);
    m_outputs.resize(1, LogicState::UNDEFINED);
}

void OutputNode::evaluate() {
    setOutput(0, getInput(0));
}

LogicState OutputNode::getState() const {
    return getOutput(0);
}

