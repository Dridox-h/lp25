#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>  // Pour la taille de données (size_t)

#ifdef __cplusplus
extern "C" {
#endif

// Déclarations des fonctions
void send_data(const char *server_address, int port, const void *data, size_t size);
void receive_data(int port, void **data, size_t *size);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_H
