#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"

int main(int argc, char *argv[]) {
    // Déclaration des variables pour les options
    int backup_flag = 0;
    int restore_flag = 0;
    int list_flag = 0;
    int dry_run = 0;
    int verbose = 0;

    const char *source = NULL;
    const char *destination = NULL;
    const char *d_server = NULL;
    int d_port = 0;
    const char *s_server = NULL;
    int s_port = 0;

    // Analyse des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--backup") == 0) {
            backup_flag = 1;
        } else if (strcmp(argv[i], "--restore") == 0) {
            restore_flag = 1;
        } else if (strcmp(argv[i], "--list-backups") == 0) {
            list_flag = 1;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--source") == 0) {
            if (i + 1 < argc) {
                source = argv[++i];
            } else {
                fprintf(stderr, "Erreur : l'option --source nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--dest") == 0) {
            if (i + 1 < argc) {
                destination = argv[++i];
            } else {
                fprintf(stderr, "Erreur : l'option --dest nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--d-server") == 0) {
            if (i + 1 < argc) {
                d_server = argv[++i];
            } else {
                fprintf(stderr, "Erreur : l'option --d-server nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--d-port") == 0) {
            if (i + 1 < argc) {
                d_port = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Erreur : l'option --d-port nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--s-server") == 0) {
            if (i + 1 < argc) {
                s_server = argv[++i];
            } else {
                fprintf(stderr, "Erreur : l'option --s-server nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--s-port") == 0) {
            if (i + 1 < argc) {
                s_port = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Erreur : l'option --s-port nécessite un argument.\n");
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Erreur : option inconnue %s.\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    // Vérification des options exclusives
    if ((backup_flag + restore_flag + list_flag) > 1) {
        fprintf(stderr, "Erreur : les options --backup, --restore et --list-backups sont mutuellement exclusives.\n");
        return EXIT_FAILURE;
    }

    // Résumé des options analysées (si nécessaire pour débogage ou confirmation)
    if (verbose) {
        if (backup_flag) {
            printf("Option sélectionnée : --backup\n");
        } else if (restore_flag) {
            printf("Option sélectionnée : --restore\n");
        } else if (list_flag) {
            printf("Option sélectionnée : --list-backups\n");
        } else {
            fprintf(stderr, "Erreur : aucune option principale (--backup, --restore, --list-backups) n'a été spécifiée.\n");
            return EXIT_FAILURE;
        }

        if (source) printf("Chemin source : %s\n", source);
        if (destination) printf("Chemin destination : %s\n", destination);
        if (d_server) printf("Serveur de destination : %s\n", d_server);
        if (d_port) printf("Port de destination : %d\n", d_port);
        if (s_server) printf("Serveur source : %s\n", s_server);
        if (s_port) printf("Port source : %d\n", s_port);
        if (dry_run) printf("Mode simulation activé (--dry-run).\n");
    }

    // Exécution en fonction de l'option choisie
    if (backup_flag) {
        // Créer un backup local ou distant
        if (d_server) {
            // Sauvegarde distante via réseau
            if (dry_run) {
                printf("Mode simulation activé, pas de transfert.\n");
            } else {
                // Sauvegarde distante
                printf("Sauvegarde vers le serveur distant %s:%d\n", d_server, d_port);
                send_data(d_server, d_port, source, strlen(source)); // Sauvegarde distante
            }
        } else {
            // Sauvegarde locale
            if (dry_run) {
                printf("Mode simulation activé, pas de sauvegarde réelle.\n");
            } else {
                create_backup(source, destination); // Sauvegarde locale
                printf("Sauvegarde locale effectuée.\n");
            }
        }
    }

    if (restore_flag) {
        // Restaurer un backup local ou distant
        if (s_server) {
            // Restauration distante via réseau
            if (dry_run) {
                printf("Mode simulation activé, pas de récupération.\n");
            } else {
                // Restaurer depuis le serveur distant
                printf("Restauration depuis le serveur distant %s:%d\n", s_server, s_port);
                receive_data(s_port); // Restauration distante
            }
        } else {
            // Restauration locale
            if (dry_run) {
                printf("Mode simulation activé, pas de restauration réelle.\n");
            } else {
                restore_backup(source, destination); // Restauration locale
                printf("Restauration locale effectuée.\n");
            }
        }
    }

    if (list_flag) {
        // Lister les sauvegardes disponibles
        if (dry_run) {
            printf("Mode simulation activé, pas de listing réel.\n");
        } else {
            list_backups(destination); // Lister les sauvegardes
        }
    }

    return EXIT_SUCCESS;
}
