#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "resource_manager.h"
#include "safety_algorithm.h"

/* Read a non-negative integer safely. */
static bool read_nonnegative_int(const char *prompt, int *value)
{
    char buffer[128];
    char extra;
    long number;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            return false;
        }

        if (sscanf(buffer, " %ld %c", &number, &extra) != 1 ||
            number < 0 || number > INT_MAX)
        {
            printf("Invalid input. Enter a non-negative integer.\n");
            continue;
        }

        *value = (int)number;
        return true;
    }
}

/* Read a positive integer safely. */
static bool read_positive_int(const char *prompt, int *value)
{
    while (1)
    {
        if (!read_nonnegative_int(prompt, value))
        {
            return false;
        }

        if (*value > 0)
        {
            return true;
        }

        printf("Value must be greater than zero.\n");
    }
}

/* Read a name without spaces. */
static bool read_name(const char *prompt, char *name, size_t size)
{
    printf("%s", prompt);

    if (size == 0 || scanf("%63s", name) != 1)
    {
        return false;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        /* Discard remaining characters on this input line. */
    }

    if (strlen(name) == 0 || strlen(name) >= size)
    {
        printf("Invalid name length.\n");
        return false;
    }

    return true;
}

/* Initialize all dynamically allocated system data. */
bool initialize_system(
    BankerSystem *system,
    int process_count,
    int resource_count)
{
    if (system == NULL ||
        process_count <= 0 ||
        resource_count <= 0)
    {
        return false;
    }

    if ((size_t)process_count > SIZE_MAX / sizeof(char *) ||
        (size_t)resource_count > SIZE_MAX / sizeof(char *) ||
        (size_t)process_count > SIZE_MAX / (size_t)resource_count)
    {
        printf("Requested system size is too large.\n");
        return false;
    }

    size_t cells =
        (size_t)process_count * (size_t)resource_count;

    if (cells > SIZE_MAX / sizeof(int))
    {
        printf("Requested matrix size is too large.\n");
        return false;
    }

    system->process_count = process_count;
    system->resource_count = resource_count;

    system->available = NULL;
    system->maximum = NULL;
    system->allocation = NULL;
    system->need = NULL;
    system->outstanding_request = NULL;
    system->process_names = NULL;
    system->resource_names = NULL;

    system->available =
        calloc((size_t)resource_count, sizeof(int));

    system->maximum = calloc(cells, sizeof(int));
    system->allocation = calloc(cells, sizeof(int));
    system->need = calloc(cells, sizeof(int));

    /* NEW: Allocate the outstanding-request matrix. */
    system->outstanding_request = calloc(cells, sizeof(int));

    system->process_names =
        calloc((size_t)process_count, sizeof(char *));

    system->resource_names =
        calloc((size_t)resource_count, sizeof(char *));

    if (system->available == NULL ||
        system->maximum == NULL ||
        system->allocation == NULL ||
        system->need == NULL ||
        system->outstanding_request == NULL ||
        system->process_names == NULL ||
        system->resource_names == NULL)
    {
        free_system(system);
        return false;
    }

    for (int i = 0; i < process_count; i++)
    {
        system->process_names[i] = calloc(64, sizeof(char));

        if (system->process_names[i] == NULL)
        {
            free_system(system);
            return false;
        }

        snprintf(system->process_names[i], 64, "P%d", i);
    }

    for (int j = 0; j < resource_count; j++)
    {
        system->resource_names[j] = calloc(64, sizeof(char));

        if (system->resource_names[j] == NULL)
        {
            free_system(system);
            return false;
        }

        snprintf(system->resource_names[j], 64, "R%d", j);
    }

    return true;
}

/* Free all allocated memory. */
void free_system(BankerSystem *system)
{
    if (system == NULL)
    {
        return;
    }

    if (system->process_names != NULL)
    {
        for (int i = 0; i < system->process_count; i++)
        {
            free(system->process_names[i]);
        }
    }

    if (system->resource_names != NULL)
    {
        for (int j = 0; j < system->resource_count; j++)
        {
            free(system->resource_names[j]);
        }
    }

    free(system->available);
    free(system->maximum);
    free(system->allocation);
    free(system->need);

    /* NEW: Free the outstanding-request matrix. */
    free(system->outstanding_request);

    free(system->process_names);
    free(system->resource_names);

    system->available = NULL;
    system->maximum = NULL;
    system->allocation = NULL;
    system->need = NULL;
    system->outstanding_request = NULL;
    system->process_names = NULL;
    system->resource_names = NULL;

    system->process_count = 0;
    system->resource_count = 0;
}

/* Input process and resource names. */
static bool input_names(BankerSystem *system)
{
    printf("\n========== PROCESS NAMES ==========\n");

    for (int i = 0; i < system->process_count; i++)
    {
        char prompt[100];

        snprintf(prompt, sizeof(prompt),
                 "Name for process P%d: ", i);

        if (!read_name(prompt, system->process_names[i], 64))
        {
            return false;
        }
    }

    printf("\n========== RESOURCE NAMES ==========\n");

    for (int j = 0; j < system->resource_count; j++)
    {
        char prompt[100];

        snprintf(prompt, sizeof(prompt),
                 "Name for resource R%d: ", j);

        if (!read_name(prompt, system->resource_names[j], 64))
        {
            return false;
        }
    }

    return true;
}

