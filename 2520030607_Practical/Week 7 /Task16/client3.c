#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 100

void handle_signal(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("\nClient 3: SIGUSR1 received from server.\n");
    }
}

int main()
{
    int write_fd;
    int read_fd;
    char message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    signal(SIGUSR1, handle_signal);

    printf("Client 3 started.\n");

    // Open Client-to-Server FIFO
    write_fd = open("client3_to_server", O_WRONLY);

    if (write_fd == -1)
    {
        perror("open client3_to_server");
        return 1;
    }

    // Open Server-to-Client FIFO
    read_fd = open("server_to_client3", O_RDONLY);

    if (read_fd == -1)
    {
        perror("open server_to_client3");
        close(write_fd);
        return 1;
    }

    printf("Client 3 connected to server.\n");

    printf("Enter message for server: ");
    fgets(message, sizeof(message), stdin);

    message[strcspn(message, "\n")] = '\0';

    // Send message to server
    write(write_fd, message, strlen(message));

    // Read response from server
    memset(response, 0, sizeof(response));

    int bytes_read = read(read_fd, response, sizeof(response) - 1);

    if (bytes_read > 0)
    {
        response[bytes_read] = '\0';

        printf("Client 3 received: %s\n", response);
    }

    close(write_fd);
    close(read_fd);

    printf("Client 3 finished.\n");

    return 0;
}
