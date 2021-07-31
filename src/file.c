#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "file.h"

enum FD_ENUM {
    cwd, root, exe, mem, del, fd
};

static const char *FD_STRING[] = {
    "cwd", "root", "exe", "maps", "del", "fd"
};

enum TYPE_ENUM {
    DIR, REG, CHR, FIFO, SOCK, unkown
};

static const char *TYPE_STRING[] = {
    "DIR", "REG", "CHR", "FIFO", "SOCK", "unkown"
};

int getFiles(pid_node_t *pids, file_info_t *infos){
    
    u_int64_t nodes;
    char path_buf[64], read_buf[128];

    pid_node_t *cur_pid = pids;
    file_info_t *cur_info = infos;
    strcpy(path_buf, PROC_DIR);

    while(cur_pid->name != NULL){
        
        strcat(path_buf, cur_pid->name);

        //read command and exe
        path_buf[6 + strlen(cur_pid->name)] = '/';
        strcpy(path_buf + 6 + strlen(cur_pid->name) + 1, FD_STRING[exe]);
        readlink(path_buf, read_buf, 128);
        nodes = getNodes(read_buf);
        fillInfo(cur_info, strrchr(read_buf, '/') + 1, atoi(cur_pid->name), cur_pid->username, FD_STRING[exe], TYPE_STRING[REG], nodes , read_buf);
        
        //read cwd
        //strcat(path_buf + 6 + strlen(cur_pid->name), cwd);
        //printf("%s",path_buf);
        //readlink(path_buf, read_buf, 128);
        //cur_info->
        //read root
        //read exe
        //read mem
        //read del
        //read fd
    

        cur_pid->next = (pid_node_t*)malloc(sizeof(pid_node_t));
        cur_pid = cur_pid->next;
        cur_pid->name = NULL;
        memset(path_buf, '\0', 64);
    }
    
    /*
    printf("cmd:    %s\n \
            pid:    %d\n \
            user:   %s\n \
            fd:     %s\n \
            type:   %s\n \
            node:   %ld\n\
            name:   %s\n ",infos->command,infos->pid,infos->user,infos->fd,infos->type,infos->node,infos->name);
    */
    return 0;
}

void fillInfo(file_info_t *info, const char* command, int pid, const char *user, const char *fd, const char *type, u_int64_t node, const char *name){
    info->command = strdup(command);
    info->pid = pid;
    info->user = strdup(user);
    info->fd = strdup(fd);
    info->type = strdup(type);
    info->node = node;
    info->name = strdup(name);
    return;
}

u_int64_t getNodes(char *path){
    static struct stat st;
    stat(path, &st);
    return st.st_ino;
}
