#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

void send_data(const char *server_address, int port, const void *data, size_t size);
size_t receive_data(int port, size_t size);

#endif // NETWORK_H
