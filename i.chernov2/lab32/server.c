#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <ctype.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket"

void on_read(struct bufferevent *bev, void *ctx);
void on_accept(struct evconnlistener *listener, evutil_socket_t fd,
               struct sockaddr *address, int socklen, void *ctx);
void on_event(struct bufferevent *bev, short events, void *ctx);


int main() {
    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Error\n");
        return 1;
    }
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SOCKET_PATH);
    struct evconnlistener *listener = evconnlistener_new_bind(
            base, on_accept, NULL, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (struct sockaddr *)&server_address, sizeof(server_address));
    if (!listener) {
        fprintf(stderr, "Error\n");
        return 1;
    }
    printf("Waiting...\n");
    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);
    unlink(SOCKET_PATH);
    return 0;
}
void on_read(struct bufferevent *bev, void *ctx) {
    char buffer[1024];
    size_t n;
    while ((n = bufferevent_read(bev, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("Recieved: %s\n", buffer);
        for (size_t i = 0; i < n; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        printf("%s\n", buffer);
        bufferevent_write(bev, buffer, n);
    }
}

void on_event(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_ERROR)
        perror("Error");
    bufferevent_free(bev);
}

void on_accept(struct evconnlistener *listener, evutil_socket_t fd,
               struct sockaddr *address, int socklen, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, on_read, NULL, on_event, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}