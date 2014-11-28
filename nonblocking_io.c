#include <stdio.h>
#include <errno.h>
#include <fcntl.h>


int main(void){
    char buf[2048];
    char *ptr;
    int nwrite, ntowrite=2048;

    fcntl(1, O_NONBLOCK);

    ptr = buf;
    while (ntowrite > 0){
        errno = 0;
        nwrite = write(1, ptr, 2048);
        printf("Wrote = %d, errno = %d\n", nwrite, errno);

        if (nwrite > 0){
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }
}
