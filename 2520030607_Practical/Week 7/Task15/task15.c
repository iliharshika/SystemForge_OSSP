#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int pipefd[2];
    pid_t producer, consumer;

    // Create pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    // Create Producer process
    producer = fork();

    if (producer == -1)
    {
        perror("fork");
        return 1;
    }

    if (producer == 0)
    {
        // Producer: ls -l

        close(pipefd[0]);

        // Redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);

        close(pipefd[1]);

        execlp("ls", "ls", "-l", NULL);

        perror("execlp ls");
        exit(1);
    }

    // Create Consumer process
    consumer = fork();

    if (consumer == -1)
    {
        perror("fork");
        return 1;
    }

    if (consumer == 0)
    {
        // Consumer: grep ".c"

        close(pipefd[1]);

        // Redirect stdin from pipe
        dup2(pipefd[0], STDIN_FILENO);

        close(pipefd[0]);

        execlp("grep", "grep", ".c", NULL);

        perror("execlp grep");
        exit(1);
    }

    // Parent closes both pipe ends
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for Producer
    waitpid(producer, NULL, 0);

    // Wait for Consumer
    waitpid(consumer, NULL, 0);

    printf("Parent: Both processes completed.\n");

    return 0;
}
