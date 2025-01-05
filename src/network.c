#include "network.h"

void send_data(const char *server_address, int port, const void *data, size_t size) {
     int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur dans la création du socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convertir l'adresse IP
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Erreur de la conversion de l'adresse IP");
        close(sock);
        return;
    }

    // Établir la connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion");
        close(sock);
        return;
    }

    // Envoyer les données
    size_t bytes_sent = send(sock, data, size, 0);
    if (bytes_sent < 0) {
        perror("Erreur dans l'envoie des données");
    } else {
        printf("Envoyé %zd octets au serveur.\n", bytes_sent);
    }

    close(sock);
}

size_t receive_data(int port, size_t size) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Erreur dans la création de socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &(server_addr.sin_addr)) ;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    // Lier le socket à l'adresse et au port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur dans la liaison du socket à l'adresse et au port");
        close(server_sock);
        return -1;
    }

    // Écouter les connexions entrantes
    if (listen(server_sock, 1) < 0) {
        perror("Erreur lors de l'écoute du serveur");
        close(server_sock);
        return -1;
    }

    printf("En attente d'une connexion sur le port %d...\n", port);

    // Accepter une connexion
    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        close(server_sock);
        return -1;
    }

    // Recevoir des données
    char buffer[(int)size];
    size_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Erreur dans la réception des données");
        close(client_sock);
        close(server_sock);
        return -1;
    }

    // Si des données ont bien été récupérées, elles sont retournées pour être sauvegardées/écrites
    close(client_sock);
    close(server_sock);

    return bytes_received;
}
