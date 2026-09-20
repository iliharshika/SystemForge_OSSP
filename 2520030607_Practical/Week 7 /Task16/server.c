#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 100

// Client to Server FIFOs
char *client_to_server[3] =
{
    "client1_to_server",
    "client2_to_server",
    "client3_to_server"
};

// Server to Client FIFOs
char *server_to_client[3] =
{
    "server_to_client1",
    "server_to_client2",
    "server_to_client3"
};

// Signal handler
void handle_signal(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("\nServer: SIGUSR1 received.\n");
        printf("Server: Client notification received.\n");
    }
    else if (sig == SIGCHLD)
    {
        while (waitpid(-1, NULL, WNOHANG) > 0)
        {
            printf("\nServer: Handler process terminated.\n");
        }
    }
    else if (sig == SIGINT)
    {
        printf("\nServer: SIGINT received.\n");
        printf("Server: Cleaning up FIFOs...\n");

        for (int i = 0; i < 3; i++)
        {
            unlink(client_to_server[i]);
            unlink(server_to_client[i]);
        }

        printf("Server: FIFOs removed.\n");
        exit(0);
    }
}

// Handler process for each client
void client_handler(int client_number)
{
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    int read_fd;
    int write_fd;

    printf("Handler %d: Started.\n", client_number);

    // Open Client-to-Server FIFO
    read_fd = open(client_to_server[client_number - 1], O_RDONLY);

    if (read_fd == -1)
    {
        perror("open client_to_server");
        exit(1);
    }

    // Open Server-to-Client FIFO
    write_fd = open(server_to_client[client_number - 1], O_WRONLY);

    if (write_fd == -1)
    {
        perror("open server_to_client");
        close(read_fd);
        exit(1);
    }

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0)
        {
            break;
        }

        buffer[bytes_read] = '\0';

        printf("Handler %d: Received: %s\n",
               client_number, buffer);

        // Prepare response
        snprintf(response,
                 sizeof(response),
                 "Server received your message");

        // Send response to client
        write(write_fd, response, strlen(response));
    }

    close(read_fd);
    close(write_fd);

    printf("Handler %d: Exiting.\n", client_number);

    exit(0);
}

int main()
{
    pid_t pid;

    printf("====================================\n");
    printf("      MULTI-CLIENT FIFO SERVER\n");
    printf("====================================\n");

    // Register signal handlers
    signal(SIGUSR1, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGCHLD, handle_signal);

    // Create six FIFOs
    for (int i = 0; i < 3; i++)
    {
        mkfifo(client_to_server[i], 0666);
        mkfifo(server_to_client[i], 0666);
    }

    printf("Server: All FIFOs created successfully.\n");

    // Create handler processes
    for (int i = 1; i <= 3; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            perror("fork");
            return 1;
        }

        if (pid == 0)
        {
            client_handler(i);
        }
    }

    printf("Server: All client handlers created.\n");
    printf("Server: Waiting for clients...\n");

    // Keep server running
    while (1)
    {
        pause();
    }

    return 0;
}
