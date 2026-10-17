#include <stdio.h>

#define MAX 10

int n, m;
int alloc[MAX][MAX], max[MAX][MAX], need[MAX][MAX];
int available[MAX], finish[MAX], seq[MAX];
int total = 0;

void findSafeSequences(int depth, int work[MAX]) {
    if (depth == n) {
        total++;

        printf("Sequence %d: ", total);
        for (int i = 0; i < n; i++)
            printf("P%d ", seq[i]);

        printf("\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        if (finish[i])
            continue;

        int j;
        for (j = 0; j < m; j++) {
            if (need[i][j] > work[j])
                break;
        }

        if (j == m) {
            int temp[MAX];

            for (j = 0; j < m; j++)
                temp[j] = work[j] + alloc[i][j];

            finish[i] = 1;
            seq[depth] = i;

            findSafeSequences(depth + 1, temp);

            finish[i] = 0;
        }
    }
}

int main(void) {
    int work[MAX];

    printf("Enter number of processes (max %d): ", MAX);
    scanf("%d", &n);

    printf("Enter number of resources (max %d): ", MAX);
    scanf("%d", &m);

    if (n < 1 || n > MAX || m < 1 || m > MAX) {
        printf("Invalid number of processes or resources.\n");
        return 1;
    }

    printf("\nEnter Allocation Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            scanf("%d", &alloc[i][j]);

    printf("\nEnter Maximum Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            scanf("%d", &max[i][j]);

    printf("\nEnter Available Resources (%d values):\n", m);
    for (int j = 0; j < m; j++)
        scanf("%d", &available[j]);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (alloc[i][j] < 0 || max[i][j] < alloc[i][j]) {
                printf("Invalid input: Maximum must be >= Allocation.\n");
                return 1;
            }

            need[i][j] = max[i][j] - alloc[i][j];
        }
    }

    for (int j = 0; j < m; j++) {
        if (available[j] < 0) {
            printf("Available resources cannot be negative.\n");
            return 1;
        }
        work[j] = available[j];
    }

    printf("\n1. NEED MATRIX:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }

    printf("\n2. ALL POSSIBLE SAFE SEQUENCES:\n");
    findSafeSequences(0, work);

    printf("\n3. SYSTEM STATUS:\n");

if (total > 0) {
    printf("System is in SAFE state.\n");
    printf("Total safe sequences: %d\n", total);
    printf("Unsafe state check: No unsafe state detected "
           "for the given input.\n");
} else {
    printf("System is in UNSAFE state.\n");
    printf("No safe execution sequence exists.\n");
    printf("Unsafe state check: No safe sequence was found.\n");
}

    return 0;
}
