#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include "file.h"

#define READ_BUF_SIZE 256
#define PATH_BUF_SIZE 64

enum FD_ENUM {
    cwd, root, exe, mem, del, FD
};

static const char *FD_STRING[] = {
    "cwd", "root", "exe", "maps", "del", "fd"
};

enum TYPE_ENUM {
    dir, reg, chr, fifo, sock, unknown
};

static const char *TYPE_STRING[] = {
    "DIR", "REG", "CHR", "FIFO", "SOCK", "unkown"
};

int getFiles(pid_node_t *pids, char **args){
    
    int filename_offset, pid_len;
    char path_buf[PATH_BUF_SIZE], read_buf[READ_BUF_SIZE];
    char cmd_buff[READ_BUF_SIZE], user_buff[32], fd_buff[10];
    const char *status = "status";
    FILE *fp;

    regex_t re;
    regmatch_t match[1];
    pid_node_t *cur_pid = pids;
    strcpy(path_buf, PROC_DIR);
    if(args[0] != NULL){
        regcomp(&re, args[0], REG_EXTENDED);
    }

    printf("COMMAND\t\t\t\tPID\tUSER\t\t\t\tFD\tTYPE\tNODE\tNAME\n");
    while(cur_pid != NULL){
        sprintf(path_buf,"%s%d/", path_buf, cur_pid->pid);
        filename_offset = strlen(path_buf);

        //filter command
        strcat(path_buf, status);
        read_buf[0] = '\0';
        fp = fopen(path_buf, "r");
        fgets(read_buf, READ_BUF_SIZE, fp);
        fclose(fp);
        strtok(read_buf, "\t");
        strcpy(cmd_buff, strtok(NULL, "\n"));
        fflush(stdout);
        if(args[0] != NULL){
            if(regexec(&re, cmd_buff, 1, match, 0)){
                cur_pid = cur_pid->next;
                path_buf[6] = '\0';
                continue;
            }
        }

        //read command and exe
        strcpy(path_buf + filename_offset, FD_STRING[exe]);
        processFile(cur_pid, path_buf, cmd_buff, exe);
        /*
        fd_p = FD_STRING[exe];
        read_buf[0] = '\0';
        readlink(path_buf, read_buf, READ_BUF_SIZE);
        if(errno == EACCES){
            type_p = TYPE_STRING[unknown];
            nodes = 0;
            strcpy(name_buff, path_buf);
            strcat(name_buff, readlink_err);
        }
        else{
            type_p = TYPE_STRING[reg];
            nodes = getNodes(read_buf);
            strcpy(name_buff, read_buf);
        }
        printInfo(cmd_buff, pid, user_buff, fd_p, type_p, nodes, name_buff);
        */
        //nodes = getNodes(read_buf);
        //exe_name = strdup(strrchr(read_buf, '/') + 1);
        
        /* 
        //read cwd
        strcpy(path_buf + filename_offset, FD_STRING[cwd]);
        memset(read_buf, '\0', READ_BUF_SIZE);
        readlink(path_buf, read_buf, READ_BUF_SIZE);
        nodes = getNodes(read_buf);
        fillInfo(cur_info, exe_name, atoi(cur_pid->name), cur_pid->username, FD_STRING[cwd], TYPE_STRING[dir], nodes , read_buf);
        cur_info->next = (file_info_t*)malloc(sizeof(file_info_t));
        cur_info = cur_info->next;

        //read root
        strcpy(path_buf + filename_offset, FD_STRING[root]);
        memset(read_buf, '\0', READ_BUF_SIZE);
        readlink(path_buf, read_buf, READ_BUF_SIZE);
        nodes = getNodes(read_buf);
        fillInfo(cur_info, exe_name, atoi(cur_pid->name), cur_pid->username, FD_STRING[root], TYPE_STRING[dir], nodes , read_buf);
        cur_info->next = (file_info_t*)malloc(sizeof(file_info_t));
        cur_info = cur_info->next;

        //read mem

        //read del
        //read fd
    
        */
        cur_pid = cur_pid->next;
        //memset(path_buf + 6, '\0', PATH_BUF_SIZE - 6);
        path_buf[6] = '\0';
    }
    /* 
    printf("cmd\tpid\tuser\tfd\ttype\tnode\tname\n");
    
    cur_info = infos;
    while(cur_info -> name != NULL){
        printf("%s\t%d\t%s\t%s\t%s\t%ld\t%s\n",cur_info->command,cur_info->pid,cur_info->user,cur_info->fd,cur_info->type,cur_info->nodes,cur_info->name);
        cur_info = cur_info->next;
    }
    */

    return 0;
}

void processFile(pid_node_t *cur_pid, char *path_buf, char *cmd_buff, int fd_t){

    static u_int64_t nodes;
    static char read_buf[READ_BUF_SIZE], name_buff[READ_BUF_SIZE];
    static char *type_p;
    const char *readlink_err = " (readlink: Permission denied)";
    const char *opendir_err = " (opendir: Permission denied)";
    read_buf[0] = '\0';
    name_buff[0] = '\0';
    readlink(path_buf, read_buf, READ_BUF_SIZE);
    if(errno == EACCES){

        type_p = TYPE_STRING[unknown];
        nodes = 0;
        strcpy(name_buff, path_buf);
        strcat(name_buff, readlink_err);
    }
    else{
        type_p = TYPE_STRING[reg];
        nodes = getNodes(read_buf);
        strcpy(name_buff, read_buf);
    }
    printInfo(cmd_buff, cur_pid->pid, cur_pid->username, FD_STRING[fd_t], type_p, nodes, name_buff);

}

void printInfo(char *command, int pid, char *user, const char *fd, const char *type, int nodes, char *name){
    printf("%-32s%d\t%-24s\t%s\t%s\t%d\t%s\n", command, pid, user, fd, type, nodes, name);
}

u_int64_t getNodes(char *path){
    static struct stat st;
    stat(path, &st);
    return st.st_ino;
}
