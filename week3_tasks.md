# Set up mutexes for single capacity intersections. - Jarett Woodard
# Use semaphores for multi-capacity intersections (e.g., sem_wait()). -Jake P
# Track held intersections and waiting trains in shared memory. - Steve Kuria
# Implement a resource allocation graph to detect cycles (e.g., check for Train1 →
# IntersectionA → Train3 → IntersectionC → Train1) - Zachary Oyer
# Start logging events (e.g., TRAIN1: Sent ACQUIRE request). – Jason Greer
# Simulate different train schedules / Introduce deadlock scenarios. -Jake Richardson