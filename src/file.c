#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "file.h"

#define READ_BUFF_SIZE 256
#define PATH_BUFF_SIZE 64

enum FD_ENUM {
    cwd, root, exe, mem, del, FD, nofd
};

static char *FD_STRING[] = {
    "cwd", "root", "exe", "mem", "del", "fd" ,"NOFD"
};

enum TYPE_ENUM {
    dir, reg, chr, fifo, sock, unknown, empty
};

const char *TYPE_STRING[] = {
    "DIR", "REG", "CHR", "FIFO", "SOCK", "unknown", ""
};

int getFiles(pid_node_t *pids, char **args){
    
    int filename_offset, pid_len, type_filter = 1;
    char path_buff[PATH_BUFF_SIZE], read_buff[READ_BUFF_SIZE];
    char cmd_buff[READ_BUFF_SIZE], user_buff[32], fd_buff[10];
    const char *status = "status";
    const char *maps = "maps";
    const char *fd = "fd";
    FILE *fp;
    DIR *dp;
    struct dirent *dir;
    struct stat st;
    pid_node_t *cur_pid = pids;

    //regex setup
    regex_t re, re2;
    regex_t *file_filter;
    regmatch_t match[1];
    if(args[0] != NULL){
        regcomp(&re, args[0], REG_EXTENDED);
    }
    if(args[2] != NULL){
        file_filter = &re2;
        regcomp(file_filter, args[2], REG_EXTENDED);
    }
    else{
        file_filter = NULL;
    }

    //set path_buff to /proc/
    strcpy(path_buff, PROC_DIR);
    FD_STRING[FD] = fd_buff;

    //type filter
    if(args[1] != NULL){
        for(int i = 0;i < 6;i++){
            if(!strcmp(TYPE_STRING[i], args[1])){
                type_filter = 1 << (i + 1);
            }
        }
    }

    printf("COMMAND\t\t\t\tPID\tUSER\t\t\t\tFD\tTYPE\tNODE\tNAME\n");
    while(cur_pid != NULL){
        sprintf(path_buff, "%s%d/", path_buff, cur_pid->pid);
        filename_offset = strlen(path_buff);

        //command filter 
        strcat(path_buff, status);
        read_buff[0] = '\0';
        fp = fopen(path_buff, "r");
        fgets(read_buff, READ_BUFF_SIZE, fp);
        fclose(fp);
        strtok(read_buff, "\t");
        strcpy(cmd_buff, strtok(NULL, "\n"));
        if(args[0] != NULL){
            if(regexec(&re, cmd_buff, 1, match, 0)){
                cur_pid = cur_pid->next;
                path_buff[6] = '\0';
                continue;
            }
        }

        //read cwd
        strcpy(path_buff + filename_offset, FD_STRING[cwd]);
        processFile(cur_pid, path_buff, cmd_buff, cwd, type_filter, file_filter);

        //read exe
        strcpy(path_buff + filename_offset, FD_STRING[exe]);
        processFile(cur_pid, path_buff, cmd_buff, exe, type_filter, file_filter);
        
        //read root
        strcpy(path_buff + filename_offset, FD_STRING[root]);
        processFile(cur_pid, path_buff, cmd_buff, root, type_filter, file_filter);
        
        //read mem
        strcpy(path_buff + filename_offset, "maps");
        fp = fopen(path_buff, "r");
        if(fp != NULL){
            while(fgets(read_buff, READ_BUFF_SIZE, fp) != NULL){
                if(strchr(read_buff, '/') != NULL){
                    strcpy(read_buff, strchr(read_buff, '/'));
                    read_buff[strlen(read_buff) - 1] = '\0';
                    processFile(cur_pid, read_buff, cmd_buff, strstr(read_buff, "(deleted)") ? del : mem, type_filter, file_filter);
                }
            }
        }

        //read fd
        strcpy(path_buff + filename_offset, fd);
        dp = opendir(path_buff);
        if(errno != EACCES){
            while((dir = readdir(dp)) != NULL){
                if(dir->d_name[0] == '.'){
                    continue;
                }
                strcpy(fd_buff, dir->d_name);
                strcat(path_buff, "/");
                strcpy(path_buff + filename_offset + 3, fd_buff);
                memset(read_buff, '\0', READ_BUFF_SIZE);
                readlink(path_buff, read_buff, READ_BUFF_SIZE);
                stat(read_buff, &st);
                if((st.st_mode & S_IRUSR) && (st.st_mode & S_IWUSR)){
                    strcat(fd_buff, "u");
                }
                else if(st.st_mode & S_IRUSR){
                    strcat(fd_buff, "r");
                }
                else if(st.st_mode & S_IWUSR){
                    strcat(fd_buff, "w");
                }
                processFile(cur_pid, read_buff, cmd_buff, FD, type_filter, file_filter);
            }
        }
        else{
            strcat(path_buff, fd);
            processFile(cur_pid, path_buff, cmd_buff, nofd, type_filter, file_filter);
        }

        cur_pid = cur_pid->next;
        path_buff[6] = '\0';
    }
    
    return 0;
}

