#include <stdio.h>
#include <unistd.h>
#include <openssl/md5.h>

int main()
{
    unsigned char c[MD5_DIGEST_LENGTH];
    char *filename="prueba2.c";
    int i;
    FILE *inFile = fopen (filename, "rb");//rb for non text files
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    printf("PID:%d ",getpid());
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf (" %s\n", filename);
    fclose (inFile);
    return 0;
}
