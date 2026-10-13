#include "CircuitDAG.h"

const std::vector<Edge> CircuitDAG::s_emptyEdges = {};

void CircuitDAG::addNode(int nodeId) {
    if (nodeId < 0) return;
    if (m_nodeIds.insert(nodeId).second) {
        m_adjList[nodeId] = {};
        m_inEdges[nodeId] = {};
        m_inDegree[nodeId] = 0;
    }
}

void CircuitDAG::removeNode(int nodeId) {
    if (!hasNode(nodeId)) return;

    // 1. Remove all outgoing edges from this node
    auto itOut = m_adjList.find(nodeId);
    if (itOut != m_adjList.end()) {
        for (const Edge &edge : itOut->second) {
            // Decrement in-degree of target node
            auto itDeg = m_inDegree.find(edge.targetNodeId);
            if (itDeg != m_inDegree.end() && itDeg->second > 0) {
                itDeg->second--;
            }
            // Remove from target node's incoming edges list
            auto itIn = m_inEdges.find(edge.targetNodeId);
            if (itIn != m_inEdges.end()) {
                itIn->second.erase(
                    std::remove(itIn->second.begin(), itIn->second.end(), edge),
                    itIn->second.end()
                );
            }
        }
    }

    // 2. Remove all incoming edges to this node
    auto itIn = m_inEdges.find(nodeId);
    if (itIn != m_inEdges.end()) {
        for (const Edge &edge : itIn->second) {
            auto itSrc = m_adjList.find(edge.sourceNodeId);
            if (itSrc != m_adjList.end()) {
                itSrc->second.erase(
                    std::remove(itSrc->second.begin(), itSrc->second.end(), edge),
                    itSrc->second.end()
                );
            }
        }
    }

    // 3. Remove all matching edges from the master edge vector
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(), [nodeId](const Edge &e) {
            return e.sourceNodeId == nodeId || e.targetNodeId == nodeId;
        }),
        m_edges.end()
    );

    // 4. Erase maps and node ID
    m_adjList.erase(nodeId);
    m_inEdges.erase(nodeId);
    m_inDegree.erase(nodeId);
    m_nodeIds.erase(nodeId);
}

void CircuitDAG::addEdge(const Edge &edge) {
    if (!edge.isValid()) return;

    // Ensure nodes exist in graph
    addNode(edge.sourceNodeId);
    addNode(edge.targetNodeId);

    // Prevent duplicate edges
    for (const Edge &existing : m_edges) {
        if (existing == edge) return;
    }

    m_edges.push_back(edge);
    m_adjList[edge.sourceNodeId].push_back(edge);
    m_inEdges[edge.targetNodeId].push_back(edge);
    m_inDegree[edge.targetNodeId]++;
}

void CircuitDAG::removeEdge(const Edge &edge) {
    if (!edge.isValid()) return;

    // Remove from master edge list
    auto itEdge = std::find(m_edges.begin(), m_edges.end(), edge);
    if (itEdge == m_edges.end()) return;
    m_edges.erase(itEdge);

    // Remove from source's outgoing list
    auto itOut = m_adjList.find(edge.sourceNodeId);
    if (itOut != m_adjList.end()) {
        auto it = std::find(itOut->second.begin(), itOut->second.end(), edge);
        if (it != itOut->second.end()) {
            itOut->second.erase(it);
        }
    }

    // Remove from target's incoming list
    auto itIn = m_inEdges.find(edge.targetNodeId);
    if (itIn != m_inEdges.end()) {
        auto it = std::find(itIn->second.begin(), itIn->second.end(), edge);
        if (it != itIn->second.end()) {
            itIn->second.erase(it);
        }
    }

    // Decrement in-degree
    auto itDeg = m_inDegree.find(edge.targetNodeId);
    if (itDeg != m_inDegree.end() && itDeg->second > 0) {
        itDeg->second--;
    }
}

void CircuitDAG::clear() {
    m_nodeIds.clear();
    m_edges.clear();
    m_adjList.clear();
    m_inEdges.clear();
    m_inDegree.clear();
}

void CircuitDAG::build(const std::vector<int> &nodeIds, const std::vector<Edge> &edges) {
    clear();
    for (int id : nodeIds) {
        addNode(id);
    }
    for (const Edge &edge : edges) {
        addEdge(edge);
    }
}

// -------------------------------------------------------------------------
// Kahn's Algorithm for Topological Sorting & Cycle Detection
// -------------------------------------------------------------------------
bool CircuitDAG::topologicalSort(std::vector<int> &sortedOrder, std::vector<int> &cycleNodes) const {
    sortedOrder.clear();
    cycleNodes.clear();

    if (m_nodeIds.empty()) return true;

    // Copy in-degrees for simulation processing
    std::unordered_map<int, int> currentInDegree = m_inDegree;
    std::queue<int> zeroInDegreeQueue;

    // Enqueue all nodes with in-degree 0 (inputs or disconnected gates)
    for (int nodeId : m_nodeIds) {
        if (currentInDegree[nodeId] == 0) {
            zeroInDegreeQueue.push(nodeId);
        }
    }

    // Process nodes in topological order
    while (!zeroInDegreeQueue.empty()) {
        int u = zeroInDegreeQueue.front();
        zeroInDegreeQueue.pop();
        sortedOrder.push_back(u);

        auto it = m_adjList.find(u);
        if (it != m_adjList.end()) {
            for (const Edge &edge : it->second) {
                int v = edge.targetNodeId;
                currentInDegree[v]--;
                if (currentInDegree[v] == 0) {
                    zeroInDegreeQueue.push(v);
                }
            }
        }
    }

    // If all nodes were visited, graph is a pure DAG (acyclic)
    if (sortedOrder.size() == m_nodeIds.size()) {
        return true;
    }

    // Otherwise, a cycle exists (feedback loops)
    for (int nodeId : m_nodeIds) {
        if (std::find(sortedOrder.begin(), sortedOrder.end(), nodeId) == sortedOrder.end()) {
            cycleNodes.push_back(nodeId);
        }
    }
    return false;
}

