#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

void create_backup(const char *source, const char *destination);
void restore_backup(const char *backup_id, const char *destination);
void list_backup(const char *directory);

#endif // BACKUP_MANAGER_H