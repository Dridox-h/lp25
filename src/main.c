#include "backup_manager.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *command;
    const char *source_dir;
    const char *backup_dir;
    const char *backup_file;
    const char *restore_dir;
    const char *server_address;
    int port;
    FILE *file;
    size_t size;
    void *data;
    const char *output_file = argv[3];

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command> [options]\n", argv[0]);
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "  backup <source_dir> <backup_dir>\n");
        fprintf(stderr, "  restore <backup_dir> <restore_dir>\n");
        fprintf(stderr, "  send <backup_file> <server_address> <port>\n");
        fprintf(stderr, "  receive <port> <output_file>\n");
        return 1;
    }
    command = argv[1];
    if (strcmp(command, "backup") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s backup <source_dir> <backup_dir>\n", argv[0]);
            return 1;
        }
        source_dir = argv[2];
        backup_dir = argv[3];
        create_backup(source_dir, backup_dir);
        printf("Backup completed: %s -> %s\n", source_dir, backup_dir);

    }
    else if (strcmp(command, "restore") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s restore <backup_dir> <restore_dir>\n", argv[0]);
            return 1;
        }
        backup_dir = argv[2];
        restore_dir = argv[3];
        restore_backup(backup_dir, restore_dir);
        printf("Restore completed: %s -> %s\n", backup_dir, restore_dir);
    }
    else if (strcmp(command, "send") == 0)
    {
        if (argc != 5)
        {
            fprintf(stderr, "Usage: %s send <backup_file> <server_address> <port>\n", argv[0]);
            return 1;
        }
        backup_file = argv[2];
        server_address = argv[3];
        port = atoi(argv[4]);
        file = fopen(backup_file, "rb");
        if (!file)
        {
            perror("Error opening backup file");
            return 1;
        }
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        rewind(file);
        data = malloc(size);
        if (!data)
        {
            perror("Memory allocation failed");
            fclose(file);
            return 1;
        }
        fread(data, 1, size, file);
        fclose(file);
        send_data(server_address, port, data, size);
        printf("Backup file sent to %s:%d\n", server_address, port);
        free(data);
    }
    else if (strcmp(command, "receive") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: %s receive <port> <output_file>\n", argv[0]);
            return 1;
        }
        port = atoi(argv[2]);
        output_file = argv[3];
        data = NULL;
        size = 0;
        receive_data(port);
        file = fopen(output_file, "wb");
        if (!file)
        {
            perror("Error creating output file");
            free(data);
            return 1;
        }
        fwrite(data, 1, size, file);
        fclose(file);
        printf("Backup file received and saved to %s\n", output_file);
        free(data);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        fprintf(stderr, "Available commands: backup, restore, send, receive\n");
        return 1;
    }
    return 0;
}
