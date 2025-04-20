// test_rag.c
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-11-2025
// Test program for the Resource Allocation Graph (RAG) module.
// This program simulates a scenario where two trains request and allocate intersections,
// and checks for deadlocks in the system.
// It uses the RAG module to manage the relationships between trains and intersections.
// The program initializes the graph, simulates train requests and allocations, and checks for deadlocks.
// It also prints the graph for debugging purposes.
#include <stdio.h>
#include "resource_allocation_graph.h"

int main() {
    init_graph();

    // Simulate Train 1 requesting Intersection A
    add_request_edge(1, "A");

    // Simulate Intersection A is granted to Train 1
    add_allocation_edge(1, "A");

    // Simulate Train 2 requesting Intersection B
    add_request_edge(2, "B");

    // Simulate Intersection B granted to Train 2
    add_allocation_edge(2, "B");

    // Simulate Train 1 requesting Intersection B
    add_request_edge(1, "B");

    // Simulate Train 2 requesting Intersection A
    add_request_edge(2, "A");

    // Check for deadlock
    if (detect_deadlock()) {
        printf("Deadlock detected!\n");
    } else {
        printf("No deadlock.\n");
    }

    print_graph();
    return 0;
}
