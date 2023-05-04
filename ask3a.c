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
    int fd_parent[child_num][2];
    int fd_child[child_num][2];

    for (int i = 0; i < child_num; i++) {

        pid = fork();
        if (pipe(fd_parent[i]) == -1) {
            perror("parent->child pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(fd_child[i]) == -1) {
            perror("child->parent pipe");
            exit(EXIT_FAILURE);
        }

        // child's code todo how to send from parent to child, check next todo, error check for terminal and help - exit command in terminal
        if (pid == 0) {
            int val;
            while (1) {
                read(fd_parent[i][PIPE_READ_END], &val, sizeof(int));
                printf("[Child] [%d] Child received %d!\n", i, getpid(), val);
                val++;
                sleep(5);
                write(fd_child[i][PIPE_WRITE_END], &val, sizeof(int));
                printf("[Child %d] [%d] Child Finished hard work, writing back %d\n", i, getpid(), val);
            }
        }

    }

    for (int i = 0; i < child_num; i++)

        if (getpid() == father_id) {
            printf("Printing pipe array for child %d\n", i);
            printf("Parent to child\n");

            for (int j = 0; j < 2; ++j)
                printf(" %d ", fd_parent[i][j]);


            printf("\n");
            printf("Child to parent\n");

            for (int j = 0; j < 2; ++j)
                printf(" %d ", fd_child[i][j]);


            printf("\n");
        }


    fd_set readfds;
//    int max_fd = fd_parent[PIPE_READ_END] > STDIN_FILENO ? fd_parent[PIPE_READ_END] : STDIN_FILENO;

    // Start being aware and on the lookout for input
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int max_fd = fd_child[child_num-1][PIPE_WRITE_END]; // final fd created
        printf("The max fd is %d\n", max_fd);
        for (int i = 0; i < child_num; ++i) FD_SET(fd_child[i][PIPE_READ_END], &readfds);

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

            // Check if there was an error during the conversion
                if (errno == ERANGE || endptr == buffer || *endptr != '\n') {
                    // Invalid input
                    printf("Type a number to send job to a child!\n");
                }  else { //todo send the number to a child (round-robin or random)
                    // Valid input
                    printf("Input is a number: %ld\n", num);
                }

                printf("Read from terminal: %s", buffer);
            }
        }


        for (int i = 0; i < child_num; ++i)
            if (FD_ISSET(fd_parent[i][PIPE_READ_END], &readfds)) {
                char buffer[256];
                ssize_t num_bytes_read = read(fd_parent[i][PIPE_READ_END], buffer, sizeof(buffer));
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
