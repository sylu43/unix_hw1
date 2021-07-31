#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include "pids.h"

int getPids(pid_node_t *pids){

	DIR *dp;
	FILE *fp;
    struct dirent *dir;
    char status_file[20], status[128];

    pid_node_t *cur_pid = pids;

	if((dp = opendir(PROC_DIR)) == NULL){
		return -1;
	}

    passwd();

    strcpy(status_file, PROC_DIR);
    //iterate files in /proc
	while((dir = readdir(dp)) != NULL){
		char *c = dir->d_name;
		while(*c != '\0'){
			if(!isdigit(*c)){
				break;
			}
			c++;
		}
		if(*c != '\0'){
			continue;
		}

        //if number
        cur_pid->name = strdup(dir->d_name);
       
        //check uid
        memset(status_file+6, 0, 20-6);
        strcpy(status_file+6, cur_pid->name);
        strcat(status_file, "/status");
        fp = fopen(status_file, "r");
        while(fgets(status, 128, fp) != NULL){
            if(!strncmp("Uid:", status, 4)){
                cur_pid->username = strdup(parseUid(status));
                break;
            }
        }
        fclose(fp);

        //make next node
        cur_pid->next = (pid_node_t*)malloc(sizeof(pid_node_t));
        cur_pid = cur_pid->next;
        cur_pid->name = NULL;
	}
    closedir(dp);
	return 0;
}

char *parseUid(char *line){
    char del[2] = "\t";
    char *uid = strtok(line, del);
    uid = strtok(NULL, del);
    return getUsername(atoi(uid));
}

char *getUsername(int uid){
    
    user_node_t *user = users;
    while(user->name != NULL){
        if(user->uid == uid){
            return user->name;
        }
        user = user->next;
    }
    return "errr";
}

void passwd(){
    
    const char *del = ":";
    FILE *fp = fopen("/etc/passwd", "r");
    char line[256];
    char *ptr;
    users = (user_node_t*)malloc(sizeof(user_node_t));
    user_node_t *user = users;
    while(fgets(line, 256, fp) != NULL){
        user->name = strdup(strtok(line, del));
        strtok(NULL, del);
        user->uid = atoi(strtok(NULL, del));

        user->next = (user_node_t*)malloc(sizeof(user_node_t));
        user = user->next;
        user->name = NULL;
    }
    return;
}
