#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void send_data(const char *server_address, int port, const void *data, size_t size) {
    // Créer une socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création de la socket");
        return;
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, server_address, &server.sin_addr) <= 0) {
        perror("Adresse IP invalide ou non supportée");
        close(sock);
        return;
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        close(sock);
        return;
    }

    // Envoi des données
    size_t sent = send(sock, data, size, 0);
    if (sent != size) {
        perror("Erreur lors de l'envoi des données");
    } else {
        printf("Données envoyées : %zu octets\n", sent);
    }

    // Fermer la socket
    close(sock);
}

void receive_data(int port, void **data, size_t *size) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024];
    size_t total_read = 0;
    size_t buffer_size = sizeof(buffer);

    // Créer une socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Attacher la socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erreur lors de l'attachement de la socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, 3) < 0) {
        perror("Erreur lors de l'écoute des connexions");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("En attente de connexions sur le port %d...\n", port);

    // Accepter une connexion entrante
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Recevoir les données
    *data = NULL;
    *size = 0;

    while (1) {
        ssize_t bytes_read = read(new_socket, buffer, buffer_size);
        if (bytes_read < 0) {
            perror("Erreur lors de la réception des données");
            free(*data);
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0) {
            // Fin de la transmission
            break;
        }

        // Allouer ou réallouer de la mémoire pour stocker les données
        void *temp = realloc(*data, total_read + bytes_read);
        if (!temp) {
            perror("Erreur d'allocation mémoire");
            free(*data);
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        *data = temp;

        // Copier les données reçues dans la mémoire allouée
        memcpy((char *)(*data) + total_read, buffer, bytes_read);
        total_read += bytes_read;
    }

    *size = total_read;

    printf("Données reçues : %zu octets\n", *size);

    // Fermer les sockets
    close(new_socket);
    close(server_fd);
}
