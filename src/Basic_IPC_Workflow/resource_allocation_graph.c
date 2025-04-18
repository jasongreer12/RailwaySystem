// resource_allocation_graph.c
// Author: Zachary Oyer
// Date: 4-11-2025
// Group B
// Implements a Resource Allocation Graph (RAG) to track relationships between train processes 
// and intersection resources. The graph is used to detect circular wait conditions (deadlocks) 
// via depth-first search (DFS). Nodes represent trains and intersections, and edges represent 
// request and allocation states.
#include <stdio.h>
#include <string.h>
#include "resource_allocation_graph.h"
#include "../logger/csv_logger.h" // For logging

/*
See csv_logger.h for definitions.
Update relevant fields with local variables to pass into csv log for debugging.
LOG_CSV(0, "SYSTEM", "INIT_INTERSECTION", "SUCCESS", getpid(), NULL, NULL, NULL, 0, false, 0, NULL, NULL);
*/

// Internal structures
typedef struct {
    NodeType type;
    int id; // train_id or intersection_id
} Node;

static int adj[MAX_NODES][MAX_NODES]; // Adjacency matrix
static Node nodes[MAX_NODES]; // ID mapping
static int node_count = 0;

static int get_or_create_node(NodeType type, int id);
static int find_node(NodeType type, int id);
static bool dfs(int v, bool* visited, bool* rec_stack);

// Initializes the graph
void init_graph() {
    memset(adj, 0, sizeof(adj));
    node_count = 0;
}

// Adds a waiting edge: Train → Intersection
void add_request_edge(int train_id, const char* intersection) {
    int t_idx = get_or_create_node(NODE_TRAIN, train_id);
    int i_id = (int)(intersection[0]); // Simplified hash — use better one if needed
    int i_idx = get_or_create_node(NODE_INTERSECTION, i_id);
    adj[t_idx][i_idx] = 1;
}

// Replaces waiting edge with allocation edge: Intersection → Train
void add_allocation_edge(int train_id, const char* intersection) {
    int t_idx = find_node(NODE_TRAIN, train_id);
    int i_id = (int)(intersection[0]);
    int i_idx = find_node(NODE_INTERSECTION, i_id);
    adj[t_idx][i_idx] = 0; // remove waiting edge
    adj[i_idx][t_idx] = 1; // add holding edge
}

// Removes all edges related to this train/intersection
void remove_edges(int train_id, const char* intersection) {
    int t_idx = find_node(NODE_TRAIN, train_id);
    int i_id = (int)(intersection[0]);
    int i_idx = find_node(NODE_INTERSECTION, i_id);
    adj[t_idx][i_idx] = 0;
    adj[i_idx][t_idx] = 0;
}

// Detects a cycle using DFS
bool detect_deadlock() {
    bool visited[MAX_NODES] = { false };
    bool rec_stack[MAX_NODES] = { false };

    for (int i = 0; i < node_count; i++) {
        if (!visited[i] && dfs(i, visited, rec_stack)) {
            return true;
        }
    }
    return false;
}

// DFS for cycle detection
static bool dfs(int v, bool* visited, bool* rec_stack) {
    visited[v] = true;
    rec_stack[v] = true;

    for (int u = 0; u < node_count; u++) {
        if (adj[v][u]) {
            if (!visited[u] && dfs(u, visited, rec_stack))
                return true;
            else if (rec_stack[u])
                return true;
        }
    }

    rec_stack[v] = false;
    return false;
}

// Returns node index and creates if doesn't exist
static int get_or_create_node(NodeType type, int id) {
    int idx = find_node(type, id);
    if (idx >= 0) return idx;

    nodes[node_count].type = type;
    nodes[node_count].id = id;
    return node_count++;
}

// Finds node index by type + id
static int find_node(NodeType type, int id) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].type == type && nodes[i].id == id)
            return i;
    }
    return -1;
}

// Optional print graph
void print_graph() {
    printf("Resource Allocation Graph:\n");
    for (int i = 0; i < node_count; i++) {
        for (int j = 0; j < node_count; j++) {
            if (adj[i][j])
                printf("  Node[%d] → Node[%d]\n", i, j);
        }
    }
}
