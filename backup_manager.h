#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "deduplication.h"

// Déclarations des fonctions
void create_backup(const char *source_dir, const char *backup_dir);
void backup_file(const char *filename, const char *backup_dir);
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count);
void restore_backup(const char *backup_id, const char *restore_dir);
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count);
void list_backups(const char *backup_dir);

#endif // BACKUP_MANAGER_H