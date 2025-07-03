#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h> // message queue ipc
#include <sys/msg.h>
#include <sys/wait.h> // message queue ipc
#include <signal.h>
#include <time.h>
#include<fcntl.h>
#include "ipc_common.h"  // user defined header that defines shared constants and structures.

#define LOAD_THRESHOLD 50

pid_t children[MAX_CHILDREN];  // children array to store child PIDs
int child_count = 0; // count of currently active child

// Simulated load generator
int simulate_load() {
    return rand() % 100; // Load from 0 to 99%
}

// Spawn child to handle task
void produce_task_handler(const char *task) {
    if (child_count >= MAX_CHILDREN) {
        printf("[Allocator] Max child limit reached. Cannot produce more.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process: Simulate processing task
        printf("[Child %d] Started task: %s\n", getpid(), task);
        sleep(3); // Simulate task duration
        printf("[Child %d] Completed task: %s\n", getpid(), task);
        exit(0);
    } else if (pid > 0) {
        children[child_count++] = pid; // adds the new child PIDs to the array.
        printf("[Allocator] Spawned child %d for task: %s\n", pid, task);
    } else {
        perror("fork");
    }
}
int main() {
    srand(time(NULL)); // Seed for load simulation

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }

    struct msgbuf message;  // Declares a message buffer for incoming messages.
    printf("[Allocator] Ready to receive tasks...\n");

    while (1) {
    
        
        ssize_t bytes = msgrcv(msgid, &message, sizeof(message.mtext), 1, IPC_NOWAIT);
        
        
        if (bytes > 0) {
            message.mtext[bytes] = '\0'; // if message recieved null terminate the text string.

            int current_load = simulate_load();
            printf("[Allocator] Current Load: %d%%\n", current_load);
            
            // only accept task if load is greater than load_threshold
            if (current_load > LOAD_THRESHOLD) {
                printf("[Allocator] Load high, accepting task: %s\n", message.mtext);
                 produce_task_handler(message.mtext);
            } else {
                printf("[Allocator] Load too low (%d%%). Skipping task: %s\n", current_load, message.mtext);
            }
        }

        // Reap completed children
        for (int i = 0; i < child_count; ++i)  // Loop through active children 
        {
            int status;
            pid_t result = waitpid(children[i], &status, WNOHANG); // checks if the child has terminated without blocking.
            
            // if child has exited remove it from children array[] and decrement count.
            if (result > 0) {
                // Shift children left
                for (int j = i; j < child_count - 1; ++j) {
                    children[j] = children[j + 1];
                }
                child_count--;
                i--;
            }
        }

          sleep(4); 
          }
          
          return 0;
}


