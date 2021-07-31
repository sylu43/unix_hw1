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
    
    int filename_offset;
    u_int64_t nodes;
    char path_buf[64], read_buf[128], *exe_name;

    pid_node_t *cur_pid = pids;
    file_info_t *cur_info = infos;
    strcpy(path_buf, PROC_DIR);

    while(cur_pid->name != NULL){
        
        strcat(path_buf, cur_pid->name);
        path_buf[6 + strlen(cur_pid->name)] = '/';
        filename_offset = 6 + strlen(cur_pid->name) + 1;

        //read command and exe
        strcpy(path_buf + filename_offset, FD_STRING[exe]);
        memset(read_buf, '\0', 128);
        readlink(path_buf, read_buf, 128);
        nodes = getNodes(read_buf);
        exe_name = strdup(strrchr(read_buf, '/') + 1);
        fillInfo(cur_info, exe_name ,atoi(cur_pid->name), cur_pid->username, FD_STRING[exe], TYPE_STRING[REG], nodes , read_buf);
        cur_info->next = (file_info_t*)malloc(sizeof(file_info_t));
        cur_info = cur_info->next;

        //read cwd
        strcpy(path_buf + filename_offset, FD_STRING[cwd]);
        memset(read_buf, '\0', 128);
        readlink(path_buf, read_buf, 128);
        nodes = getNodes(read_buf);
        fillInfo(cur_info, exe_name, atoi(cur_pid->name), cur_pid->username, FD_STRING[cwd], TYPE_STRING[DIR], nodes , read_buf);
        cur_info->next = (file_info_t*)malloc(sizeof(file_info_t));
        cur_info = cur_info->next;

        //read root
        strcpy(path_buf + filename_offset, FD_STRING[root]);
        memset(read_buf, '\0', 128);
        readlink(path_buf, read_buf, 128);
        nodes = getNodes(read_buf);
        fillInfo(cur_info, exe_name, atoi(cur_pid->name), cur_pid->username, FD_STRING[root], TYPE_STRING[DIR], nodes , read_buf);
        cur_info->next = (file_info_t*)malloc(sizeof(file_info_t));
        cur_info = cur_info->next;

        //read mem

        //read del
        //read fd
    

        cur_pid = cur_pid->next;
        memset(path_buf + 6, '\0', 64 - 6);
    }
    
    printf("cmd\tpid\tuser\tfd\ttype\tnode\tname\n");
    
    cur_info = infos;
    while(cur_info -> name != NULL){
        printf("%s\t%d\t%s\t%s\t%s\t%ld\t%s\n",cur_info->command,cur_info->pid,cur_info->user,cur_info->fd,cur_info->type,cur_info->nodes,cur_info->name);
        cur_info = cur_info->next;
    }

    return 0;
}

void fillInfo(file_info_t *info, const char* command, int pid, const char *user, const char *fd, const char *type, u_int64_t nodes, const char *name){
    info->command = command;
    info->pid = pid;
    info->user = user;
    info->fd = fd;
    info->type = type;
    info->nodes = nodes;
    info->name = strdup(name);
    return;
}

u_int64_t getNodes(char *path){
    static struct stat st;
    stat(path, &st);
    return st.st_ino;
}
