#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "resource_request.h"
#include "safety_algorithm.h"

#define HISTORY_FILE "data/request_history.csv"

/* ---------------------------------------------------------
   Helper: Check whether a resource vector is valid
   --------------------------------------------------------- */

static bool valid_resource_vector(const int *resources,
                                  int resource_count)
{
    if (resources == NULL || resource_count <= 0)
    {
        return false;
    }

    for (int j = 0; j < resource_count; j++)
    {
        if (resources[j] < 0)
        {
            return false;
        }
    }

    return true;
}

/* ---------------------------------------------------------
   Helper: Save one request attempt to CSV history
   --------------------------------------------------------- */

static void save_request_history(int request_id,
                                 const BankerSystem *system,
                                 int process_id,
                                 const int *request,
                                 RequestStatus status,
                                 const char *reason)
{
    FILE *file = fopen(HISTORY_FILE, "a");

    if (file == NULL)
    {
        printf("Warning: Could not open request history file.\n");
        return;
    }

    fprintf(file, "%d,%s,%s,\"",
            request_id,
            system->process_names[process_id],
            status == REQUEST_GRANTED ? "GRANTED" :
            status == REQUEST_POSTPONED ? "POSTPONED" :
            "REJECTED");

    for (int j = 0; j < system->resource_count; j++)
    {
        fprintf(file, "%s%d", j == 0 ? "" : " ", request[j]);
    }

    fprintf(file, "\",\"%s\"\n", reason);
    fclose(file);
}

/* ---------------------------------------------------------
   Resource Request Algorithm
   --------------------------------------------------------- */

bool request_resources(BankerSystem *system,
                       int process_id,
                       const int *request,
                       int *safe_sequence)
{
    if (system == NULL ||
        request == NULL ||
        safe_sequence == NULL ||
        system->process_count <= 0 ||
        system->resource_count <= 0 ||
        process_id < 0 ||
        process_id >= system->process_count ||
        system->available == NULL ||
        system->allocation == NULL ||
        system->maximum == NULL ||
        system->need == NULL)
    {
        printf("\nREQUEST STATUS: REJECTED\n");
        printf("Reason: Invalid request or system data.\n");
        return false;
    }

    int n = system->process_count;
    int m = system->resource_count;

    /* Step 1: Validate the requested resources. */

    for (int j = 0; j < m; j++)
    {
        if (request[j] < 0)
        {
            printf("\nREQUEST STATUS: REJECTED\n");
            printf("Reason: Resource quantities cannot be negative.\n");
            return false;
        }

        if (request[j] > system->need[process_id * m + j])
        {
            printf("\nREQUEST STATUS: REJECTED\n");
            printf("Reason: Request exceeds the process's remaining need.\n");
            return false;
        }
    }

    /* Step 2: Check currently available resources. */

    for (int j = 0; j < m; j++)
    {
        if (request[j] > system->available[j])
        {
            printf("\nREQUEST STATUS: POSTPONED\n");
            printf("Reason: Insufficient resources are currently available.\n");
            printf("The original system state remains unchanged.\n");
            return false;
        }
    }

    /* Step 3: Create a temporary state for the safety test. */

    int *trial_available = malloc((size_t)m * sizeof(int));
    int *trial_allocation = malloc((size_t)n * (size_t)m * sizeof(int));
    int *trial_need = malloc((size_t)n * (size_t)m * sizeof(int));

    if (trial_available == NULL ||
        trial_allocation == NULL ||
        trial_need == NULL)
    {
        free(trial_available);
        free(trial_allocation);
        free(trial_need);

        printf("\nREQUEST STATUS: REJECTED\n");
        printf("Reason: Insufficient memory for the safety test.\n");
        return false;
    }

    memcpy(trial_available,
           system->available,
           (size_t)m * sizeof(int));

    memcpy(trial_allocation,
           system->allocation,
           (size_t)n * (size_t)m * sizeof(int));

    memcpy(trial_need,
           system->need,
           (size_t)n * (size_t)m * sizeof(int));

    /* Apply the request only to the temporary state. */

    for (int j = 0; j < m; j++)
    {
        trial_available[j] -= request[j];
        trial_allocation[process_id * m + j] += request[j];
        trial_need[process_id * m + j] -= request[j];
    }

    BankerSystem trial_system = *system;

    trial_system.available = trial_available;
    trial_system.allocation = trial_allocation;
    trial_system.need = trial_need;

    /* Step 4: Run the safety algorithm on the temporary state. */

    int *trial_sequence = malloc((size_t)n * sizeof(int));

    if (trial_sequence == NULL)
    {
        free(trial_available);
        free(trial_allocation);
        free(trial_need);

        printf("\nREQUEST STATUS: REJECTED\n");
        printf("Reason: Could not allocate memory for the safety sequence.\n");
        return false;
    }

    bool safe = safety_check(&trial_system, trial_sequence);

    if (!safe)
    {
        printf("\nREQUEST STATUS: POSTPONED\n\n");
        printf("Safety check: FAILED\n");
        printf("System state: UNSAFE\n\n");
        printf("No complete safe sequence was found.\n\n");
        printf("Original resource state preserved.\n");

        free(trial_available);
        free(trial_allocation);
        free(trial_need);
        free(trial_sequence);

        return false;
    }

    /* Step 5: Commit the temporary state because it is safe. */

    memcpy(system->available,
           trial_available,
           (size_t)m * sizeof(int));

    memcpy(system->allocation,
           trial_allocation,
           (size_t)n * (size_t)m * sizeof(int));

    memcpy(system->need,
           trial_need,
           (size_t)n * (size_t)m * sizeof(int));

    memcpy(safe_sequence,
           trial_sequence,
           (size_t)n * sizeof(int));

    /* Step 6: Display the actual safe sequence. */

    printf("\nREQUEST STATUS: GRANTED\n\n");
    printf("Safety check: PASSED\n");
    printf("System state: SAFE\n\n");
    printf("Safe sequence:\n");

    for (int i = 0; i < n; i++)
    {
        printf("%s", system->process_names[safe_sequence[i]]);

        if (i < n - 1)
        {
            printf(" -> ");
        }
    }

    printf("\n");

    free(trial_available);
    free(trial_allocation);
    free(trial_need);
    free(trial_sequence);

    return true;
}

