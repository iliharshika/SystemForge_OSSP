#include <stdio.h>
#include <stdlib.h>

struct Student {
    int id;
    float marks;
};

int main(void) {
    int n = 3;

    struct Student *s = malloc(n * sizeof(struct Student));

    if (s == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf("Memory allocated successfully!\n");

    for (int i = 0; i < n; i++) {
        s[i].id = i + 1;
        s[i].marks = 85.5f + i;
    }

    printf("\nStudent Details:\n");

    for (int i = 0; i < n; i++) {
        printf("ID: %d, Marks: %.2f\n",
               s[i].id, s[i].marks);
    }

    free(s);
    s = NULL;

    printf("\nMemory freed successfully!\n");

    return 0;
}
