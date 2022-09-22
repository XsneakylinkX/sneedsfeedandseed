#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define check_error(expr, usrMsg) \
    do                            \
    {                             \
        if (!(expr))              \
        {                         \
            perror(usrMsg);       \
            exit(EXIT_FAILURE);   \
        }                         \
    } while (0);

#define ARRAY_MAX 1024

typedef struct
{
    sem_t inDataReady;
    sem_t dataProcessed;
    char str[ARRAY_MAX];
} OsInputData;

char *strrev(char s[])
{
    int n = strlen(s);
    char *rev = malloc(n);
    for (int i = 0; i < n; i++)
        rev[i] = s[n-i-1];
    rev[n] = '\0';
    strcpy(s, rev);
    free(rev);
    return s;   
}

int main(int argc, char const *argv[])
{
    check_error(argc == 2, "argumenti");


    int memFd = shm_open(argv[1], O_RDWR, 0600);
    check_error(memFd != -1, "shm open");
    struct stat finfo;
    check_error(fstat(memFd, &finfo) != -1, "fstat");

    void *addr = mmap(NULL, finfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    check_error(addr != MAP_FAILED, "mmap");

    OsInputData *sharedMem = (OsInputData *)addr;

    close(memFd);

    check_error(sem_wait(&(sharedMem->inDataReady)) != -1, "sem wait");
    strrev(sharedMem->str);
    check_error(sem_post(&(sharedMem->dataProcessed)) != -1, "sem post");

    check_error(munmap(sharedMem, finfo.st_size) != -1, "munmap");


    return 0;
}
