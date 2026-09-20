#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#define A2B "fifoA"
#define B2A "fifoB"

int main()
{
    int fd;
    char msg[100];
    pid_t pid;

    mkfifo(A2B, 0666);
    mkfifo(B2A, 0666);

    pid = fork();

    if (pid < 0)
    {
        perror("failed to create child");
        exit(0);
    }

    else if (pid == 0)
    {
        fd = open(A2B, O_RDONLY);

        while (1)
        {
            read(fd, msg, sizeof(msg));

            if (strcmp(msg, "bye") == 0)
                break;

            printf("\nUser A: %s\nUser B: ", msg);
            fflush(stdout);
        }

        close(fd);
    }

    else
    {
        fd = open(B2A, O_WRONLY);

        while (1)
        {
            printf("User B: ");
            fgets(msg, sizeof(msg), stdin);

            msg[strcspn(msg, "\n")] = '\0';

            write(fd, msg, strlen(msg) + 1);

            if (strcmp(msg, "bye") == 0)
                break;
        }

        close(fd);
    }

    return 0;
}
