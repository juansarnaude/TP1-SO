#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 1024
int main()
{
    char buffer[BUF_SIZE];
    while (scanf("%s", buffer) != EOF)
    {
        execlp("md5sum", "md5sum", buffer, NULL);
    }
    return 0;
}
