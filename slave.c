// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "./lib/slave.h"

int main()
{
    char *buffer = NULL;
    size_t bufsize = 0;

    int pid = (int)getpid();

    while (getline(&buffer, &bufsize, stdin) != EOF)
    {
        buffer[strlen(buffer) - 1] = '\0';
        if (fork() == 0)
        {
            execlp("md5sum", "md5sum", buffer, NULL);
        }
    }
    return 0;
}
