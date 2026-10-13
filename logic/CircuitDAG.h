#ifndef CIRCUITDAG_H
#define CIRCUITDAG_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <queue>
#include <algorithm>

#include "LogicTypes.h"
#include "Node.h"

// =========================================================================
// CircuitDAG: Directed Acyclic Graph manager for the digital logic circuit
//
// Responsibilities:
// 1. Manages logic nodes and directed pin-to-pin edges
// 2. Kahn's Algorithm for Topological Sorting & Cycle Detection
// 3. Signal propagation and multi-stage evaluation engine
// =========================================================================
class CircuitDAG {
public:
    CircuitDAG() = default;
    ~CircuitDAG() = default;

    // ---------------------------------------------------------------------
    // Graph Construction & Modification
    // ---------------------------------------------------------------------
    void addNode(int nodeId);
    void removeNode(int nodeId);
    void addEdge(const Edge &edge);
    void removeEdge(const Edge &edge);
    void clear();

    // Rebuild the graph in bulk from a list of node IDs and Edges
    void build(const std::vector<int> &nodeIds, const std::vector<Edge> &edges);

    // ---------------------------------------------------------------------
    // Topological Sort & Cycle Detection (Kahn's Algorithm)
    // ---------------------------------------------------------------------
    // Returns true if the circuit is acyclic (a valid DAG).
    // - sortedOrder: populated with node IDs in dependency order (inputs first)
    // - cycleNodes: populated with node IDs involved in loops if a cycle is found
    bool topologicalSort(std::vector<int> &sortedOrder, std::vector<int> &cycleNodes) const;

    // Fast check whether the graph contains any feedback loops/cycles
    bool hasCycle() const;

    // Returns the topological order if acyclic, or empty vector if cyclic
    std::vector<int> getTopologicalOrder() const;

    // ---------------------------------------------------------------------
    // Simulation Engine Execution
    // ---------------------------------------------------------------------
    // Propagates signals through nodes following the topological sort order.
    // If feedback cycles are present, falls back to iterative relaxation
    // (up to maxIterations) to stabilize latches/flip-flops.
    // Returns true if the circuit reached a stable deterministic state.
    bool evaluate(const std::unordered_map<int, std::unique_ptr<Node>> &nodes, int maxIterations = 32);

    // ---------------------------------------------------------------------
    // Graph Inspection Queries
    // ---------------------------------------------------------------------
    bool hasNode(int nodeId) const;
    int getNodeCount() const { return static_cast<int>(m_nodeIds.size()); }
    int getEdgeCount() const { return static_cast<int>(m_edges.size()); }
    int getInDegree(int nodeId) const;

    const std::unordered_set<int>& getNodeIds() const { return m_nodeIds; }
    const std::vector<Edge>& getEdges() const { return m_edges; }
    const std::vector<Edge>& getOutgoingEdges(int nodeId) const;
    const std::vector<Edge>& getIncomingEdges(int nodeId) const;

private:
    std::unordered_set<int> m_nodeIds;
    std::vector<Edge> m_edges;

    // Adjacency lists
    std::unordered_map<int, std::vector<Edge>> m_adjList; // srcNodeId -> [outgoing edges]
    std::unordered_map<int, std::vector<Edge>> m_inEdges; // dstNodeId -> [incoming edges]
    std::unordered_map<int, int> m_inDegree;              // dstNodeId -> count of incoming edges

    static const std::vector<Edge> s_emptyEdges;
};

#endif // CIRCUITDAG_H
