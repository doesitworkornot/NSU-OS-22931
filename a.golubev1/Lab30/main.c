#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "connection.h"

int main(int argc, char* argv[]) {
  struct sockaddr_un addr;
  ssize_t ret;
  int data_socket;
  char buffer[BUFFER_SIZE];

  data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (data_socket == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

  ret = connect(data_socket, (const struct sockaddr*) &addr,
                sizeof(addr));
  if (ret == -1) {
    fprintf(stderr, "The server is down.\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; ++i) {
    ret = write(data_socket, argv[i], strlen(argv[i]) + 1);
    if (ret == -1) {
      perror("write");
      break;
    }
  }

  strcpy(buffer, "END");
  ret = write(data_socket, buffer, strlen(buffer) + 1);
  if (ret == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  ret = read(data_socket, buffer, sizeof(buffer));
  if (ret == -1) {
    perror("read");
    exit(EXIT_FAILURE);
  }

  buffer[sizeof(buffer) - 1] = 0;

  printf("Result = %s\n", buffer);

  close(data_socket);

  exit(EXIT_SUCCESS);
}
