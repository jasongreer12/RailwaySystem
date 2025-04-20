// resource_allocation_graph.h
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-11-2025
// Header file for the Resource Allocation Graph (RAG) module used to model resource dependencies between trains and intersections 
// in the railway simulation. Provides function declarations for adding and removing edges, cycle detection, and graph visualization.
#ifndef RESOURCE_ALLOCATION_GRAPH_H
#define RESOURCE_ALLOCATION_GRAPH_H

#include <stdbool.h>

#define MAX_TRAINS 10
#define MAX_RESOURCES 10
#define MAX_NODES (MAX_TRAINS + MAX_RESOURCES)

// Node types
typedef enum {
    NODE_TRAIN,
    NODE_INTERSECTION
} NodeType;

// Graph edge table
void init_graph();
void add_request_edge(int train_id, const char* intersection);
void add_allocation_edge(int train_id, const char* intersection);
void remove_edges(int train_id, const char* intersection);
bool detect_deadlock();
void print_graph();

#endif
