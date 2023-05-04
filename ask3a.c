#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1


int main(int argc, char **argv) {

//ta orismata prepei na einai afstira eite 2 eite 3

    if (argc != 2 && argc != 3) {

        perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 0;

    }

//as doume an to n einai natural number

    char *endptr;
    long child_num = strtol(argv[1], &endptr, 10);
    int index_flag = 0;


    if (errno != 0 || *endptr != '\0' || child_num <= 0) {
        perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 0;

    }


//elegxoume an edwse round-robin/random/tipota

    if (argc == 3) {

        int result = strcmp(argv[2], "--random");

        if (!result) index_flag = 1;

        else {

            result = strcmp(argv[2], "--round-robin");

            if (result != 0) {

                perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                return 0;
            }

        }

    }

//telos elegxwn, o user exei dwsei swsta ton arithmo apo child processes me round-robin/random/tipota

    printf("I will create %ld child processes using %s\n", child_num, (index_flag == 0) ? "round-robin" : "random");


    pid_t pid;

    pid_t father_id = getpid();
    int fd_P2C[child_num][2];
    int fd_C2P[child_num][2];


    for (int i = 0; i < child_num; i++) {

        pid = fork();

//        pgid = getpgrp();

        if (pipe(fd_P2C[i]) == -1) {
            perror("parent->child pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(fd_C2P[i]) == -1) {
            perror("child->parent pipe");
            exit(EXIT_FAILURE);
        }

        // child's code todo how to send from parent to child, check next todo, error check for terminal and help - exit command in terminal
        if (pid == 0) {
            int val;

            close(fd_P2C[i][PIPE_WRITE_END]);
            // Redirect stdin to the read end of the pipe
//            dup2(fd_P2C[i][PIPE_READ_END], STDIN_FILENO);
            while (1) {
//                close(fd_P2C[i][PIPE_WRITE_END]);

//                if((read(fd_P2C[i][PIPE_READ_END], &val, sizeof(int))) == sizeof(int)) {
                (read(fd_P2C[i][PIPE_READ_END], &val, sizeof(int)))  ;;;

                    printf("[Child %d] [%d] Child received %d!\n", i, getpid(), val);
                    val++;
                    sleep(5);

                    close(fd_P2C[i][PIPE_READ_END]);

                    // FINISHED READING. NOW WRITING

                    close(fd_C2P[i][PIPE_READ_END]);

                    write(fd_C2P[i][PIPE_WRITE_END], &val, sizeof(int));
                    printf("[Child %d] [%d] Child Finished hard work, writing back %d\n", i, getpid(), val);

                    close(fd_C2P[i][PIPE_WRITE_END]);


            }
        }
    }

    for (int i = 0; i < child_num; i++)
        if (getpid() == father_id) {
            printf("Printing pipe array for child %d\n", i);
            printf("Parent to child\n");

            for (int j = 0; j < 2; ++j)
                printf(" %d ", fd_P2C[i][j]);


            printf("\n");
            printf("Child to parent\n");

            for (int j = 0; j < 2; ++j)
                printf(" %d ", fd_C2P[i][j]);


            printf("\n");
        }


    fd_set readfds;
//    int max_fd = fd_P2C[PIPE_READ_END] > STDIN_FILENO ? fd_P2C[PIPE_READ_END] : STDIN_FILENO;

    // Start being aware and on the lookout for input
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int max_fd = fd_C2P[child_num - 1][PIPE_WRITE_END]; // final fd created
        // printf("The max fd is %d\n", max_fd);
        for (int i = 0; i < child_num; ++i) FD_SET(fd_C2P[i][PIPE_READ_END], &readfds);

        int ready_fd_count = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready_fd_count == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) { // reading from terminal
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
                perror("Error while reading from terminal\n");
            else { //
                long num = strtol(buffer, &endptr, 10);
                // checks if help in buffer
                if (strstr(buffer, "help") != NULL)
                    printf("Type a number to send job to a child!\n");
                else if (strstr(buffer, "exit") != NULL) {// todo prints wrong output
                    printf("exit received\n");
                    kill(0, SIGTERM); // send SIGTERM to the parent process group
                    int status;
                    while (child_num > 0) {
                        pid = waitpid(-1, &status, 0);
                        if (pid == -1) {
                            if (errno == EINTR) {
                                continue;
                            }
                            perror("Error: waitpid failed\n");
                            exit(1);
                        } else if (pid > 0) {
                            printf("Waiting for %ld\n", child_num);
                            child_num--;
                        }
                    }
                    printf("All children terminated\n");
                    return 0;
                }
                // Check if there was an error during the conversion
                else if (errno == ERANGE || endptr == buffer || *endptr != '\n') {
                    // Invalid input
                    printf("Type a number to send job to a child!\n");
                }  else { //todo send the number to a child (round-robin or random) for now 0-th child
                    // Valid input
                    close(fd_P2C[0][PIPE_READ_END]);


                    int w_err_check = write(fd_P2C[0][PIPE_WRITE_END], &num, sizeof(num));
                    if (w_err_check == -1) perror("error in write\n");

                    close(fd_P2C[0][PIPE_WRITE_END]);

                    printf("Input is a number: %ld\n", num);
                }

                // printf("Read from terminal: %s", buffer);
            }
        }


        for (int i = 0; i < child_num; ++i)
            if (FD_ISSET(fd_P2C[i][PIPE_READ_END], &readfds)) {
                char buffer[256];
                ssize_t num_bytes_read = read(fd_P2C[i][PIPE_READ_END], buffer, sizeof(buffer));
                if (num_bytes_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (num_bytes_read == 0)
                    perror("Error reached EOF while reading from parent\n");
                else {
                    printf("Read from parent->child pipe: %.*s", (int) num_bytes_read, buffer);
                }
            }

    }

    return 0;
}
