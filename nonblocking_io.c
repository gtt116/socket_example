#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_BYTE 204800

int main(void){
    char buf[MAX_BYTE];
    char *ptr;
    int nwrite, ntowrite=MAX_BYTE;

    memset(buf, 'c', MAX_BYTE);
    //fcntl(1, O_NONBLOCK);

    ptr = buf;
    while (ntowrite > 0){
        errno = 0;
        nwrite = write(1, ptr, ntowrite);
        printf("Wrote = %d, errno = %d\n", nwrite, errno);

        if (nwrite > 0){
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }
}
