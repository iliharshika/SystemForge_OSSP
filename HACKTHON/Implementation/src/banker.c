#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "banker.h"

/*
 * Read the outstanding Request Matrix.
 *
 * This matrix represents the resources each process
 * is currently waiting for.
 */
bool input_outstanding_requests(BankerSystem *system)
{
    if (system == NULL ||
        system->outstanding_request == NULL ||
        system->process_names == NULL ||
        system->resource_names == NULL)
    {
        return false;
    }

    printf("\n========================================\n");
    printf("       OUTSTANDING REQUEST MATRIX\n");
    printf("========================================\n");

    printf("Enter the resources each process is currently waiting for.\n");
    printf("Enter 0 for a resource that is not currently requested.\n");

    for (int i = 0; i < system->process_count; i++)
    {
        printf("\nProcess: %s\n", system->process_names[i]);

        for (int j = 0; j < system->resource_count; j++)
        {
            size_t index =
                (size_t)i * system->resource_count + j;

            while (1)
            {
                printf("Outstanding request for %s: ",
                       system->resource_names[j]);

                if (scanf("%d", &system->outstanding_request[index]) != 1)
                {
                    printf("Invalid input. Enter an integer.\n");

                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF)
                    {
                    }

                    continue;
                }

                if (system->outstanding_request[index] < 0)
                {
                    printf("Request cannot be negative.\n");
                    continue;
                }

                break;
            }
        }
    }

    return true;
}

/*
 * Detect deadlock using the standard multiple-instance
 * resource deadlock-detection algorithm.
 *
 * Returns:
 *   true  - Detection completed successfully.
 *   false - Invalid data or memory allocation failure.
 *
 * deadlocked_processes[i] is true when process i remains
 * unfinished after the detection algorithm terminates.
 */
bool detect_deadlock(
    const BankerSystem *system,
    bool *deadlocked_processes)
{
    if (system == NULL ||
        deadlocked_processes == NULL ||
        system->available == NULL ||
        system->allocation == NULL ||
        system->outstanding_request == NULL ||
        system->process_count <= 0 ||
        system->resource_count <= 0)
    {
        return false;
    }

    int process_count = system->process_count;
    int resource_count = system->resource_count;

    int *work = calloc((size_t)resource_count, sizeof(int));
    bool *finish = calloc((size_t)process_count, sizeof(bool));

    if (work == NULL || finish == NULL)
    {
        free(work);
        free(finish);
        return false;
    }

    /*
     * Initially, Work equals Available.
     */
    for (int j = 0; j < resource_count; j++)
    {
        if (system->available[j] < 0)
        {
            free(work);
            free(finish);
            return false;
        }

        work[j] = system->available[j];
    }

    /*
     * A process holding no resources can initially be
     * considered finishable for this detection algorithm.
     */
    for (int i = 0; i < process_count; i++)
    {
        bool holds_resources = false;

        for (int j = 0; j < resource_count; j++)
        {
            size_t index =
                (size_t)i * resource_count + j;

            if (system->allocation[index] < 0 ||
                system->outstanding_request[index] < 0)
            {
                free(work);
                free(finish);
                return false;
            }

            if (system->allocation[index] > 0)
            {
                holds_resources = true;
            }
        }

        finish[i] = !holds_resources;
    }

    /*
     * Find processes whose outstanding requests can
     * be satisfied using the current Work vector.
     */
    bool progress;

    do
    {
        progress = false;

        for (int i = 0; i < process_count; i++)
        {
            if (finish[i])
            {
                continue;
            }

            bool can_finish = true;

            for (int j = 0; j < resource_count; j++)
            {
                size_t index =
                    (size_t)i * resource_count + j;

                if (system->outstanding_request[index] > work[j])
                {
                    can_finish = false;
                    break;
                }
            }

            if (!can_finish)
            {
                continue;
            }

            /*
             * Simulate the process completing and
             * releasing all resources it currently holds.
             */
            for (int j = 0; j < resource_count; j++)
            {
                size_t index =
                    (size_t)i * resource_count + j;

                if (system->allocation[index] > INT_MAX - work[j])
                {
                    free(work);
                    free(finish);
                    return false;
                }

                work[j] += system->allocation[index];
            }

            finish[i] = true;
            progress = true;
        }

    } while (progress);

    /*
     * Processes that remain unfinished are classified
     * as deadlocked by this detection model.
     */
    for (int i = 0; i < process_count; i++)
    {
        deadlocked_processes[i] = !finish[i];
    }

    free(work);
    free(finish);

    return true;
}

/* Display the deadlock-detection result. */
void display_deadlock_status(
    const BankerSystem *system,
    const bool *deadlocked_processes)
{
    if (system == NULL || deadlocked_processes == NULL)
    {
        return;
    }

    bool deadlock_found = false;

    printf("\n");
    printf("================================================\n");
    printf("              DEADLOCK DETECTION\n");
    printf("================================================\n");

    for (int i = 0; i < system->process_count; i++)
    {
        if (deadlocked_processes[i])
        {
            deadlock_found = true;
            break;
        }
    }

    if (deadlock_found)
    {
        printf("\nDEADLOCK DETECTED\n\n");
        printf("Affected processes:\n");

        for (int i = 0; i < system->process_count; i++)
        {
            if (deadlocked_processes[i])
            {
                printf("- %s\n", system->process_names[i]);
            }
        }
    }
    else
    {
        printf("\nDEADLOCK STATUS: NOT DETECTED\n\n");
        printf("All processes can finish under the supplied\n");
        printf("allocation and outstanding-request state.\n");
    }

    printf("\n================================================\n");
}