void processFile(pid_node_t *cur_pid, char *path_buff, char *cmd_buff, int fd_t, int type_filter, regex_t *re){

    static u_int64_t nodes;
    static char read_buff[READ_BUFF_SIZE], name_buff[READ_BUFF_SIZE];
    static char const *type_p;
    static struct stat st;
    static const char *readlink_err = " (readlink: Permission denied)";
    static const char *opendir_err = " (opendir: Permission denied)";
    static const char *pipe = "pipe";

    //clear buff
    memset(read_buff, '\0', READ_BUFF_SIZE);
    name_buff[0] = '\0';
    
    //file filter
    static regmatch_t match[1];

    //only get real name when read mem
    if(fd_t ==  mem || fd_t == del || fd_t == FD || fd_t == nofd){
        strcpy(read_buff, path_buff);
    }
    else{
        readlink(path_buff, read_buff, READ_BUFF_SIZE);
    }

    //uknown if can't accest link
    if(fd_t == nofd){
        if(type_filter != 1){
            return;
        }
        type_p = TYPE_STRING[empty];
        nodes = 0;
        strcpy(name_buff, path_buff);
        strcat(name_buff, opendir_err);
    }
    else if((fd_t != mem || fd_t != del) && errno == EACCES){
        if(!(type_filter & unknown)){
            return;
        }
        type_p = TYPE_STRING[unknown];
        nodes = 0;
        strcpy(name_buff, path_buff);
        strcat(name_buff, readlink_err);
    }
    else{
        stat(read_buff, &st);
        if(!strncmp(read_buff, pipe, 4)){
            if(!(type_filter & ((1 << fifo) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[fifo];
        }
        else{
            switch(st.st_mode & S_IFMT){
                case S_IFSOCK:
                    if(!(type_filter & ((1 << sock) | 1 ))){
                        return;
                    }
                    type_p = TYPE_STRING[sock];
                    break;
                case S_IFDIR:
                    if(!(type_filter & ((1 << dir) | 1 ))){
                        return;
                    }
                    type_p = TYPE_STRING[dir];
                    break;
                case S_IFREG:
                    if(!(type_filter & ((1 << reg) | 1 ))){
                        return;
                    }
                    type_p = TYPE_STRING[reg];
                    break;
                case S_IFCHR:
                    if(!(type_filter & ((1 << chr) | 1 ))){
                        return;
                    }
                    type_p = TYPE_STRING[chr];
                    break;
            }
        }
        nodes = st.st_ino;
        strcpy(name_buff, read_buff);
    }
    if(re != NULL){
        if(regexec(re, name_buff, 1, match, 0)){
            //printf("here\n");
            return;
        }
    }

    printInfo(cmd_buff, cur_pid->pid, cur_pid->username, FD_STRING[fd_t], type_p, nodes, name_buff);
    return;
}

void printInfo(char *command, int pid, char *user, const char *fd, const char *type, int nodes, char *name){
    printf("%-32s%d\t%-24s\t%s\t%s\t%.0d\t%s\n", command, pid, user, fd, type, nodes, name);
}
