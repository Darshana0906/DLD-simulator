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

#endif // LOGICTYPES_H