/* ---------------------------------------------------------
   Add a request to the waiting queue
   --------------------------------------------------------- */

bool enqueue_request(PendingRequest **queue,
                     int *queue_count,
                     int *next_request_id,
                     int process_id,
                     const int *resources,
                     int resource_count)
{
    if (queue == NULL ||
        queue_count == NULL ||
        next_request_id == NULL ||
        process_id < 0 ||
        !valid_resource_vector(resources, resource_count) ||
        *queue_count < 0 ||
        *next_request_id <= 0 ||
        *queue_count == INT_MAX)
    {
        printf("Unable to add the request to the queue.\n");
        return false;
    }

    PendingRequest *new_queue = realloc(
        *queue,
        (size_t)(*queue_count + 1) * sizeof(PendingRequest)
    );

    if (new_queue == NULL)
    {
        printf("Memory allocation failed while adding the request.\n");
        return false;
    }

    *queue = new_queue;

    int *request_copy = malloc(
        (size_t)resource_count * sizeof(int)
    );

    if (request_copy == NULL)
    {
        printf("Unable to store the requested resource quantities.\n");
        return false;
    }

    memcpy(request_copy,
           resources,
           (size_t)resource_count * sizeof(int));

    PendingRequest *item = &new_queue[*queue_count];

    item->request_id = *next_request_id;
    item->process_id = process_id;
    item->resources = request_copy;
    item->status = REQUEST_POSTPONED;

    snprintf(item->reason,
             sizeof(item->reason),
             "Waiting for processing");

    (*queue_count)++;
    (*next_request_id)++;

    printf("Request R%d added to the waiting queue.\n",
           item->request_id);

    return true;
}

/* ---------------------------------------------------------
   Process postponed requests
   --------------------------------------------------------- */

