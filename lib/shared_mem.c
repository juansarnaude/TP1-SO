#include "shared_mem.h"

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
    sem_t *sem_write;
    sem_t *sem_read;
} shm_CDT;

// Main is only for testing purposes
//int main()
//{
//    shm_ADT javier = create_shm(12);
//    char test[64] = "e14a3ff5b5e67ede599cac94358e1028266626";
//    char *readVal;
//    write_shm(javier, test);
//    read_shm(javier, readVal);
//    printf("%s\n", readVal);
//    delete_shm(javier);
//}

// Function that creates the shared memory
shm_ADT create_shm(int file_qty)
{
    shm_ADT new_shm = malloc(sizeof(shm_CDT));

    char *name = "/shm_md5";

    // Creates the shared memory
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, (mode_t)MODE);
    if (shm_fd == -1)
    {
        perror("Creating the shared memory failed: shm_open failed");
        exit(1);
    }

    // Sets the size of the shared memory to file_qty * FILE_SIZE_SHM
    if (ftruncate(shm_fd, file_qty * FILE_SIZE_SHM) == -1)
    {
        perror("Creating the shared memory failed: ftruncate failed");
        exit(1);
    }

    // Maps the shared memory
    char *ptr = mmap(NULL, file_qty * FILE_SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Creating the shared memory failed: mmap failed");
        exit(1);
    }

    new_shm->shm_name = name;
    new_shm->shm_id = shm_fd;
    new_shm->shm_ptr = ptr;
    new_shm->size = file_qty * FILE_SIZE_SHM;
    new_shm->write_index = 0;
    new_shm->read_index = 0;

    new_shm->sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
    }

    new_shm->sem_write = sem_open(SEM_NAME_WRITE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
    }
    int fp = open("outsharedmem.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    int semvalue;
    sem_getvalue(new_shm->sem_write,&semvalue);
    dprintf(fp,"%d",semvalue);
    close(fp);
    new_shm->sem_read = sem_open(SEM_NAME_READ, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (new_shm->sem == SEM_FAILED)
    {
        delete_shm(new_shm);
        perror("Creating the shared memory failed: semaphor error");
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
    
    // Special semaphores for Write to avoid race condition and data overlaps
    
    sem_wait(shm->sem_write);
    

    // Check if theres enough space in the shared memory
    //if (shm->write_index + FILE_SIZE_SHM >= shm->size)
    
    //if (shm->write_index + FILE_SIZE_SHM >= shm->size)
    if (shm->write_index + buff_len >= shm->size)
    {
        perror("Not enough space in the shared memory");
        exit(1);
    }
    
    int shm_idx = shm->write_index;
    //shm->write_index += FILE_SIZE_SHM;
    shm->write_index += buff_len;
    if (sem_post(shm->sem_write) == -1)
    {
        shm->write_index -= buff_len;
        perror("Failed in write function");
        sem_close(shm->sem_write);
        sem_unlink(SEM_NAME_WRITE);
        delete_shm(shm);
        exit(1);
    }

    int i = 0;
    // Cortar antes, cuando no queda mas pid (el tema de los 0)
    for (; i < buff_len; i++)
    {
        shm->shm_ptr[shm_idx + i] = buff[i];
    }

    if (sem_post(shm->sem) == -1)
    {
        perror("Failed in write function");
        sem_close(shm->sem);
        sem_unlink(SEM_NAME);
        delete_shm(shm);
        exit(1);
    }

    

    return 1;
}

// Reads the shared memory.
int read_shm(shm_ADT shm, char *buff)
{
    sem_wait(shm->sem);

    // Special semaphores for Write to avoid race condition and data misreadings
    sem_wait(shm->sem_read);

    // Check if theres enough space in the shared memory
    if (shm->read_index + FILE_SIZE_SHM >= shm->size)
    {
        perror("Trying to read out of shared memory bounds");
        exit(1);
    }
    int shm_idx = shm->read_index;
    shm->read_index += FILE_SIZE_SHM;

    if (sem_post(shm->sem_read) == -1)
    {
        shm->read_index -= FILE_SIZE_SHM;
        perror("Failed in read function");
        sem_close(shm->sem_read);
        sem_unlink(SEM_NAME_READ);
        delete_shm(shm);
        exit(1);
    }

    int i = 0;

    // Cortar antes, cuando no queda mas pid
    for (; i < FILE_SIZE_SHM; i++)
    {
        buff[i] = shm->shm_ptr[shm_idx + i];
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
