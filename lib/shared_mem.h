#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

typedef struct shm_CDT
{
    char *shm_name;
    char *shm_ptr;

    int size;
    int write_index;
    int read_index;

    sem_t *sem;
    sem_t *sem_read;
} shm_CDT;

typedef struct shm_CDT *shm_ADT;

#define MODE 0666

#define SEM_NAME "shm_sem"
#define SEM_NAME_READ "shm_sem_read"

#define PAGE_SIZE 4096
#define FILE_SIZE_SHM 64

shm_ADT create_shm(int file_qty, char *name);
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM], int buff_len);
int read_shm(shm_ADT shm, char *buff);
void delete_shm(shm_ADT shm);
void delete_semaphores(shm_ADT shm);
int connect_shm(shm_ADT shared_memory, char *shared_memory_name, int file_qty);