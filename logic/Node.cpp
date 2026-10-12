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

