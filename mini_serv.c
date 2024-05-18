#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct s_client {
    int     id;
    char    *msg;
} t_client;

int     print_error(char *str, int fd, t_client* user) {
    write(fd, str, strlen(str));
    if (user)
        free(user);
    exit(1);
}

void    send_all(char* buf_client, fd_set main, int clientfd, int fdmax, t_client* user, int sockfd) {
    int fd = 0;
    while (fd <= fdmax) {
        if (FD_ISSET(fd, &main) && fd != clientfd && fd != sockfd) {
            if (send(fd, buf_client, strlen(buf_client), 0) == -1)
                print_error("Fatal Error\n", 2, user);
        }
        fd++;
    }
}

t_client*    client_crea(t_client *user, int clientfd, int fdmax) {
    if (!user)
        user = malloc(sizeof(t_client) * (fdmax + 1));
    else if (user)
        user = realloc(user, sizeof(t_client) * (fdmax + 1));
    if (!user)
        print_error("Fatal error\n", 2, user);
    user[clientfd].id = clientfd;
    user[clientfd].msg = NULL;
    return (user);
}

void    delete_all(t_client* user, int fdmax, fd_set main, fd_set read) {
    for (int i = 0; i != fdmax; i++) {
        if (user[i].msg)
            free(user[i].msg);
        FD_CLR(i, &main);
        FD_CLR(i &read)
        close(i);
    }
    free(user);
}

int main(int argc, char** argv) {
    
    t_client    *user = NULL;

    char        buf_client[3000];
    char        buf_recv[3000];

    fd_set      read, main;
    int         fdmax = 0;
    
    if (argc != 2)
        print_error("Wrong number of arguments\n", 2, user);
    int                 port = atoi(argv[1]);

    int                 sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        print_error("Fatal error\n", 2, user);

    struct sockaddr_in  servaddr, cli;
    socklen_t           addrlen;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(port);
    
    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
        print_error("Fatal error\n", 2, user);
    
    FD_ZERO(&main);
    FD_ZERO(&read);

    if (listen(sockfd, 10) != 0)
        print_error("Fatal error\n", 2, user);
    
    FD_SET(sockfd, &main);
    FD_SET(sockfd, &read);
    fdmax = sockfd;

    int i = 0;
    while (1) {
        read = main;
        if (select(fdmax + 1, &read, NULL, NULL, NULL) == -1)
            print_error("Fatal error\n", 2, user);
        i = 0;
        while (i <= fdmax) {
            if (FD_ISSET(i, &read)) {
                if (i == sockfd) {
                    addrlen = sizeof cli;
                    int clientfd = accept(sockfd, (struct sockaddr *)&cli, &addrlen);
                    if (clientfd == -1)
                        continue ;
                    if (clientfd > fdmax)
                        fdmax = clientfd;
                    FD_SET(clientfd, &main);
                    user = client_crea(user, clientfd, fdmax);
                    sprintf(buf_client, "server: client %d just arrived\n", clientfd);
                    send_all(buf_client, main, clientfd, fdmax, user, sockfd);
                    break ;
                }
                else {
                    int nbytes = recv(i, buf_recv, sizeof buf_recv - 1, 0);
                    buf_recv[nbytes] = '\0';

                    if (nbytes <= 0) {
                        sprintf(buf_client, "server: client %d just left\n", i);
                        send_all(buf_client, main, i, fdmax, user, sockfd);
                        FD_CLR(i, &main);
                        close(i);
                        break ;
                    }
                    else {
                        int start = 0;
                        int j = 0;
                        if (user[i].msg) {
                            j = strlen(user[i].msg);
                            user[i].msg = realloc(user[i].msg, sizeof(char) * nbytes);
                            if (!user[i].msg)
                                print_error("Fatal error\n", 2, user);
                        }

                        while (start < nbytes) {
                            if (j == 0) {
                                user[i].msg = malloc(sizeof(char) * (nbytes));
                                if (!user[i].msg)
                                    print_error("Fatal error\n", 2, user);
                            }
                            user[i].msg[j] = buf_recv[start];
                            if (user[i].msg[j] == '\n') {
                                user[i].msg[j] = '\0';
                                sprintf(buf_client, "client %d: %s\n", i, user[i].msg);
                                send_all(buf_client, main, i, fdmax, user, sockfd);
                                bzero(user[i].msg, strlen(user[i].msg));
                                j = -1;
                            }
                            j++;
                            start++;
                        }
                        break ;
                    }
                }
            }
            i++;
        }
    }
    write(1, "fermeture du server", 19);
    delete_all(user, fdmax, main, read);
    return (0);
}
