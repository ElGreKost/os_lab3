#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

void P2C_write(const int *fd, long *num);

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
        for (int i = 0;i < 2; ++i) {
            int val;
            // Close unused write end of pipe
            close(fd[PIPE_WRITE_END]);
            // Read from pipe
            read(fd[PIPE_READ_END], &val, sizeof(int));
            // Increment value and print to terminal
            val++;
            printf("[Child] %d\n", val);

            write(fd[1], &val, sizeof(int));

        }
        // Close read end of pipe
        close(fd[PIPE_READ_END]);

    } else { // Parent process



        // here we read from terminal

        fd_set readfds;


        while (1) {

            // setup for select
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            int max_fd = fd[1];
            FD_SET(fd[0], &readfds);

            select(max_fd + 1, &readfds, NULL, NULL, NULL);


            if (FD_ISSET(STDIN_FILENO, &readfds)) { // reading from terminal
                char buffer[256];
                fgets(buffer, sizeof(buffer), stdin);
                char *endptr;
                long num = strtol(buffer, &endptr, 10);
                printf("num is %ld\n", num);


                // sending to pipe
                P2C_write(fd, &num);
            }
        }
    }

    return 0;
}

void P2C_write(const int *fd, long *num) {// Close unused read end of pipe
    close(fd[PIPE_READ_END]);
    // Read from terminal
    // Write to pipe
    write(fd[PIPE_WRITE_END], num, sizeof(int));
    // Close write end of pipe
    close(fd[PIPE_WRITE_END]);
}
