#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

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

typedef struct shm_CDT *shm_ADT;

#define MODE 0666

#define SEM_NAME "smh_sem1"
#define SEM_NAME_WRITE "smh_sem_write1"
#define SEM_NAME_READ "smh_sem_read1"

#define PAGE_SIZE 4096
#define FILE_SIZE_SHM 64

shm_ADT create_shm(int file_qty);
int write_shm(shm_ADT shm, const char buff[FILE_SIZE_SHM],int buff_len);
int read_shm(shm_ADT shm, char *buff);
int delete_shm(shm_ADT shm);
int connect_shm(shm_ADT shared_memory, char * shared_memory_name, int file_qty);