#ifndef CIRCUITBUILDER_H
#define CIRCUITBUILDER_H

#include <QString>
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <unordered_map>
#include <vector>
#include <memory>

#include "EquationParser.h"

class CircuitScene;
class CircuitAdapter;
class QGraphicsItem;
class Gate;
class Point;
class Wire;

// =========================================================================
// Build Result: returned after circuit generation
// =========================================================================
struct BuildResult {
    bool success = false;
    QString errorMessage;
    int gatesCreated = 0;
    int switchesCreated = 0;
    int ledsCreated = 0;
    int wiresCreated = 0;
    QRectF bounds;
};

// =========================================================================
// CircuitBuilder: Converts an AST into visual circuit components on the scene
// =========================================================================
class CircuitBuilder {
public:
    explicit CircuitBuilder(CircuitScene *scene);

    // Build circuit from parsed equations
    BuildResult build(const std::vector<ParsedEquation> &equations);

private:
    CircuitScene *m_scene;
    CircuitAdapter *m_adapter;

    // Counters for stats
    int m_gatesCreated = 0;
    int m_wiresCreated = 0;

    // Layout constants
    static constexpr qreal LEFT_MARGIN = 400.0;
    static constexpr qreal TOP_MARGIN = 300.0;
    static constexpr qreal HORIZONTAL_SPACING = 240.0;
    static constexpr qreal VERTICAL_SPACING = 140.0;

    // Track switches by variable name so we reuse a single Switch per variable
    std::unordered_map<QString, QGraphicsItem*> m_variableToSwitch;
    class GND *m_gndItem = nullptr;
    class Const1 *m_const1Item = nullptr;

    // Track the output pin of each AST node after it's built, so we can wire it
    struct BuildInfo {
        QGraphicsItem *item = nullptr;   // the Gate, Switch, GND, Const1 that was created
        Point *outputPin = nullptr;       // the output pin to wire from
        int depth = 0;                    // depth in AST (for layering)
        QString pendingYVar;             // if waiting for an output pin from a Y variable
    };

    struct PendingYConnection {
        Point *targetInputPin = nullptr;
        QString yVarName;
    };

    std::vector<PendingYConnection> m_pendingYConnections;
    std::unordered_map<QString, Point*> m_outputPins;

    // Common Subexpression Elimination (CSE) cache: signature -> BuildInfo
    std::unordered_map<QString, BuildInfo> m_builtSubexpressions;

    // Generate canonical signature for AST subtree (used by CSE)
    QString getSignature(const ASTNode *node) const;

    // Compute the depth (number of layers) of an AST subtree
    int computeDepth(const ASTNode *node) const;

    // Collect all unique variable names from the AST
    void collectVariables(const ASTNode *node, QStringList &vars) const;

    // Recursively build the circuit for an AST node
    // Returns the output pin that represents this sub-expression's result
    BuildInfo buildNode(const ASTNode *node, int depth);

    // Create a gate item of the appropriate type on the scene
    Gate* createGate(ASTNodeType type, int inputCount);

    // Place an item at a computed position and register with adapter
    void placeItem(QGraphicsItem *item, qreal x, qreal y);

    // Wire two pins together: source output → target input
    void wireConnect(Point *sourceOutput, Point *targetInput);

    // Layout tracking: how many items have been placed at each depth layer
    std::unordered_map<int, int> m_layerItemCount;

    // Get next Y position for a given layer
    qreal nextYForLayer(int layer);

    // Get X position for a given layer
    qreal xForLayer(int layer) const;
};

#endif // CIRCUITBUILDER_H
