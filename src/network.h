#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>  // Pour la taille de données (size_t)

#ifdef __cplusplus
extern "C" {
#endif

void send_data(const char *server_address, int port, const void *data, size_t size);
void receive_data(int port);

#endif // NETWORK_H