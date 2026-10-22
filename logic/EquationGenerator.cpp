#include "EquationGenerator.h"
#include "CircuitAdapter.h"
#include "Node.h"
#include "CircuitDAG.h"
#include <algorithm>

CircuitReadiness EquationGenerator::checkReadiness(const CircuitAdapter *adapter) {
    CircuitReadiness result;
    result.isReady = true;

    if (!adapter) {
        result.isReady = false;
        result.issues.append("Circuit adapter is not initialized.");
        return result;
    }

    const auto &items = adapter->getItems();
    if (items.empty()) {
        result.isReady = false;
        result.issues.append("Circuit is empty. Place components and wires to generate equations.");
        return result;
    }

    const CircuitDAG &dag = adapter->getDAG();

    // 1. Check for feedback loops / sequential cycles
    if (dag.hasCycle()) {
        result.isReady = false;
        result.hasFeedbackLoop = true;
        result.issues.append("Feedback loop detected");
        return result;
    }

    int gateCount = 0;
    int ledCount = 0;

    for (const auto &pair : items) {
        int id = pair.first;
        QGraphicsItem *item = pair.second;
        if (!item) continue;

        Node *node = adapter->getNode(id);
        if (!node) continue;

        NodeType type = node->getType();

        // 2. Check Gates: All input pins must be connected, output must be connected
        if (GateNode *gateNode = adapter->getGateNode(id)) {
            gateCount++;
            int expectedInputs = static_cast<int>(gateNode->inputCount());
            const auto &inEdges = dag.getIncomingEdges(id);

            std::vector<bool> pinConnected(expectedInputs, false);
            for (const Edge &e : inEdges) {
                if (e.targetPinIdx >= 0 && e.targetPinIdx < expectedInputs) {
                    pinConnected[e.targetPinIdx] = true;
                }
            }

            for (int p = 0; p < expectedInputs; ++p) {
                if (!pinConnected[p]) {
                    result.isReady = false;
                    result.issues.append(QString("Gate G%1: Input pin %2 is open / not connected.").arg(id).arg(p + 1));
                }
            }

            const auto &outEdges = dag.getOutgoingEdges(id);
            if (outEdges.empty()) {
                result.isReady = false;
                result.issues.append(QString("Gate G%1: Output is open / not connected to anything.").arg(id));
            }
        }
        // 3. Check LEDs: Input pin must be connected
        else if (type == NodeType::OUTPUT_LED) {
            ledCount++;
            const auto &inEdges = dag.getIncomingEdges(id);
            bool hasInput = false;
            for (const Edge &e : inEdges) {
                if (e.targetPinIdx == 0) {
                    hasInput = true;
                    break;
                }
            }
            if (!hasInput) {
                result.isReady = false;
                result.issues.append(QString("Output Y%1: Input pin is open / not connected.").arg(id));
            }
        }
        // 4. Check Switches: Output should be connected to something
        else if (type == NodeType::INPUT_SWITCH) {
            const auto &outEdges = dag.getOutgoingEdges(id);
            if (outEdges.empty()) {
                result.isReady = false;
                result.issues.append(QString("Switch S%1: Output is open / not connected to any gate.").arg(id));
            }
        }
    }

    if (ledCount == 0) {
        result.isReady = false;
        result.issues.append("No output LED found in the circuit. Connect an LED to observe the output equation.");
    }

    return result;
}

QString EquationGenerator::cleanOuterParens(const QString &expr) {
    QString s = expr.trimmed();
    if (s.startsWith('(') && s.endsWith(')')) {
        // Verify matching paren depth
        int depth = 0;
        bool outerMatchesAll = true;
        for (int i = 0; i < s.length(); ++i) {
            if (s[i] == '(') depth++;
            else if (s[i] == ')') depth--;
            if (depth == 0 && i < s.length() - 1) {
                outerMatchesAll = false;
                break;
            }
        }
        if (outerMatchesAll) {
            return cleanOuterParens(s.mid(1, s.length() - 2));
        }
    }
    return s;
}

