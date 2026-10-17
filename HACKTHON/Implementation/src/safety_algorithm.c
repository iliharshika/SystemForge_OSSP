#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "safety_algorithm.h"

/* Calculate Need = Maximum - Allocation. */
bool calculate_need(BankerSystem *system)
{
    if (system == NULL || system->maximum == NULL ||
        system->allocation == NULL || system->need == NULL ||
        system->process_count <= 0 || system->resource_count <= 0)
    {
        return false;
    }

    int n = system->process_count;
    int m = system->resource_count;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            size_t index = (size_t)i * (size_t)m + (size_t)j;

            if (system->maximum[index] < 0 ||
                system->allocation[index] < 0 ||
                system->maximum[index] < system->allocation[index])
            {
                printf("Error: Maximum must be >= Allocation, and values cannot be negative.\n");
                return false;
            }

            system->need[index] =
                system->maximum[index] - system->allocation[index];
        }
    }

    return true;
}

/* Display the Need Matrix. */
void display_need(const BankerSystem *system)
{
    if (system == NULL || system->need == NULL ||
        system->process_names == NULL || system->resource_names == NULL ||
        system->process_count <= 0 || system->resource_count <= 0)
    {
        printf("Invalid system data for displaying Need Matrix.\n");
        return;
    }

    printf("\n========== NEED MATRIX ==========\n");
    printf("%-16s", "Process");

    for (int j = 0; j < system->resource_count; j++)
    {
        printf("%-12s", system->resource_names[j]);
    }

    printf("\n");

    for (int i = 0; i < system->process_count; i++)
    {
        printf("%-16s", system->process_names[i]);

        for (int j = 0; j < system->resource_count; j++)
        {
            size_t index = (size_t)i * (size_t)system->resource_count
                         + (size_t)j;
            printf("%-12d", system->need[index]);
        }

        printf("\n");
    }
}

/* Check safety without modifying the actual system state. */
bool safety_check(const BankerSystem *system, int *safe_sequence)
{
    if (system == NULL || safe_sequence == NULL ||
        system->available == NULL || system->allocation == NULL ||
        system->maximum == NULL || system->need == NULL ||
        system->process_count <= 0 || system->resource_count <= 0)
    {
        return false;
    }

    int n = system->process_count;
    int m = system->resource_count;

    int *work = malloc((size_t)m * sizeof(int));
    bool *finish = calloc((size_t)n, sizeof(bool));

    if (work == NULL || finish == NULL)
    {
        free(work);
        free(finish);
        printf("Unable to allocate safety-check memory.\n");
        return false;
    }

    for (int j = 0; j < m; j++)
    {
        if (system->available[j] < 0)
        {
            free(work);
            free(finish);
            return false;
        }

        work[j] = system->available[j];
    }

    /* Verify Need is up to date before running the algorithm. */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            size_t index = (size_t)i * (size_t)m + (size_t)j;

            if (system->allocation[index] < 0 ||
                system->maximum[index] < system->allocation[index] ||
                system->need[index] !=
                    system->maximum[index] - system->allocation[index])
            {
                free(work);
                free(finish);
                printf("Invalid system data: Need must equal Maximum minus Allocation.\n");
                return false;
            }
        }
    }

    int completed = 0;

    while (completed < n)
    {
        bool found = false;

        for (int i = 0; i < n; i++)
        {
            if (finish[i])
            {
                continue;
            }

            bool can_finish = true;

            for (int j = 0; j < m; j++)
            {
                size_t index = (size_t)i * (size_t)m + (size_t)j;

                if (system->need[index] > work[j])
                {
                    can_finish = false;
                    break;
                }
            }

            if (!can_finish)
            {
                continue;
            }

            for (int j = 0; j < m; j++)
            {
                size_t index = (size_t)i * (size_t)m + (size_t)j;

                if (system->allocation[index] > INT_MAX - work[j])
                {
                    free(work);
                    free(finish);
                    return false;
                }

                work[j] += system->allocation[index];
            }

            finish[i] = true;
            safe_sequence[completed++] = i;
            found = true;
        }

        if (!found)
        {
            free(work);
            free(finish);
            return false;
        }
    }

    free(work);
    free(finish);
    return true;
}
