// resource_allocation_graph.c
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-18-2025
// Implements a Resource Allocation Graph (RAG) to track relationships between train processes 
// and intersection resources. The graph is used to detect circular wait conditions (deadlocks) 
// via depth-first search (DFS). Nodes represent trains and intersections, and edges represent 
// request and allocation states.
#include <stdio.h>
#include <string.h>
#include "resource_allocation_graph.h"
#include "../logger/csv_logger.h" // For logging

#define NAME_LEN 64
/*
See csv_logger.h for definitions.
Update relevant fields with local variables to pass into csv log for debugging.
LOG_CSV(0, "SYSTEM", "INIT_INTERSECTION", "SUCCESS", getpid(), NULL, NULL, NULL, 0, false, 0, NULL, NULL);
*/

// Internal structures
typedef struct {
    NodeType type;
    int id; // train_id or intersection_id
    char name[NAME_LEN]; // Name of the node (for logging)
} Node;

static int adj[MAX_NODES][MAX_NODES]; // Adjacency matrix
static Node nodes[MAX_NODES]; // ID mapping
static int node_count = 0; // Number of nodes in the graph

static int get_or_create_node(NodeType type, int id, const char* name);
static int find_node(NodeType type, int id);
static bool dfs(int v, bool* visited, bool* rec_stack);

// Initializes the graph
void init_graph() {
    memset(adj, 0, sizeof(adj)); // Clear adjacency matrix
    node_count = 0; // Reset node count
}

// Adds a waiting edge: Train -> Intersection
void add_request_edge(int train_id, const char* intersection) {
    int t_idx = get_or_create_node(NODE_TRAIN, train_id, NULL); 
    int i_id = (int)(intersection[0]); // Assuming intersection names are single characters
    int i_idx = get_or_create_node(NODE_INTERSECTION, i_id, intersection);
    adj[t_idx][i_idx] = 1; // add waiting edge
}

// Replaces waiting edge with allocation edge: Intersection -> Train
void add_allocation_edge(int train_id, const char* intersection) {
    int t_idx = find_node(NODE_TRAIN, train_id);
    int i_id = (int)(intersection[0]); // Assuming intersection names are single characters
    int i_idx = find_node(NODE_INTERSECTION, i_id);
    adj[t_idx][i_idx] = 0; // remove waiting edge
    adj[i_idx][t_idx] = 1; // add holding edge
}

// Removes all edges related to this train/intersection
void remove_edges(int train_id, const char* intersection) {
    int t_idx = find_node(NODE_TRAIN, train_id);
    int i_id = (int)(intersection[0]); // Assuming intersection names are single characters
    int i_idx = find_node(NODE_INTERSECTION, i_id);
    adj[t_idx][i_idx] = 0; // remove waiting edge
    adj[i_idx][t_idx] = 0; // remove holding edge
}

// Detects a cycle using DFS
bool detect_deadlock() {
    bool visited[MAX_NODES] = { false }; // Visited nodes
    bool rec_stack[MAX_NODES] = { false }; // Recursion stack

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

static int get_or_create_node(NodeType type, int id, const char* name) {
    int idx = find_node(type, id);
    if (idx >= 0) return idx; // Node already exists

    nodes[node_count].type = type; // Assign type
    nodes[node_count].id = id; // Assign ID
    if (name)
        strncpy(nodes[node_count].name, name, NAME_LEN); // Copy name
    else if (type == NODE_TRAIN)
        snprintf(nodes[node_count].name, NAME_LEN, "Train %d", id); // Format train name
    else
        snprintf(nodes[node_count].name, NAME_LEN, "Intersection %c", (char)id); // Format intersection name

    return node_count++; // Increment node count and return index
}

static int find_node(NodeType type, int id) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].type == type && nodes[i].id == id)
            return i;
    }
    return -1;
}

void print_graph() {
    printf("Resource Allocation Graph:\n");
    for (int i = 0; i < node_count; i++) {
        for (int j = 0; j < node_count; j++) {
            if (adj[i][j]) { // If there is an edge from i to j
                const char* relation = (nodes[i].type == NODE_TRAIN && nodes[j].type == NODE_INTERSECTION)
                    ? "wants"
                    : (nodes[i].type == NODE_INTERSECTION && nodes[j].type == NODE_TRAIN)
                        ? "held by"
                        : "connects to";

                printf("  %s -> %s   // %s\n", nodes[i].name, nodes[j].name, relation);
            }
        }
    }
}
