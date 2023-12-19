#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <ctype.h>

#define SOCKET_PATH "/tmp/socket"

int main() {
    int server_fd, client_fd, max_fd, activity, i, valread;
    int sd, new_socket, max_clients = 30;
    int client_sockets[max_clients];
    fd_set readfds;
    struct sockaddr_un address;
    char buffer[1025];

    // Создание сокета
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сокета
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    // Привязка сокета к адресу
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Ожидание входящих соединений
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Инициализация массива клиентских сокетов
    for (i = 0; i < max_clients; i++) {
        client_sockets[i] = 0;
    }

    // Основной цикл сервера
    while (1) {
        // Инициализация множества файловых дескрипторов для мультиплексирования
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        // Добавление клиентских сокетов в множество
        for (i = 0; i < max_clients; i++) {
            sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_fd) {
                max_fd = sd;
            }
        }

        // Ожидание активности на сокетах
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        // Обработка входящих соединений
        if (FD_ISSET(server_fd, &readfds)) {
            if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Добавление нового клиентского сокета в массив
            for (i = 0; i < max_clients; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    break;
                }
            }
        }

        // Обработка данных от клиентов
        for (i = 0; i < max_clients; i++) {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    // Обработка отключения клиента
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&address);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Преобразование данных в верхний регистр и вывод
                    buffer[valread] = '\0';
                    for (int j = 0; j < valread; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    printf("%s\n", buffer);
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
