#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../NanoKernel/process.h"

#define MAX_INPUT 512
#define MAX_ARGS 64

void print_banner(void)
{
    printf("\n");
    printf("====================================================\n");
    printf("                    FORGE OS\n");
    printf("             PROCESS MANAGEMENT SYSTEM\n");
    printf("====================================================\n");
    printf(" ShellForge   : User Command Interface\n");
    printf(" NanoKernel   : Process Management Module\n");
    printf(" Linux Kernel : Operating System Services\n");
    printf("====================================================\n");
}

void print_help(void)
{
    printf("\n================ FORGEOS COMMANDS =================\n");

    printf("\nNavigation:\n");
    printf("  pwd\n");
    printf("  ls\n");
    printf("  ls -l\n");
    printf("  ls -la\n");
    printf("  cd <directory>\n");
    printf("  cd ..\n");
    printf("  cd ~\n");
    printf("  clear\n");

    printf("\nFile Management:\n");
    printf("  touch <file>\n");
    printf("  cat <file>\n");
    printf("  less <file>\n");
    printf("  head <file>\n");
    printf("  tail <file>\n");
    printf("  cp <source> <destination>\n");
    printf("  mv <source> <destination>\n");
    printf("  rm <file>\n");

    printf("\nDirectory Management:\n");
    printf("  mkdir <directory>\n");
    printf("  rmdir <directory>\n");
    printf("  rm -r <directory>\n");

    printf("\nSearch:\n");
    printf("  find <location> <condition>\n");
    printf("  locate <filename>\n");
    printf("  which <command>\n");

    printf("\nUser Information:\n");
    printf("  whoami\n");
    printf("  who\n");
    printf("  id\n");
    printf("  hostname\n");

    printf("\nSystem Information:\n");
    printf("  date\n");
    printf("  cal\n");
    printf("  uname\n");
    printf("  uname -a\n");
    printf("  uptime\n");

    printf("\nDisk and Memory:\n");
    printf("  df -h\n");
    printf("  du -sh <directory>\n");
    printf("  free -h\n");

    printf("\nPermissions:\n");
    printf("  chmod <permission> <file>\n");
    printf("  chown <user> <file>\n");

    printf("\nProcess Management:\n");
    printf("  ps\n");
    printf("  top\n");
    printf("  kill <PID>\n");

    printf("\nHelp:\n");
    printf("  man <command>\n");
    printf("  <command> --help\n");
    printf("  history\n");

    printf("\nEditors:\n");
    printf("  nano <file>\n");
    printf("  gedit <file>\n");

    printf("\nBuilt-in ForgeOS commands:\n");
    printf("  help\n");
    printf("  cd\n");
    printf("  clear\n");
    printf("  exit\n");

    printf("\n====================================================\n");
    printf("ForgeOS also accepts other executable Linux commands.\n");
    printf("====================================================\n");
}

int parse_command(char *input, char *args[])
{
    int count = 0;
    char *token;

    token = strtok(input, " \t\n");

    while (token != NULL && count < MAX_ARGS - 1)
    {
        args[count] = token;
        count++;

        token = strtok(NULL, " \t\n");
    }

    args[count] = NULL;

    return count;
}

int main(void)
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    print_banner();

    printf("\nType 'help' to see the available command categories.\n");
    printf("Type 'exit' to shut down ForgeOS.\n");

    while (1)
    {
        char current_directory[256];

        if (getcwd(current_directory, sizeof(current_directory)) != NULL)
        {
            printf("\nforgeos:%s$ ", current_directory);
        }
        else
        {
            printf("\nforgeos$ ");
        }

        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n[ShellForge] Input closed.\n");
            break;
        }

        if (input[0] == '\n')
        {
            continue;
        }

        int argument_count = parse_command(input, args);

        if (argument_count == 0)
        {
            continue;
        }

        if (strcmp(args[0], "exit") == 0)
        {
            printf("\n[ShellForge] Shutting down ForgeOS...\n");
            printf("[ShellForge] Goodbye!\n");
            break;
        }

        if (strcmp(args[0], "help") == 0)
        {
            print_help();
            continue;
        }

        if (strcmp(args[0], "clear") == 0)
        {
            system("clear");
            continue;
        }

        if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL)
            {
                fprintf(stderr,
                        "[ShellForge] Usage: cd <directory>\n");
            }
            else if (chdir(args[1]) != 0)
            {
                perror("[ShellForge] cd failed");
            }

            continue;
        }

        printf("\n[ShellForge] Command received: ");

        for (int i = 0; i < argument_count; i++)
        {
            printf("%s ", args[i]);
        }

        printf("\n");

        nanokernel_execute(args);
    }

    return 0;
}
