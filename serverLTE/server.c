#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#include "../message.h"

#define MAXEVENTS 64
#define SUCCESS 0
#define ERROR -1

static int makeSocketNonBlocking(int sfd)
{
    int flags, result;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    result = fcntl(sfd, F_SETFL, flags);
    if (result == ERROR)
    {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static int createAndBind(char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int retVal, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    retVal = getaddrinfo(NULL, port, &hints, &result);
    if (retVal != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retVal));
        return ERROR;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == ERROR)
            continue;

        retVal = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (retVal == SUCCESS)
        {
            // bind successful
            break;
        }

        close(sfd);
    }

    if (rp == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return ERROR;
    }

    freeaddrinfo(result);

    return sfd;
}

void send_CRNTI(int socket, s_message message)
{
    int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;

    s_message response;
    response.message_type = random_access_response;

    response.message_value.message_response.rapid = (received_ra_rnti & 0b1100000000000000) >> 8;
    printf("sent value: %d\n", response.message_value.message_response.rapid);
    send(socket, &response, sizeof(response), 0);
}

int main(int argc, char *argv[])
{
    int sfd, result;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // argv[1] - port
    sfd = createAndBind(argv[1]);
    if (sfd == ERROR)
        abort();

    result = makeSocketNonBlocking(sfd);
    if (result == ERROR)
        abort();

    result = listen(sfd, SOMAXCONN);
    if (result == ERROR)
    {
        perror("listen");
        abort();
    }

    efd = epoll_create1(0);
    if (efd == ERROR)
    {
        perror("epoll_create");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    result = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (result == ERROR)
    {
        perror("epoll_ctl");
        abort();
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
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }

            else if (sfd == events[i].data.fd)
            {
                /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
                while (1)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept(sfd, &in_addr, &in_len);
                    if (infd == ERROR)
                    {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK))
                        {
                            /* We have processed all incoming
                             connections. */
                            break;
                        }
                        else
                        {
                            perror("accept");
                            break;
                        }
                    }

                    result = getnameinfo(&in_addr, in_len,
                                         hbuf, sizeof hbuf,
                                         sbuf, sizeof sbuf,
                                         NI_NUMERICHOST | NI_NUMERICSERV);
                    if (result == SUCCESS)
                    {
                        printf("\nAccepted connection on descriptor %d "
                               "(host=%s, port=%s)\n",
                               infd, hbuf, sbuf);
                        printf("\n");
                    }

                    /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                    result = makeSocketNonBlocking(infd);
                    if (result == ERROR)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    result = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (result == ERROR)
                    {
                        perror("epoll_ctl");
                        abort();
                    }
                }
                continue;
            }
            else
            {
                /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
                int done = 0;

                while (1)
                {

                    s_message message;

                    recv(events[i].data.fd, &message, sizeof(message), 0);

                    if (message.message_type == RA_RNTI)
                    {
                        send_CRNTI(events[i].data.fd, message);
                        exit(0);
                    }
                    else
                    {
                        printf("NOT RA_RNTI TYPE!\n");
                    }

                }

                if (done)
                {
                    printf("Closing connection on descriptor %d\n",
                           events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                     from the set of descriptors which are monitored. */
                    close(events[i].data.fd);
                }
            }
        }
    }

    free(events);

    close(sfd);

    return EXIT_SUCCESS;
}
