// test_backtrack_after_preemption.c
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-19-2025
//
// Description:
// This test simulates a complex edge case involving deadlock resolution by preemption
// followed by train backtracking. It uses the Resource Allocation Graph (RAG) to
// simulate two trains in a deadlock scenario, where each train wants a resource
// (intersection) currently held by the other. Once deadlock is detected, we simulate
// preempting one train’s hold, allowing the other train to proceed. Then, the preempted
// train backtracks and re-requests the resource it lost. The test ensures that deadlock
// is resolved and that the backtracking train can retry without causing another deadlock.

#include <stdio.h>
#include "resource_allocation_graph.h"

int main() {
    printf("Backtracking After Preemption Test \n");

    // Step 1: Initialize the Resource Allocation Graph
    init_graph();

    // Step 2: Simulate a conflict setup leading to a potential deadlock
    // Train 1 wants Intersection A (T1 -> A) and already holds Intersection B (B -> T1)
    // Train 2 wants Intersection B (T2 -> B) and already holds Intersection A (A -> T2)
    // The graph looks like this:
    // T1 -> A -> T2 -> B -> T1  (Cycle)

    add_request_edge(1, "A");       // Train 1 is requesting Intersection A
    add_allocation_edge(1, "B");    // Train 1 is holding Intersection B

    add_request_edge(2, "B");       // Train 2 is requesting Intersection B
    add_allocation_edge(2, "A");    // Train 2 is holding Intersection A

    // Print the graph state before preemption
    printf("Before preemption:\n");
    print_graph();

    // Step 3: Check if a deadlock exists in the current graph
    if (detect_deadlock()) {
        printf("Deadlock detected: resolving (preemption)\n");

    // Step 4: Simulate preemption by forcibly removing Train 1 from holding Intersection B
    // This breaks the cycle in the graph, allowing Train 2 to proceed
        remove_edges(1, "B");
        printf("Train 1 was preempted from Intersection B.\n");
    }

    // Step 5: Train 2 is now free to acquire Intersection B
    add_allocation_edge(2, "B");
    printf("Train 2 now holds Intersection B.\n");

    // Step 6: Train 1 now backtracks and re-requests the resource it previously lost (Intersection B)
    add_request_edge(1, "B");
    printf("Train 1 is backtracking and re-requesting Intersection B.\n");

    // Step 7: Display final state of the graph after preemption and backtracking
    printf("\nAfter backtracking:\n");
    print_graph();

    // Step 8: Confirm that the system is no longer in deadlock
    if (detect_deadlock()) {
        printf("Still in deadlock: test failed.\n");
        return 1;
    } else {
        printf("No deadlock: Train 1 successfully backtracked.\n");
        return 0;
    }
}