/* Input currently available resources. */
bool input_available_resources(BankerSystem *system)
{
    if (system == NULL || system->available == NULL)
    {
        return false;
    }

    printf("\n========== AVAILABLE RESOURCES ==========\n");

    for (int j = 0; j < system->resource_count; j++)
    {
        char prompt[160];

        snprintf(prompt, sizeof(prompt),
                 "Available %s: ", system->resource_names[j]);

        if (!read_nonnegative_int(prompt, &system->available[j]))
        {
            return false;
        }
    }

    return true;
}

/* Input the Allocation Matrix. */
bool input_allocation_matrix(BankerSystem *system)
{
    if (system == NULL || system->allocation == NULL)
    {
        return false;
    }

    printf("\n========== ALLOCATION MATRIX ==========\n");

    for (int i = 0; i < system->process_count; i++)
    {
        printf("\nProcess %s\n", system->process_names[i]);

        for (int j = 0; j < system->resource_count; j++)
        {
            char prompt[160];

            size_t index =
                (size_t)i * system->resource_count + j;

            snprintf(prompt, sizeof(prompt),
                     "Allocated %s: ", system->resource_names[j]);

            if (!read_nonnegative_int(
                    prompt, &system->allocation[index]))
            {
                return false;
            }
        }
    }

    return true;
}

/* Input the Maximum Matrix. */
bool input_maximum_matrix(BankerSystem *system)
{
    if (system == NULL || system->maximum == NULL)
    {
        return false;
    }

    printf("\n========== MAXIMUM MATRIX ==========\n");

    for (int i = 0; i < system->process_count; i++)
    {
        printf("\nProcess %s\n", system->process_names[i]);

        for (int j = 0; j < system->resource_count; j++)
        {
            char prompt[160];

            size_t index =
                (size_t)i * system->resource_count + j;

            snprintf(prompt, sizeof(prompt),
                     "Maximum %s: ", system->resource_names[j]);

            while (1)
            {
                if (!read_nonnegative_int(
                        prompt, &system->maximum[index]))
                {
                    return false;
                }

                if (system->maximum[index] >=
                    system->allocation[index])
                {
                    break;
                }

                printf(
                    "Maximum cannot be less than Allocation. "
                    "Enter the value again.\n");
            }
        }
    }

    return true;
}

/* Display the Available vector. */
void display_available(const BankerSystem *system)
{
    if (system == NULL || system->available == NULL)
    {
        return;
    }

    printf("\nAvailable Resources:\n");

    for (int j = 0; j < system->resource_count; j++)
    {
        printf("%-16s: %d\n",
               system->resource_names[j],
               system->available[j]);
    }
}

/* Display a matrix with resource-name columns. */
static void display_matrix(
    const BankerSystem *system,
    const int *matrix,
    const char *title)
{
    if (system == NULL || matrix == NULL)
    {
        return;
    }

    printf("\n========== %s ==========\n", title);

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
            size_t index =
                (size_t)i * system->resource_count + j;

            printf("%-12d", matrix[index]);
        }

        printf("\n");
    }
}

void display_allocation(const BankerSystem *system)
{
    display_matrix(
        system, system->allocation, "ALLOCATION MATRIX");
}

void display_maximum(const BankerSystem *system)
{
    display_matrix(
        system, system->maximum, "MAXIMUM MATRIX");
}

/* Display the entire current resource state. */
void display_system_state(const BankerSystem *system)
{
    if (system == NULL)
    {
        return;
    }

    printf("\n");
    printf("====================================================\n");
    printf("          SMART RESOURCE ALLOCATION SYSTEM\n");
    printf("====================================================\n");

    display_available(system);
    display_allocation(system);
    display_maximum(system);

    if (system->need != NULL)
    {
        display_matrix(system, system->need, "NEED MATRIX");
    }

    printf("\n====================================================\n");
}

/* Initialize the system from user-entered data. */
bool initialize_resource_data(BankerSystem *system)
{
    int process_count;
    int resource_count;

    if (system == NULL)
    {
        return false;
    }

    printf("\n========== SYSTEM INITIALIZATION ==========\n");

    if (!read_positive_int("Number of processes: ", &process_count) ||
        !read_positive_int("Number of resource types: ", &resource_count))
    {
        return false;
    }

    if (!initialize_system(system, process_count, resource_count))
    {
        printf("Unable to initialize the resource system.\n");
        return false;
    }

    /* Names are entered manually, as requested. */
    if (!input_names(system) ||
        !input_available_resources(system) ||
        !input_allocation_matrix(system) ||
        !input_maximum_matrix(system))
    {
        printf("Input cancelled or invalid.\n");
        free_system(system);
        return false;
    }

    /* Validate the total inventory for each resource type. */
    for (int j = 0; j < system->resource_count; j++)
    {
        long long total = system->available[j];

        for (int i = 0; i < system->process_count; i++)
        {
            size_t index = (size_t)i * (size_t)system->resource_count + (size_t)j;
            total += system->allocation[index];
        }

        if (total > INT_MAX)
        {
            printf("Resource total is too large.\n");
            free_system(system);
            return false;
        }
    }

    if (!calculate_need(system))
    {
        printf("Unable to calculate Need Matrix. Check Maximum and Allocation values.\n");
        free_system(system);
        return false;
    }

    printf("\nSystem data entered successfully.\n");
    return true;
}
