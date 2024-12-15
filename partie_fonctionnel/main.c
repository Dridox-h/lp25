#include "backup_manager.h"
#include "file_handler.h"
#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    const char *source_dir = "test_source";  // Répertoire source à sauvegarder
    const char *backup_dir = "backups";      // Répertoire où les sauvegardes seront stockées
    const char *restore_dir = "restored";    // Répertoire où la sauvegarde sera restaurée

    // Créer le répertoire de sauvegarde si nécessaire
    if (mkdir(backup_dir, 0700) == -1 && errno != EEXIST) {
        perror("Erreur lors de la création du répertoire de sauvegarde");
        return 1;
    }

    // Créer le répertoire de restauration si nécessaire
    if (mkdir(restore_dir, 0700) == -1 && errno != EEXIST) {
        perror("Erreur lors de la création du répertoire de restauration");
        return 1;
    }

    // Créer une sauvegarde du répertoire source dans le répertoire de sauvegarde
    printf("Création de la sauvegarde...\n");
    create_backup(source_dir, backup_dir);

    // Liste les sauvegardes disponibles
    printf("\nListe des sauvegardes disponibles dans le répertoire '%s':\n", backup_dir);
    list_backups(backup_dir);

    // Supposons que nous avons un backup_id ici (pour cet exemple, nous utiliserons simplement un nom fixe)
    // Vous pouvez adapter cette logique pour trouver l'ID réel du backup créé.
    char backup_id[256];
    snprintf(backup_id, sizeof(backup_id), "%s/backup_2024-12-15_14-30-00", backup_dir);  // Exemple de chemin de sauvegarde

    // Restaurer la sauvegarde dans le répertoire de restauration
    printf("\nRestauration de la sauvegarde '%s'...\n", backup_id);
    restore_backup(backup_id, restore_dir);

    printf("Restauration terminée. Le fichier restauré devrait être dans le répertoire '%s'.\n", restore_dir);

    return 0;
}