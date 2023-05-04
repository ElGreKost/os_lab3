#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int num;
    int i;

    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  /* child process */
        close(fd[1]); /* close unused write end */

        for (i = 0; i < 2; i++) {
            read(fd[0], &num, sizeof(num));
            printf("Child process received: %d\n", num);
            num++;
            printf("Child process incremented: %d\n", num);
            write(fd[1], &num, sizeof(num));
        }

        close(fd[0]);
        close(fd[1]);
        _exit(EXIT_SUCCESS);

    } else { /* parent process */
        close(fd[0]); /* close unused read end */

        for (i = 0; i < 2; i++) {
            printf("Enter a number: ");
            scanf("%d", &num);
            printf("Parent process sending: %d\n", num);
            write(fd[1], &num, sizeof(num));
            printf("passed parent write\n");
            read(fd[0], &num, sizeof(num));
            printf("Parent process received: %d\n", num);
        }

        close(fd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }

    return 0;
}

