#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "banker.h"
#include "resource_manager.h"
#include "safety_algorithm.h"
#include "resource_request.h"

/* Read a non-negative integer safely. */
static bool read_integer(const char *prompt, int *value)
{
    char line[128];
    char extra;
    long number;

    printf("%s", prompt);

    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        return false;
    }

    if (sscanf(line, " %ld %c", &number, &extra) != 1 ||
        number < 0 ||
        number > INT_MAX)
    {
        printf("Invalid input. Enter a non-negative integer.\n");
        return false;
    }

    *value = (int)number;
    return true;
}

/* Read a complete resource vector. */
static bool read_resource_vector(const BankerSystem *system,
                                 int *vector,
                                 const char *message)
{
    if (system == NULL || vector == NULL)
    {
        return false;
    }

    printf("%s\n", message);

    for (int j = 0; j < system->resource_count; j++)
    {
        char prompt[128];

        snprintf(prompt, sizeof(prompt),
                 "Enter quantity of %s: ",
                 system->resource_names[j]);

        if (!read_integer(prompt, &vector[j]))
        {
            return false;
        }
    }

    return true;
}

/* Find a process using its displayed name. */
static int select_process(const BankerSystem *system)
{
    int process_id;

    printf("\nAvailable processes:\n");

    for (int i = 0; i < system->process_count; i++)
    {
        printf("%d. %s\n", i, system->process_names[i]);
    }

    if (!read_integer("Enter process number: ", &process_id))
    {
        return -1;
    }

    if (process_id >= system->process_count)
    {
        printf("Invalid process number.\n");
        return -1;
    }

    return process_id;
}

/* Create and initialize the resource-allocation system. */
static bool setup_system(BankerSystem *system)
{
    printf("\n========== SYSTEM INITIALIZATION ==========\n");

    /*
     * initialize_resource_data() reads the process/resource counts,
     * allocates the system, and collects names and resource matrices.
     * Do not call initialize_system() separately here.
     */
    if (!initialize_resource_data(system))
    {
        printf("System initialization failed.\n");
        return false;
    }

    if (!calculate_need(system))
    {
        printf("Could not calculate Need. Check Maximum and Allocation values.\n");
        free_system(system);
        return false;
    }

    printf("\nSystem initialized successfully!\n");

    display_system_state(system);
    display_need(system);

    int *sequence = malloc(
        (size_t)system->process_count * sizeof(int)
    );

    if (sequence == NULL)
    {
        printf("Could not allocate memory for the safety sequence.\n");
        free_system(system);
        return false;
    }

    bool safe = safety_check(system, sequence);

    if (safe)
    {
        printf("\nInitial system state: SAFE\n");
        printf("Safe sequence: ");

        for (int i = 0; i < system->process_count; i++)
        {
            printf("%s", system->process_names[sequence[i]]);

            if (i < system->process_count - 1)
            {
                printf(" -> ");
            }
        }

        printf("\n");
    }
    else
    {
        printf("\nInitial system state: UNSAFE\n");
        printf("The initial state has no complete safe sequence.\n");
    }

    free(sequence);
    return true;
}

