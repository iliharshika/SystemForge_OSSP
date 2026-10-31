#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "process.h"

int nanokernel_execute(char *args[])
{
    pid_t child_pid;
    int status;

    printf("\n[NanoKernel] Request received.\n");
    printf("[NanoKernel] Creating child process...\n");

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("[NanoKernel] fork failed");
        return 1;
    }

    if (child_pid == 0)
    {
        printf("[NanoKernel] Child process created.\n");
        printf("[NanoKernel] Child PID  : %d\n", getpid());
        printf("[NanoKernel] Parent PID : %d\n", getppid());

        printf("[NanoKernel] Executing using execvp()...\n");
        printf("[NanoKernel] Command     : %s\n\n", args[0]);

        execvp(args[0], args);

        perror("[NanoKernel] execvp failed");

        _exit(127);
    }

    printf("[NanoKernel] Parent PID : %d\n", getpid());
    printf("[NanoKernel] Child PID  : %d\n", child_pid);
    printf("[NanoKernel] Parent waiting for child...\n");

    if (waitpid(child_pid, &status, 0) == -1)
    {
        perror("[NanoKernel] waitpid failed");
        return 1;
    }

    printf("\n[NanoKernel] Child process terminated.\n");

    if (WIFEXITED(status))
    {
        int exit_status = WEXITSTATUS(status);

        printf("[NanoKernel] Normal termination : YES\n");
        printf("[NanoKernel] Exit status         : %d\n",
               exit_status);

        if (exit_status == 0)
        {
            printf("[NanoKernel] Result              : SUCCESS\n");
        }
        else
        {
            printf("[NanoKernel] Result              : COMMAND FAILED\n");
        }
    }
    else if (WIFSIGNALED(status))
    {
        printf("[NanoKernel] Normal termination : NO\n");
        printf("[NanoKernel] Signal              : %d\n",
               WTERMSIG(status));

        printf("[NanoKernel] Result              : TERMINATED BY SIGNAL\n");
    }

    return 0;
}
