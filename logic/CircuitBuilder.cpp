#include "CircuitBuilder.h"
#include "CircuitAdapter.h"
#include "../CircuitScene.h"
#include "../elements/Switch.h"
#include "../elements/LED.h"
#include "../elements/GND.h"
#include "../elements/Const1.h"
#include "../elements/Point.h"
#include "../elements/Wire.h"
#include "../gates/AndGate.h"
#include "../gates/OrGate.h"
#include "../gates/NotGate.h"
#include "../gates/NandGate.h"
#include "../gates/NorGate.h"
#include "../gates/XorGate.h"
#include "../gates/XnorGate.h"
#include <algorithm>

CircuitBuilder::CircuitBuilder(CircuitScene *scene)
    : m_scene(scene), m_adapter(scene ? scene->getAdapter() : nullptr) {
}

// =========================================================================
// Compute depth of an AST subtree (for layering)
// =========================================================================

int CircuitBuilder::computeDepth(const ASTNode *node) const {
    if (!node) return 0;

    if (node->type == ASTNodeType::VARIABLE ||
        node->type == ASTNodeType::CONST_0 ||
        node->type == ASTNodeType::CONST_1) {
        return 0; // Leaf: layer 0
    }

    int maxChildDepth = 0;
    for (const auto &child : node->children) {
        maxChildDepth = std::max(maxChildDepth, computeDepth(child.get()));
    }
    return maxChildDepth + 1;
}

// =========================================================================
// Collect all unique variable names from AST
// =========================================================================

void CircuitBuilder::collectVariables(const ASTNode *node, QStringList &vars) const {
    if (!node) return;

    if (node->type == ASTNodeType::VARIABLE) {
        if (!vars.contains(node->variableName)) {
            vars.append(node->variableName);
        }
        return;
    }

    for (const auto &child : node->children) {
        collectVariables(child.get(), vars);
    }
}

// =========================================================================
// Layout helpers
// =========================================================================

qreal CircuitBuilder::xForLayer(int layer) const {
    return LEFT_MARGIN + layer * HORIZONTAL_SPACING;
}

qreal CircuitBuilder::nextYForLayer(int layer) {
    int idx = m_layerItemCount[layer]++;
    return TOP_MARGIN + idx * VERTICAL_SPACING;
}

// =========================================================================
// Place an item at a position and register it
// =========================================================================

void CircuitBuilder::placeItem(QGraphicsItem *item, qreal x, qreal y) {
    if (!item || !m_scene) return;
    item->setPos(x, y);
    m_scene->addItem(item);
    if (m_adapter) {
        m_adapter->registerItem(item);
    }
}

// =========================================================================
// Wire two pins together
// =========================================================================

void CircuitBuilder::wireConnect(Point *sourceOutput, Point *targetInput) {
    if (!sourceOutput || !targetInput || !m_scene) return;

    Wire *wire = new Wire(sourceOutput, targetInput);
    m_scene->addItem(wire);

    if (m_adapter) {
        m_adapter->onWireConnected(wire);
    }

    m_wiresCreated++;
}

// =========================================================================
// Create a gate of the appropriate type
// =========================================================================

Gate* CircuitBuilder::createGate(ASTNodeType type, int inputCount) {
    Gate *gate = nullptr;

    switch (type) {
    case ASTNodeType::AND:
        gate = new AndGate();
        break;
    case ASTNodeType::OR:
        gate = new OrGate();
        break;
    case ASTNodeType::NOT:
        gate = new NotGate();
        break;
    case ASTNodeType::NAND:
        gate = new NandGate();
        break;
    case ASTNodeType::NOR:
        gate = new NorGate();
        break;
    case ASTNodeType::XOR:
        gate = new XorGate();
        break;
    case ASTNodeType::XNOR:
        gate = new XnorGate();
        break;
    default:
        return nullptr;
    }

    // Adjust input count for multi-input gates (default is 2)
    if (gate && type != ASTNodeType::NOT && inputCount != 2) {
        gate->setInputCount(inputCount);
    }

    return gate;
}

// =========================================================================
// Compute canonical signature for an AST subtree (for CSE)
// =========================================================================

