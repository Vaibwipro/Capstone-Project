// fault_tolerance.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<fcntl.h>
#include "ipc_common.h" // custom header that defines MAX_CHILDREN , MSG_KEY , struct msgbuf

pid_t child_pids[MAX_CHILDREN];
char* task_names[MAX_CHILDREN]; // stores dynamically allocated copies of task names.
int num_children = 0; // tracks how many childs are currently running.

void cleanup()
{
printf("[Fault Tolerance] Cleaning up allocated memory...\n");
for(int i=0;i<num_children;++i)
{
if(task_names[i])
{
free(task_names[i]);
task_names[i] = NULL;
}
}
}
void produce_monitored_child(int index, const char* task) {
    pid_t pid = fork();
    if (pid == 0) {
        printf("[Child %d] Task: %s\n", getpid(), task);
        sleep(5);
        exit(0);
    } else if (pid > 0) {
        // Saves the child pid and duplicates the task string for later recovery.
        child_pids[index] = pid;
        task_names[index] = malloc(strlen(task) + 1);
        strcpy(task_names[index], task);
    }
}

void sigchld_handler(int sig)  // Registered as a handler for SIGCHLD signal , sent when a child terminates.
{
    int status;
    pid_t pid;
    // waits non-blocking for any child to exit loops through all exited children.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) 
    {
        // when a child terminates find its index and restart the same task using the saved task name.
        for (int i = 0; i < num_children; ++i) {
            if (child_pids[i] == pid) {
                printf("[FaultTolerance] Restarting task: %s\n", task_names[i]);
                produce_monitored_child(i, task_names[i]);
                break;
            }
        }
    }
}

int main() {
     // signal handler registered to catch child process terminations.
    signal(SIGCHLD, sigchld_handler);
    atexit(cleanup);  // Automatically call cleanup on exit.
    
    int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }
    
    struct msgbuf msg;
    printf("[Fault Tolerance] Monitoring Started...\n");

    while (1) {
    
        // if space is available , start the task and increment the counter.
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, IPC_NOWAIT) > 0) {
        
            msg.mtext[MAX_TEXT - 1] = '\0';
            if (num_children < MAX_CHILDREN) {
                produce_monitored_child(num_children, msg.mtext);
                num_children++;
            }
        }
        sleep(2);
    }
    return 0;
}

	


