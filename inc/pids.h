#ifndef PIDS_H
#define PIDS_H
#include <sys/types.h>
#define PROC_DIR    "/proc/"
typedef struct pid_node pid_node_t;
typedef struct user_node user_node_t;
static user_node_t *users;
struct pid_node{
    int pid;
    char *username;
    struct pid_node *next;
};
struct user_node{
    char *name;
    int uid;
    struct user_node *next;
};
pid_node_t *getPids();
char *parseUid(char *line);
char *getUsername(int uid);
void passwd(); 
#endif
