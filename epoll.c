/*
 * A TCP server based on epoll. The server will echo anything
 * receive from client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXEVENTS 10
#define PORT 9999


static int make_socket_non_blocking (int sfd)
{
    int nb;
    nb = 1;
    return ioctl(sfd, FIONBIO, &nb);
}

void main(int argc, char *argv[]){
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;
    struct sockaddr_in servaddr;

    sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sfd == -1|| sfd == 0){
        perror("Create listen socket failed.");
    }else{
        printf("Create socket success fd = %d\n", sfd);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        perror("Bind failed.");
    }

    s = make_socket_non_blocking (sfd);
    if (s == -1){
        perror("set nonblock failed.");
    }

    if (listen(sfd, 10) == -1){
        perror("Listen failed.");
    }

    efd = epoll_create(10);
    if (efd == -1) {
        perror("epoll_create failed");
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;

    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        perror("epoll_ctl");
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof event);

    /* The event loop */
    while (1)
    {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN)))
            {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            }

            else if (sfd == events[i].data.fd) {
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept (sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            printf("accept error: %d", errno);
                            break;
                        }
                        else {
                            perror ("accept");
                            break;
                        }
                    }

                    s = getnameinfo (&in_addr, in_len,
                            hbuf, sizeof hbuf,
                            sbuf, sizeof sbuf,
                            NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0)
                    {
                        printf("Accepted connection on descriptor %d "
                                "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                    /* Make the incoming socket non-blocking and add it to the
                       list of fds to monitor. */
                    s = make_socket_non_blocking(infd);
                    if (s == -1)
                        perror("set nonblock failed.");

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1)
                    {
                        perror ("epoll_ctl");
                    }
                }
                continue;
            }
            else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                int done = 0;
                printf("Oh, going to read from client.");

                while (1)
                {
                    ssize_t count;
                    char buf[12];

                    /*
                     * reading an nonblocking socket, return -1 and errno is
                     * EAGAIN means no data to read, but socket still open.
                     * return 0 means socket is close by the other end.
                    */
                    count = read(events[i].data.fd, buf, sizeof buf);
                    printf("Read byte: %d\n", count);
                    if (count == -1) {
                        /* If errno == EAGAIN, that means we have read all
                           data. So go back to the main loop. */
                        if (errno == EAGAIN|| errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror ("read");
                            done = 1;
                        }
                    }

                    else if (count == 0) {
                        /* End of file. The remote has closed the
                           connection. */
                        done = 1;
                        break;
                    }

                    /* Write the buffer to standard output */
                    s = write (1, buf, count);
                    if (s == -1) {
                        perror ("write");
                    }
                }

                if (done) {
                    printf ("Closed connection on descriptor %d\n",
                            events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                }
            }
        }
    }

    free (events);
    close (sfd);
    return EXIT_SUCCESS;
}
