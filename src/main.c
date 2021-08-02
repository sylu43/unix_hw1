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
    
    //get pids
	pid_node_t *pids = getPids();
    if(pids == NULL){
        printf("get pids err");
        return -1;
    }
    
    //get info from pids
    if(getFiles(pids, args) == -1){
        printf("err");
        return -1;
    }

	return 0;
}
