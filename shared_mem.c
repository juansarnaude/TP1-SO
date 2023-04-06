#include "shared_mem.h"

shm_ADT create_shm(int file_qty);
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM]);
int read_shm(shm_ADT shm, char *buff);
int delete_shm(shm_ADT);

/*
In this memory we will have 32 bytes of the md5 value and the following 32 bytes of the pid,
summing up to 64 bytes per file proccesed.
*/

typedef struct shm_CDT
{
    int shm_id;
    char *shm_name;
    char *shm_ptr;

    int size;
    int write_index;
    int read_index;

    sem_t *sem;

} shm_CDT;

// Main is only for testing purposes
int main()
{
    shm_ADT javier = create_shm(12);
    char test[64] = "e14a3ff5b5e67ede599cac94358e1028266626";
    char *readVal;
    write_shm(javier, test);
    read_shm(javier, readVal);
    printf("%s\n", readVal);
    delete_shm(javier);
}

// Function that creates the shared memory
shm_ADT create_shm(int file_qty)
{
    shm_ADT new_shm = malloc(sizeof(shm_CDT));

    char *name = "/shm_md5";

    // Creates the shared memory
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, (mode_t)MODE);
    if (shm_fd == -1)
    {
        perror("shm_open failed");
        exit(1);
    }

    // Sets the size of the shared memory to file_qty * FILE_SIZE_SHM
    if (ftruncate(shm_fd, file_qty * FILE_SIZE_SHM) == -1)
    {
        perror("ftruncate failed");
        exit(1);
    }

    // Maps the shared memory
    char *ptr = mmap(NULL, file_qty * FILE_SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap failed");
        exit(1);
    }

    new_shm->shm_name = name;
    new_shm->shm_id = shm_fd;
    new_shm->shm_ptr = ptr;
    new_shm->size = file_qty * FILE_SIZE_SHM;
    new_shm->write_index = 0;
    new_shm->read_index = 0;
}

/*
Writes in the shared memory;
The md5 value will be written in the first 32 bytes and int the following 32 bytes the pid,
summing up to 64 bytes each time the function is called.
*/
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM])
{
    // Check if theres enough space in the shared memory
    if (shm->write_index + FILE_SIZE_SHM >= shm->size)
    {
        perror("Not enough spae in the shared memory");
        exit(1);
    }

    int i = 0;
    // Cortar antes, cuando no queda mas pid (el tema de los 0)
    for (; i < FILE_SIZE_SHM; i++)
    {
        shm->shm_ptr[shm->write_index + i] = buff[i];
    }

    shm->write_index += FILE_SIZE_SHM;

    return 1;
}

int read_shm(shm_ADT shm, char *buff)
{
    if (shm->read_index + FILE_SIZE_SHM >= shm->size)
    {
        perror("Trying to read out of bounds");
        exit(1);
    }

    int i = 0;

    // Cortar antes, cuando no queda mas pid
    for (; i < FILE_SIZE_SHM; i++)
    {
        buff[i] = shm->shm_ptr[shm->read_index + i];
    }

    shm->read_index += FILE_SIZE_SHM;
    buff[i] = '\0';

    return 1;
}

// Deletes the shared memory
int delete_shm(shm_ADT shm)
{
    if (munmap(shm->shm_ptr, shm->size) == -1)
    {
        perror("munmap failed");
        exit(1);
    }

    if (shm_unlink(shm->shm_name) == -1)
    {
        perror("shm_unlink failed");
        exit(1);
    }
    free(shm);

    return 1; // ALL GOOD
}