void process_waiting_requests(BankerSystem *system,
                              PendingRequest *queue,
                              int queue_count)
{
    if (system == NULL || queue_count < 0)
    {
        printf("Invalid waiting queue.\n");
        return;
    }

    printf("\n========== WAITING QUEUE PROCESSING ==========\n");

    bool found_pending = false;

    for (int i = 0; i < queue_count; i++)
    {
        PendingRequest *item = &queue[i];

        if (item->status != REQUEST_POSTPONED)
        {
            continue;
        }

        found_pending = true;

        printf("\nProcessing request R%d for %s...\n",
               item->request_id,
               system->process_names[item->process_id]);

        int *safe_sequence = malloc(
            (size_t)system->process_count * sizeof(int)
        );

        if (safe_sequence == NULL)
        {
            snprintf(item->reason,
                     sizeof(item->reason),
                     "Memory allocation failed");

            printf("Request remains postponed because of insufficient memory.\n");
            continue;
        }

        bool valid = true;

        for (int j = 0; j < system->resource_count; j++)
        {
            if (item->resources[j] >
                system->need[item->process_id *
                             system->resource_count + j])
            {
                valid = false;
                break;
            }
        }

        if (!valid)
        {
            item->status = REQUEST_REJECTED;

            snprintf(item->reason,
                     sizeof(item->reason),
                     "Request exceeds remaining process need");

            printf("REQUEST STATUS: REJECTED\n");
        }
        else
        {
            bool granted = request_resources(
                system,
                item->process_id,
                item->resources,
                safe_sequence
            );

            if (granted)
            {
                item->status = REQUEST_GRANTED;

                snprintf(item->reason,
                         sizeof(item->reason),
                         "Request granted; resulting state is safe");
            }
            else
            {
                item->status = REQUEST_POSTPONED;

                snprintf(item->reason,
                         sizeof(item->reason),
                         "Insufficient resources or unsafe trial state");
            }
        }

        save_request_history(
            item->request_id,
            system,
            item->process_id,
            item->resources,
            item->status,
            item->reason
        );

        free(safe_sequence);
    }

    if (!found_pending)
    {
        printf("There are no postponed requests to process.\n");
    }

    printf("\n========== QUEUE PROCESSING COMPLETE ==========\n");
}

/* ---------------------------------------------------------
   Release resources held by a process
   --------------------------------------------------------- */

bool release_resources(BankerSystem *system,
                       int process_id,
                       const int *release)
{
    if (system == NULL ||
        release == NULL ||
        process_id < 0 ||
        process_id >= system->process_count)
    {
        printf("Invalid process or release request.\n");
        return false;
    }

    int m = system->resource_count;

    /* Validate the complete release before changing anything. */

    for (int j = 0; j < m; j++)
    {
        int allocated = system->allocation[process_id * m + j];

        if (release[j] < 0)
        {
            printf("Release quantities cannot be negative.\n");
            return false;
        }

        if (release[j] > allocated)
        {
            printf("A process cannot release more resources than it holds.\n");
            return false;
        }

        if (system->available[j] > INT_MAX - release[j])
        {
            printf("Resource count would overflow. Release cancelled.\n");
            return false;
        }
    }

    /* Apply the release only after all quantities pass validation. */

    for (int j = 0; j < m; j++)
    {
        system->allocation[process_id * m + j] -= release[j];
        system->available[j] += release[j];

        system->need[process_id * m + j] =
            system->maximum[process_id * m + j] -
            system->allocation[process_id * m + j];
    }

    printf("\nResources released successfully by %s.\n",
           system->process_names[process_id]);

    display_available(system);

    return true;
}

/* ---------------------------------------------------------
   Display request history currently held in memory
   --------------------------------------------------------- */

void display_request_history(const PendingRequest *queue,
                             int queue_count,
                             const BankerSystem *system)
{
    if (system == NULL || queue_count < 0)
    {
        printf("Invalid request history.\n");
        return;
    }

    printf("\n================ REQUEST HISTORY ================\n");

    if (queue_count == 0)
    {
        printf("No requests have been recorded in this session.\n");
        return;
    }

    printf("%-10s %-15s %-15s %-25s\n",
           "Request ID",
           "Process",
           "Status",
           "Requested Resources");

    printf("------------------------------------------------------------------\n");

    for (int i = 0; i < queue_count; i++)
    {
        const char *status =
            queue[i].status == REQUEST_GRANTED ? "GRANTED" :
            queue[i].status == REQUEST_POSTPONED ? "POSTPONED" :
            "REJECTED";

        printf("R%-9d %-15s %-15s ",
               queue[i].request_id,
               system->process_names[queue[i].process_id],
               status);

        printf("[");

        for (int j = 0; j < system->resource_count; j++)
        {
            printf("%s%d",
                   j == 0 ? "" : ", ",
                   queue[i].resources[j]);
        }

        printf("]\n");
    }

    printf("==================================================\n");
}

/* ---------------------------------------------------------
   Free memory used by the waiting queue
   --------------------------------------------------------- */

void free_request_queue(PendingRequest *queue, int queue_count)
{
    if (queue == NULL)
    {
        return;
    }

    for (int i = 0; i < queue_count; i++)
    {
        free(queue[i].resources);
    }

    free(queue);
}