/* Submit a new resource request. */
static void submit_request(BankerSystem *system,
                           PendingRequest **queue,
                           int *queue_count,
                           int *next_request_id)
{
    int process_id = select_process(system);

    if (process_id < 0)
    {
        return;
    }

    int *request = calloc(
        (size_t)system->resource_count,
        sizeof(int)
    );

    if (request == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    if (!read_resource_vector(
            system,
            request,
            "Enter the resource quantities requested by this process:"))
    {
        free(request);
        return;
    }

    int *sequence = malloc(
        (size_t)system->process_count * sizeof(int)
    );

    if (sequence == NULL)
    {
        printf("Memory allocation failed.\n");
        free(request);
        return;
    }

    /*
     * First check whether the request is valid and can be considered.
     * If it is postponed, retain it in the waiting queue.
     */

    bool valid = true;

    for (int j = 0; j < system->resource_count; j++)
    {
        if (request[j] > system->need[
                process_id * system->resource_count + j])
        {
            valid = false;
            break;
        }
    }

    if (!valid)
    {
        printf("\nREQUEST STATUS: REJECTED\n");
        printf("Reason: Request exceeds the process's remaining need.\n");
    }
    else
    {
        bool granted = request_resources(
            system,
            process_id,
            request,
            sequence
        );

        if (!granted)
        {
            /*
             * The request may be postponed because resources are
             * unavailable or because the trial state is unsafe.
             */
            enqueue_request(
                queue,
                queue_count,
                next_request_id,
                process_id,
                request,
                system->resource_count
            );
        }
        else
        {
            printf("\nUpdated resource state:\n");
            display_system_state(system);
        }
    }

    free(sequence);
    free(request);
}

/* Release resources currently allocated to a process. */
static void release_process_resources(BankerSystem *system)
{
    int process_id = select_process(system);

    if (process_id < 0)
    {
        return;
    }

    int *release = calloc(
        (size_t)system->resource_count,
        sizeof(int)
    );

    if (release == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    if (read_resource_vector(
            system,
            release,
            "Enter the quantities to release:"))
    {
        if (release_resources(system, process_id, release))
        {
            display_system_state(system);
        }
    }

    free(release);
}

/* Run the separate deadlock-detection algorithm. */
static void run_deadlock_detection(BankerSystem *system)
{
    bool *deadlocked = calloc(
        (size_t)system->process_count,
        sizeof(bool)
    );

    if (deadlocked == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    printf("\n========== DEADLOCK DETECTION ==========\n");

    printf("Enter each process's outstanding resource requests.\n");
    printf("These are current outstanding requests, not maximum claims.\n");

    if (input_outstanding_requests(system))
    {
        if (detect_deadlock(system, deadlocked))
        {
            display_deadlock_status(system, deadlocked);
        }
    }

    free(deadlocked);
}

int main(void)
{
    BankerSystem system = {0};
    PendingRequest *queue = NULL;
    int queue_count = 0;
    int next_request_id = 1;
    int request_count = 0;
    int release_count = 0;

    printf("\nWelcome to Smart Resource Allocation Using Banker's Algorithm!\n");

    /* Stage 1: initialize once, using the user's manually entered data. */
    if (!setup_system(&system))
    {
        free_system(&system);
        return EXIT_FAILURE;
    }

    /* Ask for the total request count once, before processing requests. */
    while (!read_integer("\nHow many resource requests do you want to submit? ",
                         &request_count) || request_count < 1)
    {
        printf("Please enter a positive number of requests.\n");
    }

    /* Stage 2: process the requested number of requests consecutively. */
    for (int i = 0; i < request_count; i++)
    {
        printf("\n=================================================\n");
        printf("RESOURCE REQUEST %d OF %d\n", i + 1, request_count);
        printf("=================================================\n");

        submit_request(&system, &queue, &queue_count, &next_request_id);

        printf("\nResource state after request %d:\n", i + 1);
        display_system_state(&system);
    }

    /* Stage 3: automatically retry pending requests once after the batch. */
    if (queue_count > 0)
    {
        printf("\n========== AUTOMATIC WAITING-QUEUE RETRY ==========\n");
        process_waiting_requests(&system, queue, queue_count);
        display_system_state(&system);
    }

    /*
     * Stage 4: allow releases after the request batch. Each release
     * automatically triggers another waiting-queue retry.
     */
    while (!read_integer(
               "\nHow many resource-release operations do you want to perform? ",
               &release_count))
    {
        printf("Enter a valid non-negative number.\n");
    }

    for (int i = 0; i < release_count; i++)
    {
        printf("\n=================================================\n");
        printf("RESOURCE RELEASE %d OF %d\n", i + 1, release_count);
        printf("=================================================\n");

        release_process_resources(&system);

        printf("\nAutomatically retrying waiting requests after release...\n");
        process_waiting_requests(&system, queue, queue_count);
        display_system_state(&system);
    }

    /* Stage 5: run the separate deadlock detector. */
    printf("\n========== DEADLOCK DETECTION ==========\n");
    run_deadlock_detection(&system);

    /* Stage 6: display request history and the final state automatically. */
    printf("\n========== REQUEST HISTORY ==========\n");
    display_request_history(queue, queue_count, &system);

    printf("\n========== FINAL RESOURCE STATE ==========\n");
    display_system_state(&system);

    printf("\nAll requested operations have completed. Exiting.\n");

    free_request_queue(queue, queue_count);
    free_system(&system);
    return EXIT_SUCCESS;
}
