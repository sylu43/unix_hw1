#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "args.h"

int procArgs(int argc, char *argv[], char *args[]){

	int cmd_opt = 0;

	while(1){
		cmd_opt = getopt(argc, argv, "c:t:f:");
		if(cmd_opt == -1){
			break;
		}
		if(cmd_opt == '?'){
			printf("wrong arguement!\n");
			return -1;
		}
		switch(cmd_opt){
			case 'c':
				args[0] = strdup(optarg);
				break;
			case 't':
				args[1] = strdup(optarg);
				break;
			case 'f':
				args[2] = strdup(optarg);
				break;
			default:
				printf("no arg\n", optarg);
				break;
		}
	}

	if(argc > optind){
		printf("too many args\n");
		return -1;
	}

	return 0;
}
