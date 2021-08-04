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
    regex_t command_filter_re, file_filter_re, extract_inode_re;
    regex_t *file_filter;
    regmatch_t match[1];
    //-c
    if(args[0] != NULL){
        regcomp(&command_filter_re, args[0], REG_EXTENDED);
    }
    //-f
    if(args[2] != NULL){
        file_filter = &file_filter_re;
        regcomp(file_filter, args[2], REG_EXTENDED);
    }
    else{
        file_filter = NULL;
    }
    
    regcomp(&extract_inode_re, "[^[]:\[[0-9]\]", REG_EXTENDED);
    

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

    //printf("%-32s%8d\t%-24s\t%-8s\t%-8s\t%-8.0d\t%-s\n", command, pid, user, fd, type, nodes, name);
    printf("COMMAND\t\t\t\t\t\t\tPID\t\tUSEt\t\t\t\t\tFD\t\tTYPE\tNODE\t\tNAME\n");
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
            if(regexec(&command_filter_re, cmd_buff, 1, match, 0)){
                cur_pid = cur_pid->next;
                path_buff[6] = '\0';
                continue;
            }
        }

        //read cwd
        strcpy(path_buff + filename_offset, FD_STRING[cwd]);
        processFile(cur_pid, path_buff, cmd_buff, cwd, type_filter, NULL, file_filter);

        //read exe
        strcpy(path_buff + filename_offset, FD_STRING[exe]);
        processFile(cur_pid, path_buff, cmd_buff, exe, type_filter, NULL, file_filter);
        
        //read root
        strcpy(path_buff + filename_offset, FD_STRING[root]);
        processFile(cur_pid, path_buff, cmd_buff, root, type_filter, NULL, file_filter);
        
        //read mem
        strcpy(path_buff + filename_offset, "maps");
        fp = fopen(path_buff, "r");
        if(fp != NULL){
            while(fgets(read_buff, READ_BUFF_SIZE, fp) != NULL){
                if(strchr(read_buff, '/') != NULL){
                    strcpy(read_buff, strchr(read_buff, '/'));
                    read_buff[strlen(read_buff) - 1] = '\0';
                    processFile(cur_pid, read_buff, cmd_buff, strstr(read_buff, "(deleted)") ? del : mem, type_filter, NULL, file_filter);
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
                stat(path_buff, &st);
                readlink(path_buff, read_buff, READ_BUFF_SIZE);
                if((st.st_mode & S_IRUSR) && (st.st_mode & S_IWUSR)){
                    strcat(fd_buff, "u");
                }
                else if(st.st_mode & S_IRUSR){
                    strcat(fd_buff, "r");
                }
                else if(st.st_mode & S_IWUSR){
                    strcat(fd_buff, "w");
                }
                processFile(cur_pid, path_buff, cmd_buff, FD, type_filter, &extract_inode_re, file_filter);
            }
        }
        else{
            strcat(path_buff, fd);
            processFile(cur_pid, path_buff, cmd_buff, nofd, type_filter, NULL,file_filter);
        }

        cur_pid = cur_pid->next;
        path_buff[6] = '\0';
    }
    
    return 0;
}

void processFile(pid_node_t *cur_pid, char *path_buff, char *cmd_buff, int fd_t, int type_filter, regex_t *i_re, regex_t *f_re){

    static u_int64_t nodes;
    static char read_buff[READ_BUFF_SIZE], name_buff[READ_BUFF_SIZE];
    static char const *type_p;
    static struct stat st;
    static const char *readlink_err = " (readlink: Permission denied)";
    static const char *opendir_err = " (opendir: Permission denied)";
    static const char *pipe = "pipe";
    static const char *socket = "socket";
    static const char *anon_inode = "anon_inode";

    //clear buff
    memset(read_buff, '\0', READ_BUFF_SIZE);
    name_buff[0] = '\0';
    
    //file filter
    static regmatch_t match[1];

    //only get real name when read mem
    if(fd_t ==  mem || fd_t == del || fd_t == nofd){
        strcpy(read_buff, path_buff);
    }
    else{
        readlink(path_buff, read_buff, READ_BUFF_SIZE);
    }

    //nofd
    if(fd_t == nofd){
        if(type_filter != 1){
            return;
        }
        type_p = TYPE_STRING[empty];
        nodes = 0;
        strcpy(name_buff, path_buff);
        strcat(name_buff, opendir_err);
    }
    //can't access link for exe, cwd, root
    else if((fd_t != mem || fd_t != del) && errno == EACCES){
        if(!(type_filter & unknown)){
            return;
        }
        type_p = TYPE_STRING[unknown];
        nodes = 0;
        strcpy(name_buff, path_buff);
        strcat(name_buff, readlink_err);
        nodes = 0;
    }
    //exe no such file
    else if((fd_t == exe) && errno == ENOENT){
        if(!(type_filter & ((1 << reg) | 1 ))){
            return;
        }
        type_p = TYPE_STRING[reg];
        strcpy(name_buff, path_buff);
        nodes = 0;
    }
    //get access
    else{
        //get type from name
        stat(read_buff, &st);
        if(!strncmp(read_buff, pipe, 4)){
            if(!(type_filter & ((1 << fifo) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[fifo];
            if(regexec(i_re, read_buff, 1, match, 0)){
                sscanf(read_buff, "%*[^[][%ld]", &nodes);
            }
        }
        else if(!strncmp(read_buff, socket, 6)){
            if(!(type_filter & ((1 << sock) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[sock];
            if(regexec(i_re, read_buff, 1, match, 0)){
                sscanf(read_buff, "%*[^[][%ld]", &nodes);
            }
        }
        else if(read_buff[0] != '/'){
            if(!(type_filter & 1)){
                return;
            }
            type_p = TYPE_STRING[unknown];
            stat(path_buff, &st);
            nodes = st.st_ino;
        }
        //get type from st_mode
        else if(S_ISSOCK(st.st_mode)){
            if(!(type_filter & ((1 << sock) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[sock];
            nodes = st.st_ino;
        }
        else if(S_ISFIFO(st.st_mode)){
            if(!(type_filter & ((1 << fifo) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[fifo];
            nodes = st.st_ino;
        }
        else if(S_ISCHR(st.st_mode)){
            if(!(type_filter & ((1 << chr) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[chr];
            nodes = st.st_ino;
        }
        else if(S_ISDIR(st.st_mode)){
            if(!(type_filter & ((1 << dir) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[dir];
            nodes = st.st_ino;
        }
        else if(S_ISREG(st.st_mode)){
            if(!(type_filter & ((1 << reg) | 1 ))){
                return;
            }
            type_p = TYPE_STRING[reg];
            nodes = st.st_ino;
        }
        strcpy(name_buff, read_buff);
    }

    //filter filename
    if(f_re != NULL){
        if(regexec(f_re, name_buff, 1, match, 0)){
            return;
        }
    }

    printInfo(cmd_buff, cur_pid->pid, cur_pid->username, FD_STRING[fd_t], type_p, nodes, name_buff);
    return;
}

void printInfo(char *command, int pid, char *user, const char *fd, const char *type, int nodes, char *name){
    printf("%-32s%-8d%-24s%-8s%-8s%-12.0u%-8s\n", command, pid, user, fd, type, nodes, name);
}
