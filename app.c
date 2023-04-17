// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "./lib/app.h"

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

    // Shared memory creation to communicate with view process
    shm_ADT shared_memory = create_shm(file_qty, "shared_memory");
    // Print useful information for view process
    printf("%s %d\n", shared_memory->shm_name, file_qty);
    sleep(2);

    // Calculate number of max slaves to be used
    int max_slaves = (SLAVES_QTY < ((file_qty + 1) / 2)) ? SLAVES_QTY : ((file_qty + 1) / 2);

    // Fds created for read and write pipes for each slave
    int pipefd_w[max_slaves][2];
    int pipefd_r[max_slaves][2];
    int pids[max_slaves];

    create_slave_processes(pipefd_w, pipefd_r, max_slaves, pids);

    // Set variables needed
    int files_processed = 0;
    int files_read = 0;
    char *buffer = malloc(BUFF_LEN * sizeof(char));
    fd_set read_fds;

    // Send  files to slaves
    for (int i = 0; i < max_slaves; i++)
    {
        process_files(i, pipefd_w, paths, &files_processed, amount_to_process(file_qty, files_processed));
    }

    // Check if there are slaves free and files to be processed
    while (files_read < file_qty)
    {
        int max_fd = 0;
        FD_ZERO(&read_fds);
        for (int i = 0; i < max_slaves; i++)
        {
            FD_SET(pipefd_r[i][READ], &read_fds);
            if (pipefd_r[i][READ] > max_fd)
                max_fd = pipefd_r[i][READ];
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            print_error_msg("Error in select");
        }

        for (int i = 0; i < max_slaves; i++)
        {
            // If theres something to be read from the pipe
            if (FD_ISSET(pipefd_r[i][READ], &read_fds))
            {

                int fp = open("resultado.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
                char md5_result[BUFF_LEN];
                int j;
                for (j = 0; read(pipefd_r[i][READ], buffer, 1) > 0 && *buffer != '\n'; j++)
                {
                    md5_result[j] = buffer[0];
                }
                md5_result[j] = '\0';

                // Format output in char array to_return
                char to_return[BUFF_LEN];
                int to_return_size = sprintf(to_return, "%d\t%s\n", pids[i], md5_result);
                write(fp, to_return, to_return_size);
                write_shm(shared_memory, to_return, to_return_size);
                close(fp);

                // File was read
                files_read++;

                // We send another file to the free slave to process
                if (files_processed < file_qty)
                {
                    process_files(i, pipefd_w, paths, &files_processed, 1);
                }
            }
        }
    }

    // Close slave pipes
    close_pipes(pipefd_w, pipefd_r, max_slaves);

    // Free memory allocated for paths
    free(paths);
    free(buffer);
    free(shared_memory);
    return 0;
}

void create_slave_processes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves, int pids[])
{

    char *newargv[] = {"slave", NULL};
    char *newenv[] = {NULL};
    // Slave restriction and identifiers
    // Create all pipes nescesary for the comunication between md5sum and slaves
    for (int n_slave = 0; n_slave < max_slaves; n_slave++)
    {
        if (pipe(pipefd_w[n_slave]) != 0 || pipe(pipefd_r[n_slave]) != 0)
        {
            char errmsg[] = "Failed to create pipes";
            print_error_msg(errmsg);
        }
        if ((pids[n_slave] = fork()) == 0) // Child process, assigning fork result to each pid
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
        print_error_msg("Error in processing of files");
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
    exit(1);
}

void close_pipes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves)
{
    for (int i = 0; i < max_slaves; i++)
    {
        close(pipefd_r[i][READ]);
        close(pipefd_w[i][WRITE]);
    }
}