QString CircuitBuilder::getSignature(const ASTNode *node) const {
    if (!node) return "";

    if (node->type == ASTNodeType::VARIABLE) {
        return QString("VAR:%1").arg(node->variableName);
    }
    if (node->type == ASTNodeType::CONST_0) {
        return "CONST:0";
    }
    if (node->type == ASTNodeType::CONST_1) {
        return "CONST:1";
    }

    std::vector<QString> childSigs;
    for (const auto &child : node->children) {
        childSigs.push_back(getSignature(child.get()));
    }

    // For commutative operations, sort children signatures so A·B and B·A match
    bool commutative = (node->type == ASTNodeType::AND ||
                        node->type == ASTNodeType::OR ||
                        node->type == ASTNodeType::XOR ||
                        node->type == ASTNodeType::XNOR ||
                        node->type == ASTNodeType::NAND ||
                        node->type == ASTNodeType::NOR);
    if (commutative && childSigs.size() > 1) {
        std::sort(childSigs.begin(), childSigs.end());
    }

    QString sig = QString::number(static_cast<int>(node->type)) + "(";
    for (size_t i = 0; i < childSigs.size(); ++i) {
        if (i > 0) sig += ",";
        sig += childSigs[i];
    }
    sig += ")";
    return sig;
}

// =========================================================================
// Recursively build circuit from AST node (with CSE & sharing)
// =========================================================================

CircuitBuilder::BuildInfo CircuitBuilder::buildNode(const ASTNode *node, int depth) {
    BuildInfo info;
    if (!node) return info;

    // ----- Leaf: Variable (Switch) -----
    if (node->type == ASTNodeType::VARIABLE) {
        // Reuse existing switch if same variable name was already created
        auto it = m_variableToSwitch.find(node->variableName);
        if (it != m_variableToSwitch.end()) {
            if (Switch *sw = dynamic_cast<Switch*>(it->second)) {
                info.item = sw;
                info.outputPin = sw->getOutputPoint();
                info.depth = 0;
                return info;
            }
        }

        Switch *sw = new Switch();
        qreal x = xForLayer(0);
        qreal y = nextYForLayer(0);
        placeItem(sw, x, y);

        m_variableToSwitch[node->variableName] = sw;

        if (m_adapter) {
            int nodeId = m_adapter->getNodeId(sw);
            if (nodeId >= 0) {
                m_adapter->setTag(nodeId, node->variableName);
            }
        }

        info.item = sw;
        info.outputPin = sw->getOutputPoint();
        info.depth = 0;
        return info;
    }

    // ----- Leaf: Constant 0 (GND) - Reused across whole circuit -----
    if (node->type == ASTNodeType::CONST_0) {
        if (!m_gndItem) {
            m_gndItem = new GND();
            qreal x = xForLayer(0);
            qreal y = nextYForLayer(0);
            placeItem(m_gndItem, x, y);
        }

        info.item = m_gndItem;
        info.outputPin = m_gndItem->getOutputPoint();
        info.depth = 0;
        return info;
    }

    // ----- Leaf: Constant 1 - Reused across whole circuit -----
    if (node->type == ASTNodeType::CONST_1) {
        if (!m_const1Item) {
            m_const1Item = new Const1();
            qreal x = xForLayer(0);
            qreal y = nextYForLayer(0);
            placeItem(m_const1Item, x, y);
        }

        info.item = m_const1Item;
        info.outputPin = m_const1Item->getOutputPoint();
        info.depth = 0;
        return info;
    }

    // ----- Common Subexpression Elimination (CSE) Check -----
    QString sig = getSignature(node);
    if (!sig.isEmpty()) {
        auto it = m_builtSubexpressions.find(sig);
        if (it != m_builtSubexpressions.end()) {
            // Found existing sub-expression! Reuse the existing gate!
            return it->second;
        }
    }

    // ----- Gate nodes -----
    int inputCount = static_cast<int>(node->children.size());

    // Build all children first to get their output pins
    std::vector<BuildInfo> childInfos;
    int maxChildDepth = 0;
    for (const auto &child : node->children) {
        BuildInfo ci = buildNode(child.get(), depth + 1);
        maxChildDepth = std::max(maxChildDepth, ci.depth);
        childInfos.push_back(std::move(ci));
    }

    int gateLayer = maxChildDepth + 1;

    // Create the gate
    Gate *gate = createGate(node->type, inputCount);
    if (!gate) {
        return info;
    }

    qreal x = xForLayer(gateLayer);
    qreal y = nextYForLayer(gateLayer);
    placeItem(gate, x, y);
    m_gatesCreated++;

    // Wire children's output pins to this gate's input pins
    const auto &inputPins = gate->getInputPins();
    for (int i = 0; i < static_cast<int>(childInfos.size()) && i < inputPins.size(); ++i) {
        if (childInfos[i].outputPin) {
            wireConnect(childInfos[i].outputPin, inputPins[i]);
        }
    }

    info.item = gate;
    info.outputPin = gate->getOutputPin();
    info.depth = gateLayer;

    // Cache this sub-expression for CSE
    if (!sig.isEmpty()) {
        m_builtSubexpressions[sig] = info;
    }

    return info;
}

