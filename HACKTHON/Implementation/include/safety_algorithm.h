#ifndef SAFETY_ALGORITHM_H
#define SAFETY_ALGORITHM_H

#include "banker.h"

/*
 * Calculate Need Matrix.
 *
 * Need = Maximum - Allocation
 */
bool calculate_need(BankerSystem *system);

/* Display the Need Matrix */
void display_need(const BankerSystem *system);

/*
 * Check whether the current system state is safe.
 *
 * Returns:
 * true  -> A complete safe sequence exists.
 * false -> No complete safe sequence exists.
 *
 * safe_sequence stores the calculated process order
 * when the state is safe.
 *
 * This function must not modify the real resource state.
 */
bool safety_check(
    const BankerSystem *system,
    int *safe_sequence
);

#endif