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

    while (bytes_readed = getline(&buffer, &bufsize, stdin) != EOF)
    {
        buffer[bytes_readed + 1] = '\0';
        int fp = open("outa.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        dprintf(fp,"%d\t%s\tbytes_read = %d\n",pid,buffer,(int)bytes_readed);
        close(fp);

        if (fork() == 0)
        {
            execlp("md5sum", "md5sum", buffer, NULL);
        }
        wait(NULL);
    }
    return 0;
}
