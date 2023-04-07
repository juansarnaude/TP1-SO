#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>

#define BUFF_LEN 64
#define SLAVES_QTY 4
#define MAX_FILES_SLAVE 2

#define READ 0
#define WRITE 1

// Funciones usadas dentro de este archivo
void create_slave_processes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves);
void print_error_msg(char *str);
void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed);

int main(int argc, char *argv[])
{
    // Verification of arguments
    if (argc <= 1)
    {
        char errmsg[] = "Invalid arguments quantity";
        print_error_msg(errmsg);
    }

    char **paths = calloc(argc - 1, sizeof(char *));
    if (paths == NULL)
    {
        char errmsg[] = "Failed to allocate memory for paths";
        print_error_msg(errmsg);
    }

    int file_qty;

    // Create an array of strings that will be used as an argument for the creation of the slave procceses
    for (file_qty = 1; file_qty <= argc - 1; file_qty++)
    {
        paths[file_qty - 1] = argv[file_qty];
    }
    file_qty--;

    // Call to the funciton that create slave processes
    int max_slaves = (SLAVES_QTY < ((file_qty + 1) / 2)) ? SLAVES_QTY : ((file_qty + 1) / 2);
    // fd para los pipes que vamos a crear
    int pipefd_w[max_slaves][2];
    int pipefd_r[max_slaves][2];

    create_slave_processes(pipefd_w, pipefd_r, max_slaves);

    int files_processed = 0;
    int files_read = 0;
    int to_read;
    char *buffer;
    int bytes_read;

    int fp; // BORRAR AL TERMINAR TESTEOS

    for (int i = 0; i < max_slaves; i++)
    {
        process_files(i, pipefd_w, paths, &files_processed);
    }

    while (files_read < file_qty)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = 0;

        for (int i = 0; i < max_slaves; i++)
        {
            max_fd = (pipefd_r[i][READ] > max_fd) ? pipefd_r[i][READ] : max_fd;
            FD_SET(pipefd_r[i][READ], &read_fds);
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Error in select");
            exit(1);
        }

        for (int i = 0; i < max_slaves; i++)
        {

            if (FD_ISSET(pipefd_r[i][READ], &read_fds))
            {
                // close(pipefd_r[i][WRITE]);
                bytes_read = read(pipefd_r[i][READ], buffer, 64);
                fp = open("out.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
                write(fp, buffer, bytes_read);
                close(fp);
                // close(pipefd_r[i][READ]);
                files_read++;
                process_files(i, pipefd_w, paths, &files_processed);
            }
        }
    }

    // Free memory allocated for paths
    free(paths);

    return 0;
}

void create_slave_processes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves)
{
    // Slave restriction and identifiers
    int n_slave;

    char *newargv[] = {"slave", NULL};
    char *newenv[] = {NULL};
    // Create all pipes nescesary for the comunication between md5sum and slaves
    for (n_slave = 0; n_slave < max_slaves; n_slave++)
    {
        if (pipe(pipefd_w[n_slave]) != 0 || pipe(pipefd_r[n_slave]) != 0)
        {
            char errmsg[] = "Failed to create pipes";
            print_error_msg(errmsg);
        }

        if (fork() == 0) // Child process
        {
            close(pipefd_w[n_slave][WRITE]);
<<<<<<< HEAD
            close(pipefd_r[n_slave][READ]);

            dup2(pipefd_w[n_slave][READ], STDIN_FILENO);
            dup2(pipefd_r[n_slave][WRITE], STDOUT_FILENO);

=======
            dup2(pipefd_w[n_slave][READ], STDIN_FILENO);
>>>>>>> 8d612c6c8a672821ef1dcf392d512a763c26272b
            close(pipefd_w[n_slave][READ]);

            dup2(pipefd_r[n_slave][WRITE], STDOUT_FILENO);
            close(pipefd_r[n_slave][READ]);
            close(pipefd_r[n_slave][WRITE]);

            execve("slave", newargv, newenv);
        }
    }
}

void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed)
{
    close(pipefd_w[n_slave][READ]); // Close the read end of the pipe in the parent process
    // Send a file to the slave process
    if (write(pipefd_w[n_slave][WRITE], paths[*files_processed], strlen(paths[*files_processed])) == -1)
    {
        char errmsg[] = "Failed to send paths to slave process";
        print_error_msg(errmsg);
    }
    if (write(pipefd_w[n_slave][WRITE], &"\n", 1) == -1)
    {
        char errmsg[] = "Failed to send paths to slave process";
        print_error_msg(errmsg);
    }
    (*files_processed)++;
    close(pipefd_w[n_slave][WRITE]);
}

void print_error_msg(char *str)
{
    perror(str);
    exit(0);
}