#ifndef LOGICTYPES_H
#define LOGICTYPES_H

#include <string>

// Pure C++ logic states (Zero Qt dependencies)
enum class LogicState {
    LOW = 0,
    HIGH = 1,
    UNDEFINED = 2
};

// Types of nodes in the logic graph
enum class NodeType {
    INPUT_SWITCH,
    INPUT_CONST1,
    INPUT_GND,
    GATE_AND,
    GATE_OR,
    GATE_NOT,
    GATE_NAND,
    GATE_NOR,
    GATE_XOR,
    GATE_XNOR,
    OUTPUT_LED
};

inline const char* logicStateToString(LogicState state) {
    switch (state) {
    case LogicState::LOW: return "LOW";
    case LogicState::HIGH: return "HIGH";
    default: return "UNDEFINED";
    }
}

inline LogicState boolToLogicState(bool value) {
    return value ? LogicState::HIGH : LogicState::LOW;
}

inline bool logicStateToBool(LogicState state) {
    return state == LogicState::HIGH;
}

// =========================================================================
// Edge: A directed logic connection between two pins in the circuit graph
// source (output pin) ──Wire──> target (input pin)
// =========================================================================
struct Edge {
    int  sourceNodeId  = -1;   // Node driving the signal
    int  sourcePinIdx  =  0;   // Output pin index on source node
    int  targetNodeId  = -1;   // Node receiving the signal
    int  targetPinIdx  =  0;   // Input pin index on target node

    bool isValid() const { return sourceNodeId >= 0 && targetNodeId >= 0; }

    bool operator==(const Edge& other) const {
        return sourceNodeId == other.sourceNodeId &&
               sourcePinIdx == other.sourcePinIdx &&
               targetNodeId == other.targetNodeId &&
               targetPinIdx == other.targetPinIdx;
    }

    bool operator!=(const Edge& other) const {
        return !(*this == other);
    }
};

#endif // LOGICTYPES_H

