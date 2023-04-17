#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include "shared_mem.h"

#define BUFF_LEN 64
#define SLAVES_QTY 4
#define MAX_FILES_SLAVE 2

#define READ 0
#define WRITE 1

void create_slave_processes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves, int pids[]);
void print_error_msg(char *str);
int amount_to_process(int file_qty, int files_processed);
void process_files(int n_slave, int pipefd_w[][2], char **paths, int *files_processed, int qty);
void close_pipes(int pipefd_w[][2], int pipefd_r[][2], int max_slaves);