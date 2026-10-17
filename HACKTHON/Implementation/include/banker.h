#ifndef BANKER_H
#define BANKER_H

#include <stdbool.h>

/*
 * Smart Resource Allocation Using Banker's Algorithm
 *
 * All matrices use dynamically allocated, row-major arrays.
 *
 * Matrix element (i, j):
 * index = i * resource_count + j
 */

typedef struct
{
    int process_count;
    int resource_count;

    int *available;
    int *maximum;
    int *allocation;
    int *need;

    /*
     * Outstanding resources currently requested by each process.
     * Used by the separate deadlock-detection module.
     */
    int *outstanding_request;

    char **process_names;
    char **resource_names;

} BankerSystem;


/* Resource manager */

bool initialize_system(
    BankerSystem *system,
    int process_count,
    int resource_count
);

void free_system(BankerSystem *system);

bool input_available_resources(BankerSystem *system);

bool input_allocation_matrix(BankerSystem *system);

bool input_maximum_matrix(BankerSystem *system);

void display_available(const BankerSystem *system);

void display_allocation(const BankerSystem *system);

void display_maximum(const BankerSystem *system);

void display_system_state(const BankerSystem *system);

bool initialize_resource_data(BankerSystem *system);


/* Banker's safety algorithm */

bool calculate_need(BankerSystem *system);

void display_need(const BankerSystem *system);

bool safety_check(
    const BankerSystem *system,
    int *safe_sequence
);


/* Resource requests */

bool request_resources(
    BankerSystem *system,
    int process_id,
    const int *request,
    int *safe_sequence
);


/* Deadlock detection */

bool input_outstanding_requests(BankerSystem *system);

bool detect_deadlock(
    const BankerSystem *system,
    bool *deadlocked_processes
);

void display_deadlock_status(
    const BankerSystem *system,
    const bool *deadlocked_processes
);

#endif