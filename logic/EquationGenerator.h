#ifndef EQUATIONGENERATOR_H
#define EQUATIONGENERATOR_H

#include <QString>
#include <QStringList>
#include <vector>
#include <utility>
#include <unordered_set>

class CircuitAdapter;

struct CircuitReadiness {
    bool isReady = false;
    bool hasFeedbackLoop = false;
    QStringList issues; // Detailed list of open ends
};

struct OutputEquation {
    int nodeId = -1;
    QString outputTag;       // e.g. "Y3"
    QString algebraicExpr;   // e.g. "(S0 · S1) + S2'"
    QString booleanExpr;     // e.g. "(S0 AND S1) OR (NOT S2)"
    QString fullEquation;    // e.g. "Y3 = (S0 · S1) + S2'"
};

struct EquationReport {
    CircuitReadiness readiness;
    std::vector<OutputEquation> equations;
};

class EquationGenerator {
public:
    // Check if the circuit has any open pins (inputs or outputs)
    static CircuitReadiness checkReadiness(const CircuitAdapter *adapter);

    // Generate logical equations for all outputs in the circuit
    static EquationReport generate(const CircuitAdapter *adapter);

private:
    static QString buildExpression(int nodeId, const CircuitAdapter *adapter, bool algebraic,
                                   std::unordered_set<int> &visited, QStringList &errors);
    static QString cleanOuterParens(const QString &expr);
};

#endif // EQUATIONGENERATOR_H

