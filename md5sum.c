#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFF_LEN 256
#define SLAVES_QTY 4
#define MAX_FILES_SLAVE

// Funciones usadas dentro de este archivo
void create_slave_processes(char **paths, int paths_qty);
void print_error_msg(char *str);

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
        char errmsg[] = "Failed to allocate memory for paths\n";
        print_error_msg(errmsg);
    }

    int file_qty;

    // Create an array of strings that will be used as an argument for the creation of the slave procceses
    for (file_qty = 1; file_qty <= argc - 1; file_qty++)
    {
        paths[file_qty - 1] = argv[file_qty];
    }

    // Call to the funciton that create slave processes
    create_slave_processes(paths, file_qty);

    // Free memory allocated for paths
    free(paths);

    return 0;
}

void create_slave_processes(char **paths, int paths_qty)
{
    int n_slave;

    const char *sem1 = "sem_1";
    /*
        const char * sem2 = "sem_2";
        const char * sem3 = "sem_3";
        const char * sem4 = "sem_4";
    */

    char *newargv[] = {"slave", NULL};
    char *newenv[] = {NULL};

    for (n_slave = 1; n_slave <= paths_qty / 2; n_slave++)
    {
        int pipefd[2];
        pipe(pipefd);
        if (fork() == 0)
        {
            close(pipefd[STDOUT_FILENO]);
            close(STDIN_FILENO);
            // What's the input? What FD should we use so the slave process can read paths
            // dup(pipefd[0]);
            close(pipefd[0]);
            execve("slave", newargv, newenv);
            perror("Slave failed");
        }
    }
}

void print_error_msg(char *str)
{
    perror(str);
    exit(0);
}