bool CircuitDAG::hasCycle() const {
    std::vector<int> order;
    std::vector<int> cycles;
    return !topologicalSort(order, cycles);
}

std::vector<int> CircuitDAG::getTopologicalOrder() const {
    std::vector<int> order;
    std::vector<int> cycles;
    if (topologicalSort(order, cycles)) {
        return order;
    }
    return {};
}

// -------------------------------------------------------------------------
// Simulation Execution
// -------------------------------------------------------------------------
bool CircuitDAG::evaluate(const std::unordered_map<int, std::unique_ptr<Node>> &nodes, int maxIterations) {
    if (nodes.empty()) return true;

    // Step 1: Evaluate all primary input nodes (Switches, GND, Const1)
    for (int id : m_nodeIds) {
        auto it = nodes.find(id);
        if (it != nodes.end() && it->second) {
            NodeType type = it->second->getType();
            if (type == NodeType::INPUT_SWITCH ||
                type == NodeType::INPUT_CONST1 ||
                type == NodeType::INPUT_GND) {
                it->second->evaluate();
            }
        }
    }

    // Step 2: Attempt fast single-pass topological evaluation
    std::vector<int> sortedOrder;
    std::vector<int> cycleNodes;
    bool isAcyclic = topologicalSort(sortedOrder, cycleNodes);

    if (isAcyclic) {
        for (int nodeId : sortedOrder) {
            auto itNode = nodes.find(nodeId);
            if (itNode == nodes.end() || !itNode->second) continue;

            Node *node = itNode->second.get();
            NodeType type = node->getType();

            // Skip primary inputs as their outputs were evaluated in Step 1
            if (type == NodeType::INPUT_SWITCH ||
                type == NodeType::INPUT_CONST1 ||
                type == NodeType::INPUT_GND) {
                continue;
            }

            // Transfer upstream signals from incoming edges to input pins
            auto itIn = m_inEdges.find(nodeId);
            if (itIn != m_inEdges.end()) {
                for (const Edge &inEdge : itIn->second) {
                    auto itSrc = nodes.find(inEdge.sourceNodeId);
                    if (itSrc != nodes.end() && itSrc->second) {
                        LogicState srcVal = itSrc->second->getOutput(static_cast<size_t>(inEdge.sourcePinIdx));
                        node->setInput(static_cast<size_t>(inEdge.targetPinIdx), srcVal);
                    }
                }
            }

            // Evaluate gate / output node
            node->evaluate();
        }
        return true;
    }

    // Step 3: Cyclic graph (Feedback loops / Latches / Flip-Flops)
    // Run iterative relaxation until convergence or max iterations reached
    for (int iter = 0; iter < maxIterations; ++iter) {
        bool changed = false;

        // Propagate signals across all edges
        for (const Edge &edge : m_edges) {
            auto itSrc = nodes.find(edge.sourceNodeId);
            auto itDst = nodes.find(edge.targetNodeId);
            if (itSrc != nodes.end() && itDst != nodes.end() && itSrc->second && itDst->second) {
                LogicState srcVal = itSrc->second->getOutput(static_cast<size_t>(edge.sourcePinIdx));
                if (itDst->second->getInput(static_cast<size_t>(edge.targetPinIdx)) != srcVal) {
                    itDst->second->setInput(static_cast<size_t>(edge.targetPinIdx), srcVal);
                    changed = true;
                }
            }
        }

        // Evaluate all gates and outputs
        for (int id : m_nodeIds) {
            auto itNode = nodes.find(id);
            if (itNode != nodes.end() && itNode->second) {
                NodeType type = itNode->second->getType();
                if (type != NodeType::INPUT_SWITCH &&
                    type != NodeType::INPUT_CONST1 &&
                    type != NodeType::INPUT_GND) {
                    LogicState oldOut = itNode->second->getOutput(0);
                    itNode->second->evaluate();
                    if (itNode->second->getOutput(0) != oldOut) {
                        changed = true;
                    }
                }
            }
        }

        // If circuit state didn't change, we reached a stable steady state
        if (!changed) {
            return true;
        }
    }

    // Exceeded max iterations (circuit oscillates or has an unstable combinational loop)
    return false;
}

// -------------------------------------------------------------------------
// Queries
// -------------------------------------------------------------------------
bool CircuitDAG::hasNode(int nodeId) const {
    return m_nodeIds.find(nodeId) != m_nodeIds.end();
}

int CircuitDAG::getInDegree(int nodeId) const {
    auto it = m_inDegree.find(nodeId);
    return (it != m_inDegree.end()) ? it->second : 0;
}

const std::vector<Edge>& CircuitDAG::getOutgoingEdges(int nodeId) const {
    auto it = m_adjList.find(nodeId);
    return (it != m_adjList.end()) ? it->second : s_emptyEdges;
}

const std::vector<Edge>& CircuitDAG::getIncomingEdges(int nodeId) const {
    auto it = m_inEdges.find(nodeId);
    return (it != m_inEdges.end()) ? it->second : s_emptyEdges;
}
