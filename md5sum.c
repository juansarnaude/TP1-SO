#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BUFF_LEN 256
#define SLAVES_QTY 4
#define MAX_FILES_SLAVE 2

#define READ 0
#define WRITE 1

// Funciones usadas dentro de este archivo
void create_slave_processes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves);
void print_error_msg(char *str);
void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed, int amount_proccess);

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
    int i = 0;
    for (; i < max_slaves && files_processed < file_qty; i++)
        process_files(i, pipefd_w, paths, &files_processed, (((file_qty - files_processed) == 1) ? 1 : MAX_FILES_SLAVE));
    // Free memory allocated for paths

    char buffer[BUFF_LEN];
    int bytes_read;
    int x;
    for (x = 0; x < max_slaves; x++)
    {
        close(pipefd_r[x][WRITE]); // Close the write end of the pipe in the parent process
        while ((bytes_read = read(pipefd_r[x][READ], buffer, BUFF_LEN)) > 0)
        {
            // Write to out.txt
            int fp = open("out.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
            write(fp, buffer, bytes_read);
            close(fp);
        }
        close(pipefd_r[x][READ]);
    }

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
            dup2(pipefd_w[n_slave][READ], STDIN_FILENO);
            close(pipefd_w[n_slave][READ]);

            dup2(pipefd_r[n_slave][WRITE], STDOUT_FILENO);
            close(pipefd_r[n_slave][READ]);
            close(pipefd_r[n_slave][WRITE]);

            execve("slave", newargv, newenv);
        }
    }
}

void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed, int amount_proccess)
{
    close(pipefd_w[n_slave][READ]); // Close the read end of the pipe in the parent process
    // Send files to slave processes
    int i = 0;
    for (; i < amount_proccess; i++)
    {
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
    }
    close(pipefd_w[n_slave][WRITE]);
}

void print_error_msg(char *str)
{
    perror(str);
    exit(0);
}