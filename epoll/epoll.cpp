#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#define SERV_IP "127.0.0.1"
#define SERV_PORT 19961
#define BUF_SIZE 4000
#define MAX_EVENTS_NUM 1024

int init_server(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP, &servAddr.sin_addr.s_addr);

    if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        perror("bind failed\n");
        close(sockfd);
        exit(-1);
    }

    listen(sockfd, 128);   // 128为最大连接数

    return sockfd;
}

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;

    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(void) {
    struct sockaddr_in clientAddr;
    struct epoll_event ev, events[MAX_EVENTS_NUM];

    int sockfd = init_server();
    int epfd = epoll_create(1);
    
    if (epfd == -1) {
        perror("epoll_create\n");
        exit(-1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1){
        perror("epoll_ctl: sockfd");
        exit(-1);
    }

    while (1){
        int nready = epoll_wait(epfd, events, MAX_EVENTS_NUM, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].data.fd == sockfd) {
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);

                if (clientfd == -1) {
                    perror("accept");
                    exit(-1);
                }

                setnonblocking(clientfd);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev) == -1) {
                    perror("epoll_ctl: clientfd");
                    exit(-1);
                }
            }else {
                char recv_buffer[BUF_SIZE], send_buffer[BUF_SIZE];
                memset(recv_buffer, 0, BUF_SIZE);
                memset(send_buffer, 0, BUF_SIZE);

                int clientfd = events[i].data.fd;

                int n = recv(clientfd, recv_buffer, BUF_SIZE, MSG_DONTWAIT);

                strcat(send_buffer, "yfd: ");
                strcat(send_buffer, recv_buffer);

                send(clientfd, send_buffer, n + 5, MSG_DONTWAIT);
            }
        }
    }


    return 0;
}