// =========================================================================
// Public API: Build circuit from parsed equations
// =========================================================================

BuildResult CircuitBuilder::build(const std::vector<ParsedEquation> &equations) {
    BuildResult result;

    if (!m_scene || !m_adapter) {
        result.success = false;
        result.errorMessage = "Scene or adapter not available.";
        return result;
    }

    if (equations.empty()) {
        result.success = false;
        result.errorMessage = "No equations to build.";
        return result;
    }

    // Reset state
    m_variableToSwitch.clear();
    m_gndItem = nullptr;
    m_const1Item = nullptr;
    m_builtSubexpressions.clear();
    m_layerItemCount.clear();
    m_gatesCreated = 0;
    m_wiresCreated = 0;

    // First pass: collect all variables across all equations and pre-create switches
    QStringList allVars;
    int totalDepth = 0;
    for (const auto &eq : equations) {
        collectVariables(eq.root.get(), allVars);
        totalDepth = std::max(totalDepth, computeDepth(eq.root.get()));
    }

    // Pre-create switches for all variables (layer 0)
    for (const QString &varName : allVars) {
        if (m_variableToSwitch.find(varName) == m_variableToSwitch.end()) {
            Switch *sw = new Switch();
            qreal x = xForLayer(0);
            qreal y = nextYForLayer(0);
            placeItem(sw, x, y);
            m_variableToSwitch[varName] = sw;

            if (m_adapter) {
                int nodeId = m_adapter->getNodeId(sw);
                if (nodeId >= 0) {
                    m_adapter->setTag(nodeId, varName);
                }
            }
        }
    }

    int switchesCreated = static_cast<int>(m_variableToSwitch.size());

    // LED layer is totalDepth + 1
    int ledLayer = totalDepth + 1;
    int ledsCreated = 0;

    // Build each equation
    for (const auto &eq : equations) {
        if (!eq.root) continue;

        // Build the expression tree
        BuildInfo exprInfo = buildNode(eq.root.get(), 0);

        if (!exprInfo.outputPin) {
            result.success = false;
            result.errorMessage = QString("Failed to build expression for output '%1'.").arg(eq.outputName);
            return result;
        }

        // Create LED at the output layer
        LED *led = new LED();
        qreal x = xForLayer(ledLayer);
        qreal y = nextYForLayer(ledLayer);
        placeItem(led, x, y);
        ledsCreated++;

        if (m_adapter) {
            int nodeId = m_adapter->getNodeId(led);
            if (nodeId >= 0) {
                QString outTag = eq.outputName.isEmpty() ? QString("Y%1").arg(ledsCreated) : eq.outputName;
                m_adapter->setTag(nodeId, outTag);
            }
        }

        // Wire the expression output to the LED input
        wireConnect(exprInfo.outputPin, led->getInputPoint());
    }

    // Extract DAG graph connections and refresh tags
    if (m_adapter) {
        m_adapter->extractWires();
    }
    if (m_scene) {
        m_scene->setShowTags(true);
        m_scene->update();
    }

    result.success = true;
    result.gatesCreated = m_gatesCreated;
    result.switchesCreated = switchesCreated;
    result.ledsCreated = ledsCreated;
    result.wiresCreated = m_wiresCreated;
    result.bounds = m_scene ? m_scene->itemsBoundingRect() : QRectF();
    return result;
}
