#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "args.h"
#include "pids.h"
#include "file.h"

int main(int argc, char *argv[]){

    int ret;

    //get args
	//0 for command
	//1 for type
	//2 for file
	char *args[3] = {NULL};
	if(procArgs(argc, argv, args) == -1){
		return -1;
	}
    //printf("%s %s %s",args[0],args[1],args[2]);
    
    //get pids
	pid_node_t *const pids = (pid_node_t*)malloc(sizeof(pid_node_t));
    if(getPids(pids) == -1){
        printf("open /proc error!!!\n");
        return -1;
    }
    
    //get info from pids
    pid_node_t *pid = pids;
    file_info_t *infos = (file_info_t*)malloc(sizeof(file_info_t));
    if(getFiles(pids, infos) == -1){
        printf("err");
        return -1;
    }
    

	return 0;
}
