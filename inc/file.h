#ifndef FILE_H
#define FILE_H
#include <stdint.h>
#include <regex.h>
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
void processFile(pid_node_t *cur_pid, char *path_buff, char *cmd_buff, int fd_t, int type_filter, regex_t *i_re, regex_t *f_re);
void printInfo(char *command, int pid, char *user, const char *fd, const char *type, int nodes, char *name);
#endif
