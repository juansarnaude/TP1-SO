// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "./lib/view.h"

// In this view file we receive the name of the shared memory and the number of files processed as arguments, sent from md5sum.c
int main(int argc, char *argv[])
{
    char shared_memory_name[MAX_SHM_NAME_LENGTH];
    int number_of_files = 0;

    // If the data is sent trough arguments
    if (argc == 3)
    {
        // Get the shared memory name
        int j = 0;
        for (; argv[1][j] != '\0'; j++)
        {
            shared_memory_name[j] = argv[1][j];
        }
        shared_memory_name[j] = '\0';
        // Convert number of files char to int
        number_of_files = atoi(argv[2]);
    }
    // If the data is sent through STDIN
    else if (argc == 1)
    {
        char input[1024] = {0};
        read(STDIN_FILENO, input, 1024);
        // End input when '\n' is detected
        input[strcspn(input, "\n")] = '\0';
        // Filter data provided, first the shared memory name
        int i = 0;
        for (; input[i] != ' '; i++)
        {
            shared_memory_name[i] = input[i];
        }
        shared_memory_name[i] = '\0';

        // Converting string to int for number of files
        char number_of_files_str[5];
        int j = 0;
        for (; input[i] != '\0'; j++, i++)
        {
            number_of_files_str[j] = input[i];
        }
        number_of_files_str[j] = '\0';
        number_of_files = atoi(number_of_files_str);
    }
    else
    {
        perror("Please pass only two arguments, the name of the shared memory and the number of files that have been processed");
        exit(1);
    }

    shm_ADT shared_memory = malloc(sizeof(struct shm_CDT));
    if (connect_shm(shared_memory, shared_memory_name, number_of_files))
    {
        char buffer[number_of_files * FILE_SIZE_SHM + 1];
        while (read_shm(shared_memory, buffer))
        {
            printf("%s\n", buffer);
        }
    }
    delete_semaphores(shared_memory);
    delete_shm(shared_memory);

    return 0;
}