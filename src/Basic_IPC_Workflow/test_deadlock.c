// test_deadlock.c
// Author: Zachary Oyer
// Date: 4-18-2025
// Group B
// Simple test for deadlock detection using Resource Allocation Graph

#include <stdio.h>
#include "resource_allocation_graph.h"

int main() {
    printf("=== Deadlock Detection Test ===\n");

    // Initialize the graph
    init_graph();

    // Preload all nodes by simulating neutral edges that create them safely
    // This ensures internal node mapping (node_count++) happens in the right order
    add_request_edge(1, "A");        // Train1 → A
    add_request_edge(2, "B");        // Train2 → B
    add_allocation_edge(1, "B");     // B → Train1
    add_allocation_edge(2, "A");     // A → Train2

    // Now add the real cycle to simulate:
    // T1 → A → T2 → B → T1

    // You’ve already created the cycle with the 4 calls above,
    // so now check if it's detected:

    if (detect_deadlock()) {
        printf("Deadlock correctly detected!\n");
        print_graph();  // Show internal graph structure
        return 0;
    } else {
        printf("Deadlock was NOT detected — something’s wrong.\n");
        return 1;
    }
}
