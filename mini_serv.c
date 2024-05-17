#include <string.h>
#include <unistd.h>

typedef struct s_client {
    int     id;
    char    msg[3024];
} t_client;

int     print_error(char *str, int fd, t_client* user) {
    write(fd, str, strlen(str));
    if (user)
        free(user);
    exit(1);
}

void    send_all(char* buf_client, fd_set main, int clientfd, int fdmax) {
    int fd = 0;
    while (fd <= fdmax) {
        if (FD_ISSET(fd, &main) && fd != clientfd) {
            if (send(fd, buf_client, strlen(buf_client), 0) == -1)
                print_error("Fatal Error\n", 2);
        }
        fd++;
    }
}

t_client*    client_crea(t_client *user, int clientfd, int fdmax) {
    user = realloc(user, sizeof(t_client) * fdmax);
    if (!user) {
        print_error("Fatal error\n", 2);
        exit(1);
    }
    user[clienfd].id = clientfd;
    user[clienfd].msg = NULL;
    return (user);
}

t_client*   client_del(t_client *user, int clientfd, int fdmax) {
    t_client*   tmp = malloc(sizeof(t_client) * fdmax - 1);

    int fd = 0;
    while (fd <= fdmax) {
        if (fd != clienfd) {
            tmp[fd].id = user[fd].id;
            tmp[fd].msg = user[fd].msg;
        }
        fd++;
    }
    free(user);
    return (tmp);
}

int main(int argc, char**argv) {
    
    t_client    *user;
    user = malloc(sizeof(t_client) * 1);
    char        buf_client[513];
    char        buf_recv[1024];

    fd_set      read, write, main;
    int         fdmax = 0;
    
    if (argc != 2)
        print_error("Wrong number of arguments\n", 2, user);
    int                 port = atoi(argv[1]); //parsing error

    int                 sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        print_error("Fatal error\n", 2, user);

    struct sockaddr_in  servaddr;
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
                    int clientfd = accept(sockfd, (struct sockaddr *)&serveraddr, &addrlen);
                    if (clientfd == -1)
                        continue ;
                    if (clientfd > fdmax)
                        fdmax = clientfd;
                    FD_SET(clientfd, &main);
                    client_crea(user, clientfd, fdmax);
                    sprintf(buf_client, "server: client %d just arrived\n", clientfd);
                    send_all(buf_client, main, clientfd, fdmax);
                    break ;
                }
                else {
                    int nbytes = recv(i, buf_recv, sizeof buf_recv - 1, 0);
                    buf_recv[nbytes] = '\0';

                    if (nbytes <= 0) {
                        sprintf(buf_client, "server: client %d just left\n", i);
                        send_all(buf_client, main, i, fdmax);
                        client_del(user, i, fdmax);
                        FD_CLR(i, &main);
                        close(i);
                        break ;
                    }
                    else {
                        
                    }
                }
            }
        }
    }

}

