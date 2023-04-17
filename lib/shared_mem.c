// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shared_mem.h"

// In this memory we will have 64 bytes per file buffer

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

    // Assign previous values to shared memory
    new_shm->shm_name = name;
    new_shm->shm_ptr = ptr;
    new_shm->size = file_qty * FILE_SIZE_SHM;
    new_shm->write_index = 0;
    new_shm->read_index = 0;

    // Creation of semaphores
    // sem syncs read and write operations to avoid race conditions
    new_shm->sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
        return 0;
    }
    // sem_read syncs read operations to avoid race conditions, initialized with 0 value
    new_shm->sem_read = sem_open(SEM_NAME_READ, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
        return 0;
    }

    return new_shm;
}

// Write to shared memory
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM], int buff_len)
{
    // Special semaphores for Write to avoid race condition and data overlaps
    sem_wait(shm->sem);

    // Check if theres enough space in the shared memory
    if (shm->write_index + FILE_SIZE_SHM > shm->size)
    {
        perror("Not enough space in the shared memory");
        return 0;
    }

    int shm_idx = shm->write_index;
    shm->write_index += FILE_SIZE_SHM;
    // Write the entire buffer and fill free space with NULL
    int i = 0;
    for (; i < buff_len; i++)
    {
        shm->shm_ptr[shm_idx + i] = buff[i];
    }
    for (; i < FILE_SIZE_SHM; i++)
        shm->shm_ptr[shm_idx + i] = '\0';

    if (sem_post(shm->sem) == -1)
    {
        perror("Failed in write function");
        sem_close(shm->sem);
        sem_unlink(SEM_NAME);
        delete_shm(shm);
        return 0;
    }
    // sem_read initial value is 0, once written in shared memory the read process can be enabled
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

    // Open semaphores
    sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    if (sem == SEM_FAILED)
    {
        perror("Opening existing shared memory failed: semaphor error");
        return 0;
    }
    sem_t *sem_read = sem_open(SEM_NAME_READ, O_RDWR);
    if (sem_read == SEM_FAILED)
    {
        perror("Failed opening existing read semaphore");
        return 0;
    }

    // Fill the shm values
    shared_memory->sem = sem;
    shared_memory->sem_read = sem_read;
    shared_memory->read_index = 0;
    shared_memory->size = file_qty * FILE_SIZE_SHM;

    return 1;
}

// Reads the shared memory.
int read_shm(shm_ADT shm, char *buff)
{
    // When the whole shm was read, returns 0
    if (shm->size == shm->read_index)
        return 0;

    // Special semaphores for read to avoid race condition and data overlaps
    sem_wait(shm->sem_read);
    sem_wait(shm->sem);

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
    return 1;
}

// Deletes the shared memory
void delete_shm(shm_ADT shm)
{
    munmap(shm->shm_ptr, shm->size);
    shm_unlink(shm->shm_name);
    free(shm);
}

// Deletes semaphores
void delete_semaphores(shm_ADT shm)
{
    sem_close(shm->sem);
    sem_unlink(SEM_NAME);
    sem_close(shm->sem_read);
    sem_unlink(SEM_NAME_READ);
}