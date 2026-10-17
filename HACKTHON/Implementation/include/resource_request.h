#ifndef RESOURCE_REQUEST_H
#define RESOURCE_REQUEST_H

#include "banker.h"

typedef enum
{
    REQUEST_GRANTED,
    REQUEST_POSTPONED,
    REQUEST_REJECTED
} RequestStatus;

typedef struct
{
    int request_id;
    int process_id;
    int *resources;
    RequestStatus status;
    char reason[128];
} PendingRequest;

/* Submit and validate a resource request. */
bool request_resources(
    BankerSystem *system,
    int process_id,
    const int *request,
    int *safe_sequence
);

/* Add a request to the pending queue. */
bool enqueue_request(
    PendingRequest **queue,
    int *queue_count,
    int *next_request_id,
    int process_id,
    const int *resources,
    int resource_count
);

/* Process pending requests in order. */
void process_waiting_requests(
    BankerSystem *system,
    PendingRequest *queue,
    int queue_count
);

/* Release resources held by a process. */
bool release_resources(
    BankerSystem *system,
    int process_id,
    const int *release
);

/* Display the request history. */
void display_request_history(
    const PendingRequest *queue,
    int queue_count,
    const BankerSystem *system
);

/* Free all memory used by the queue. */
void free_request_queue(
    PendingRequest *queue,
    int queue_count
);

#endif