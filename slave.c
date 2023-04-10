#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 1024
int main()
{
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t bytes_readed;

    int pid = (int)getpid();
    int i = 0;

    while (bytes_readed = getline(&buffer, &bufsize, stdin) != -1)
    {
        buffer[bytes_readed + 1] = '\0';
        if (strlen(buffer) > 0)
        {

            if (fork() == 0)
            {
                execlp("md5sum", "md5sum", buffer, NULL);
            }
            wait(NULL);
        }
    }
    return 0;
}

// Separetes the buffer
/*for (; i < 2; i++)
{
    bytes_readed = getline(&buffer, &bufsize, stdin);
    if (bytes_readed > -1)
    {
        buffer[bytes_readed - 1] = '\0';

        if (fork() == 0)
        {
            execlp("md5sum", "md5sum", buffer, NULL);
        }


         int fp = open(strcat(strdup(buffer), ".txt"), O_RDWR | O_CREAT, 0644);

         if (fp == -1)
         {
             perror("Error opening file");
             return 1;
         }

         write(fp, buffer, bytes_readed);
         close(fp);

    }
}*/