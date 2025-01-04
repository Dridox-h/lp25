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
    ssize_t sent = send(sock, data, size, 0);
    if (sent < 0) {
        perror("Erreur lors de l'envoi des données");
    } else {
        printf("Données envoyées : %zd octets\n", sent);
    }

    // Fermer la socket
    close(sock);
}

void receive_data(int port, void **data, size_t *size) {
    // Implémenter la logique de réception de données depuis un serveur distant
}
