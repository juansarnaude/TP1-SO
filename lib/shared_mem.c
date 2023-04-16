// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shared_mem.h"

/*
In this memory we will have 32 bytes of the md5 value and the following 32 bytes of the pid,
summing up to 64 bytes per file proccesed.
*/

// Function that creates the shared memory
shm_ADT create_shm(int file_qty, char *name)
{
    shm_ADT new_shm = malloc(sizeof(shm_CDT));
    if (new_shm == NULL)
    {
        perror("Creating the shared memory failed: allocate memory for shm failed");
        return 0;
    }
    // Creates the shared memory
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, (mode_t)MODE);
    if (shm_fd == -1)
    {
        perror("Creating the shared memory failed: shm_open failed");
        free(new_shm);
        return 0;
    }

    // Sets the size of the shared memory to file_qty * FILE_SIZE_SHM
    if (ftruncate(shm_fd, file_qty * FILE_SIZE_SHM) == -1)
    {
        perror("Creating the shared memory failed: ftruncate failed");
        free(new_shm);
        return 0;
    }

    // Maps the shared memory
    char *ptr = mmap(NULL, file_qty * FILE_SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Creating the shared memory failed: mmap failed");
        free(new_shm);

        return 0;
    }
    new_shm->shm_name = name;
    new_shm->shm_id = shm_fd;
    new_shm->shm_ptr = ptr;
    new_shm->size = file_qty * FILE_SIZE_SHM;
    new_shm->write_index = 0;
    new_shm->read_index = 0;

    new_shm->sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
        return 0;
    }

    new_shm->sem_write = sem_open(SEM_NAME_WRITE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
        return 0;
    }
    new_shm->sem_read = sem_open(SEM_NAME_READ, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
        return 0;
    }

    return new_shm;
}

/*
Writes in the shared memory;
The md5 value will be written in the first 32 bytes and int the following 32 bytes the pid,
summing up to 64 bytes each time the function is called.
*/
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM], int buff_len)
{
    printf("write\n");

    // Special semaphores for Write to avoid race condition and data overlaps
    sem_wait(shm->sem);
    sem_wait(shm->sem_write);

    // Check if theres enough space in the shared memory
    if (shm->write_index + FILE_SIZE_SHM > shm->size)
    {
        perror("Not enough space in the shared memory");
        return 0;
    }

    int shm_idx = shm->write_index;
    shm->write_index += FILE_SIZE_SHM;
    int i = 0;
    // Write the entire buffer and fill free space with NULL
    for (; i < buff_len; i++)
    {
        shm->shm_ptr[shm_idx + i] = buff[i];
    }
    for (; i < FILE_SIZE_SHM; i++)
        shm->shm_ptr[shm_idx + i] = '\0';
    if (sem_post(shm->sem_write) == -1)
    {
        shm->write_index -= buff_len;
        perror("Failed in write function");
        sem_close(shm->sem_write);
        sem_unlink(SEM_NAME_WRITE);
        delete_shm(shm);
        return 0;
    }
    if (sem_post(shm->sem) == -1)
    {
        perror("Failed in write function");
        sem_close(shm->sem);
        sem_unlink(SEM_NAME);
        delete_shm(shm);
        return 0;
    }
    if (sem_post(shm->sem_read) == -1)
    {
        perror("Failed in write function");
        sem_close(shm->sem);
        sem_unlink(SEM_NAME);
        delete_shm(shm);
        return 0;
    }
    return 1;
}

int connect_shm(shm_ADT shared_memory, char *shared_memory_name, int file_qty)
{
    // Asign the name of the shm to be connected
    shared_memory->shm_name = shared_memory_name;

    // We connect the desired shm
    int shm_fd = shm_open(shared_memory_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1)
    {
        perror("Opening existing shared memory failed: shm_open failed");
        return 0;
    }

    // Maps the memory
    shared_memory->shm_ptr = mmap(NULL, FILE_SIZE_SHM * file_qty, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory->shm_ptr == MAP_FAILED)
    {
        perror("Opening existing shared memory failed: mmap failed");
        return 0;
    }
    close(shm_fd);

    // Get the semaphore sem
    sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    if (sem == SEM_FAILED)
    {
        perror("Opening existing shared memory failed: semaphor error");
        return 0;
    }

    // Assign the semaphore to the sem of the shared_memory provided
    shared_memory->sem = sem;

    // Assig the read sem
    sem_t *sem_read = sem_open(SEM_NAME_READ, O_RDWR);
    if (sem_read == SEM_FAILED)
    {
        perror("Failed opening existing read semaphore");
        return 0;
    }
    sem_t *sem_write = sem_open(SEM_NAME_WRITE, O_RDWR);
    if (sem_write == SEM_FAILED)
    {
        perror("Failed opening existing read semaphore");
        return 0;
    }
    // Fill resting values
    shared_memory->sem_read = sem_read;
    shared_memory->sem_write = sem_write;
    shared_memory->read_index = 0;
    shared_memory->size = file_qty * FILE_SIZE_SHM;

    return 1;
}

// Reads the shared memory.
int read_shm(shm_ADT shm, char *buff)
{
    if (shm->size == shm->read_index)
        return 0;
    int semvalue1;
    sem_getvalue(shm->sem, &semvalue1);
    printf("%d\n", semvalue1);
    int semvalue;
    sem_getvalue(shm->sem_read, &semvalue);
    printf("%d\n", semvalue);
    // Special semaphores for Write to avoid race condition and data misreadings
    sem_wait(shm->sem_read);
    //  Wait for write process to end
    sem_wait(shm->sem);

    // Set shm index
    int shm_idx = shm->read_index;

    // Read the shm
    int i = 0;
    for (; i < FILE_SIZE_SHM; i++)
    {
        buff[i] = shm->shm_ptr[shm_idx + i];
    }
    shm->read_index += FILE_SIZE_SHM;
    buff[i] = '\0';

    // Enable the write process or read process
    if (sem_post(shm->sem) == -1)
    {
        shm->read_index -= FILE_SIZE_SHM;
        perror("Failed in read function");
        sem_close(shm->sem_read);
        sem_unlink(SEM_NAME_READ);
        delete_shm(shm);
        return 0;
    }
    // for (int i = 0; i < 1000; i++)
    // {
    // }
    return 1;
}

// Deletes the shared memory
void delete_shm(shm_ADT shm)
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
}

void delete_semaphores(shm_ADT shm)
{
    sem_close(shm->sem);
    sem_unlink(SEM_NAME);
    sem_close(shm->sem_write);
    sem_unlink(SEM_NAME_WRITE);
    sem_close(shm->sem_read);
    sem_unlink(SEM_NAME_READ);
}