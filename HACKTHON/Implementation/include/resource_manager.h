#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "banker.h"

/* Initialize and release system memory */
bool initialize_system(
    BankerSystem *system,
    int process_count,
    int resource_count
);

void free_system(BankerSystem *system);

/* Read initial resource information */
bool input_available_resources(BankerSystem *system);

bool input_allocation_matrix(BankerSystem *system);

bool input_maximum_matrix(BankerSystem *system);

/* Display resource information */
void display_available(const BankerSystem *system);

void display_allocation(const BankerSystem *system);

void display_maximum(const BankerSystem *system);

void display_system_state(const BankerSystem *system);

/* Initialize all resource data */
bool initialize_resource_data(BankerSystem *system);

#endif