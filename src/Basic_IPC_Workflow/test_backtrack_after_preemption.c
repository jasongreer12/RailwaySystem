// test_backtrack_after_preemption.c
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-19-2025
//
// Description:
// This test simulates a deadlock between two trains where each holds an intersection 
// the other needs. It resolves the deadlock by preempting Train 1, allowing Train 2 
// to proceed. Train 1 then backtracks and re-requests the intersection. The test 
// verifies that deadlock is resolved and does not recur.

#include <stdio.h>
#include "resource_allocation_graph.h"

int main() {
    printf("Backtracking After Preemption Test\n");

    // Initialize the graph
    init_graph();

    // Setup initial state
    add_request_edge(1, "A");      // Train 1 requests A
    add_request_edge(1, "B");      // Train 1 requests B
    add_allocation_edge(1, "B");   // Train 1 holds B

    add_request_edge(2, "B");      // Train 2 requests B
    add_request_edge(2, "A");      // Train 2 requests A
    add_allocation_edge(2, "A");   // Train 2 holds A

    printf("Before preemption:\n");
    print_graph();

    // Detecting a deadlock
    if (detect_deadlock()) {
        printf("Deadlock detected. Resolving...\n");
        remove_edges(1, "B");
    } else {
        printf("No deadlock detected — test failed\n");
        return 1;
    }

    // Train 2 proceeds and gets B
    add_allocation_edge(2, "B");
    printf("Train 2 now holds Intersection B.\n");

    // Train 1 backtracks and re-requests B
    add_request_edge(1, "B");
    printf("Train 1 re-requests B.\n");

    printf("\nAfter backtracking:\n");
    print_graph();

    if (detect_deadlock()) {
        printf("Still in deadlock — test failed.\n");
        return 1;
    } else {
        printf("No deadlock — Train 1 successfully backtracked.\n");
        return 0;
    }
}