QString EquationGenerator::buildExpression(int nodeId, const CircuitAdapter *adapter, bool algebraic,
                                           std::unordered_set<int> &visited, QStringList &errors) {
    if (!adapter) return "?";
    Node *node = adapter->getNode(nodeId);
    if (!node) return "?";

    if (visited.find(nodeId) != visited.end()) {
        errors.append(QString("Cyclic reference at Node %1").arg(nodeId));
        return QString("LOOP(%1)").arg(nodeId);
    }
    visited.insert(nodeId);

    NodeType type = node->getType();

    if (type == NodeType::INPUT_SWITCH) {
        visited.erase(nodeId);
        return QString("S%1").arg(nodeId);
    }
    if (type == NodeType::INPUT_CONST1) {
        visited.erase(nodeId);
        return "1";
    }
    if (type == NodeType::INPUT_GND) {
        visited.erase(nodeId);
        return "0";
    }

    const CircuitDAG &dag = adapter->getDAG();
    const auto &inEdges = dag.getIncomingEdges(nodeId);

    if (GateNode *gate = adapter->getGateNode(nodeId)) {
        int inCount = static_cast<int>(gate->inputCount());
        std::vector<QString> pinExprs(inCount, "?");

        for (const Edge &e : inEdges) {
            if (e.targetPinIdx >= 0 && e.targetPinIdx < inCount) {
                pinExprs[e.targetPinIdx] = buildExpression(e.sourceNodeId, adapter, algebraic, visited, errors);
            }
        }

        visited.erase(nodeId);

        auto wrapIfNeeded = [](const QString &s) -> QString {
            if (s.isEmpty()) return s;
            if (s.length() <= 3 && !s.contains(' ') && !s.contains('+')) {
                return s; // single atom like "S1", "0", "1"
            }
            if (s.startsWith('(') && s.endsWith(')')) {
                return s;
            }
            return QString("(%1)").arg(s);
        };

        if (type == NodeType::GATE_NOT) {
            QString inner = pinExprs.empty() ? "?" : pinExprs[0];
            if (algebraic) {
                if (inner.length() <= 3 && !inner.contains(' ') && !inner.contains('+')) {
                    return inner + "'";
                }
                return wrapIfNeeded(inner) + "'";
            } else {
                return QString("NOT %1").arg(wrapIfNeeded(inner));
            }
        }

        QString opSymbol = algebraic ? QString::fromUtf8(" · ") : " AND ";
        if (type == NodeType::GATE_OR || type == NodeType::GATE_NOR) {
            opSymbol = algebraic ? " + " : " OR ";
        } else if (type == NodeType::GATE_XOR || type == NodeType::GATE_XNOR) {
            opSymbol = algebraic ? QString::fromUtf8(" ⊕ ") : " XOR ";
        }

        QString combined;
        for (size_t i = 0; i < pinExprs.size(); ++i) {
            if (i > 0) combined += opSymbol;
            combined += pinExprs[i];
        }

        if (type == NodeType::GATE_NAND || type == NodeType::GATE_NOR) {
            if (algebraic) {
                return QString("(%1)'").arg(combined);
            } else {
                return QString("NOT (%1)").arg(combined);
            }
        } else if (type == NodeType::GATE_XNOR) {
            if (algebraic) {
                return QString("(%1)'").arg(combined);
            } else {
                return QString("NOT (%1)").arg(combined);
            }
        }

        return QString("(%1)").arg(combined);
    }

    if (type == NodeType::OUTPUT_LED) {
        if (!inEdges.empty()) {
            QString sub = buildExpression(inEdges[0].sourceNodeId, adapter, algebraic, visited, errors);
            visited.erase(nodeId);
            return sub;
        }
    }

    visited.erase(nodeId);
    return "?";
}

EquationReport EquationGenerator::generate(const CircuitAdapter *adapter) {
    EquationReport report;
    report.readiness = checkReadiness(adapter);

    if (!report.readiness.isReady || !adapter) {
        return report;
    }

    const auto &items = adapter->getItems();
    const CircuitDAG &dag = adapter->getDAG();

    // Generate equations only for output LEDs
    for (const auto &pair : items) {
        int id = pair.first;
        Node *node = adapter->getNode(id);
        if (!node || node->getType() != NodeType::OUTPUT_LED) continue;

        const auto &inEdges = dag.getIncomingEdges(id);
        if (inEdges.empty()) continue;

        int drivingNodeId = inEdges[0].sourceNodeId;

        std::unordered_set<int> visitedAlg;
        QStringList errorsAlg;
        QString alg = buildExpression(drivingNodeId, adapter, true, visitedAlg, errorsAlg);
        alg = cleanOuterParens(alg);

        std::unordered_set<int> visitedBool;
        QStringList errorsBool;
        QString bExpr = buildExpression(drivingNodeId, adapter, false, visitedBool, errorsBool);
        bExpr = cleanOuterParens(bExpr);

        OutputEquation eq;
        eq.nodeId = id;
        eq.outputTag = QString("Y%1").arg(id);
        eq.algebraicExpr = alg;
        eq.booleanExpr = bExpr;
        eq.fullEquation = QString("Y%1 = %2").arg(id).arg(alg);

        report.equations.push_back(eq);
    }

    return report;
}
