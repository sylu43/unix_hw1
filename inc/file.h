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
    u_int64_t node;
    char *name;
    struct file_info *next;
};
int getFiles(pid_node_t *pids,file_info_t *files);
void fillInfo(file_info_t *info, const char *command, int pid, const char *user, const char *fd, const char *type, u_int64_t node, const char *name);
u_int64_t getNodes(char *path);
#endif
