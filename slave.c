#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void md5_algorithm(char *filename);

#define FILE_SEPARATOR 1

int main()
{
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t bytes_readed;

    int i = 0;
    // Separetes the buffer
    for (; i < 2; i++)
    {
        bytes_readed = getline(&buffer, &bufsize, stdin);
        if (bytes_readed > -1)
        {
            buffer[bytes_readed - 1] = '\0';

            int fp = open(strcat(strdup(buffer), ".txt"), O_RDWR | O_CREAT, 0644);

            if (fp == -1)
            {
                perror("Error opening file");
                return 1;
            }

            write(fp, buffer, bytes_readed);
            close(fp);
        }
    }

    // file = strtok(buffer, &separator);

    // At max it will receive two files

    return 0;
}

void md5_algorithm(char *filename)
{
    return;
}
