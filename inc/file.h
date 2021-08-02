#ifndef FILE_H
#define FILE_H
#include <stdint.h>
#include "pids.h"
typedef struct file_info file_info_t;
struct file_info{
    char *command;
    int pid;
    char *user;
    char *fd;
    char *type;
    u_int64_t nodes;
    char *name;
    struct file_info *next;
};
int getFiles(pid_node_t *pids, char **args);
void printInfo(char *command, int pid, char *user, char *fd, char *type, int nodes, char *name);
u_int64_t getNodes(char *path);
#endif
