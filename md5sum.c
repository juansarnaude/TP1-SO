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
int amount_to_process(int file_qty, int files_processed);
void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed, int qty);
void close_pipes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves);

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
    char *buffer = malloc(BUFF_LEN * sizeof(char));
    int bytes_read;

    int fp; // BORRAR AL TERMINAR TESTEOS

    fd_set read_fds;

    for (int i = 0; i < max_slaves; i++)
    {
        process_files(i, pipefd_w, paths, &files_processed, amount_to_process(file_qty, files_processed));
    }

    while (files_read < file_qty)
    {
        int max_fd = FD_SETSIZE;
        FD_ZERO(&read_fds);
        for (int i = 0; i < max_slaves; i++)
        {
            FD_SET(pipefd_r[i][READ], &read_fds);
        }

        int fx = open("aux.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        write(fx, "b", 1);
        close(fx);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Error in select");
            exit(1);
        }

        write(fx, "b", 1);

        for (int i = 0; i < max_slaves; i++)
        {
            if (FD_ISSET(pipefd_r[i][READ], &read_fds))
            {
                bytes_read = read(pipefd_r[i][READ], buffer, 64);

                fp = open("out.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
                write(fp, buffer, bytes_read);
                close(fp);
                files_read++;

                if (files_processed < file_qty)
                {
                    process_files(i, pipefd_w, paths, &files_processed, amount_to_process(file_qty, files_processed));
                }
            }
            FD_CLR(pipefd_r[i][READ], &read_fds);
        }
    }

    close_pipes(pipefd_w, pipefd_r, max_slaves);

    // Free memory allocated for paths
    free(paths);
    free(buffer);

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
            dup2(pipefd_w[n_slave][READ], STDIN_FILENO);
            close(pipefd_w[n_slave][READ]);

            close(pipefd_r[n_slave][READ]);
            dup2(pipefd_r[n_slave][WRITE], STDOUT_FILENO);
            close(pipefd_r[n_slave][WRITE]);

            execve("slave", newargv, newenv);
        }
    }
}

int amount_to_process(int file_qty, int files_processed)
{
    if (files_processed > file_qty)
    {
        perror("Error in processing of files");
    }
    if (files_processed + MAX_FILES_SLAVE <= file_qty)
    {
        return MAX_FILES_SLAVE;
    }
    return file_qty - files_processed;
}

void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed, int qty)
{
    for (int i = 0; i < qty; i++)
    {
        //   Send a file to the slave process
        if (write(pipefd_w[n_slave][WRITE], paths[*files_processed], strlen(paths[*files_processed])) == -1)
        {
            char errmsg[] = "Failed to send paths to slave process";
            print_error_msg(errmsg);
        }

        // Each file will be written in a line
        if (write(pipefd_w[n_slave][WRITE], &"\n", 1) == -1)
        {
            char errmsg[] = "Failed to send paths to slave process";
            print_error_msg(errmsg);
        }

        (*files_processed)++;
    }
}

void print_error_msg(char *str)
{
    perror(str);
    exit(0);
}

void close_pipes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves)
{
    for (int i = 0; i < max_slaves; i++)
    {
        close(pipefd_r[i][READ]);
        close(pipefd_w[i][WRITE]);
    }
}