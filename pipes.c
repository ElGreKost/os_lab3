#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

int main() {
    int fd[2];
    pid_t pid;

    // Create pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork child process
    pid = fork();

    if (pid < 0) { // Error
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        int val;
        // Close unused write end of pipe
        close(fd[PIPE_WRITE_END]);
        // Read from pipe
        read(fd[PIPE_READ_END], &val, sizeof(int));
        // Increment value and print to terminal
        val++;
        printf("[Child] %d\n", val);
        // Close read end of pipe
        close(fd[PIPE_READ_END]);
        // Exit child process
        exit(EXIT_SUCCESS);
    } else { // Parent process
        int val;
        // Close unused read end of pipe
        close(fd[PIPE_READ_END]);
        // Read from terminal
        scanf("%d", &val);
        // Write to pipe
        write(fd[PIPE_WRITE_END], &val, sizeof(int));
        // Close write end of pipe
        close(fd[PIPE_WRITE_END]);
        // Wait for child process to terminate
        wait(NULL);
    }

    return 0;
}
