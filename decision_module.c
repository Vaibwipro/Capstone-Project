#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ipc_common.h"  // Include shared message structure


int main() {
    // Open the named pipe for reading sensor data
    int pipe_fd = open("inform_pipe", O_RDONLY);
    if (pipe_fd < 0) {
        perror("open pipe");
        return 1;
    }

    // Get or create message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }    
    
     char buffer[100];
    struct msgbuf message;
    message.mtype = 1;

    while (1) {
    
        int bytes = read(pipe_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("Decision module received: %s\n", buffer);

            int traffic, air, waste;
            sscanf(buffer, "Traffic:%d Air:%d Waste:%d", &traffic, &air, &waste);

            // Decision logic + task creation
            if (traffic > 30) {
                printf("[Traffic Control] High Congestion Detected (%d%%). Initiating rerouting...\n", traffic);
                strncpy(message.mtext, "High Traffic incoming from NH-9 ", MAX_TEXT);
                msgsnd(msgid, &message, sizeof(message.mtext), 0);
            } 
            else {
             printf("[Traffic Control] Low Traffic (%d%%). Maintain normal flow...\n", traffic);
            }

            if (air > 40) {
                printf("[Air Quality] Poor air quality (%d AQI). Issuing advisories...\n", air);
                strncpy(message.mtext, "Air Quality Critical in Central region", MAX_TEXT);
                msgsnd(msgid, &message, sizeof(message.mtext), 0);
            } else {
                printf("[Air Quality] Air quality in safe range (%d AQI)...\n", air);
            }

            if (waste > 55) {
                printf("[Waste Management] Waste bin critical (%d%% full). Dispatching collection...\n", waste);
                strncpy(message.mtext, "Garbage Bin Full at phase-4", MAX_TEXT);
                msgsnd(msgid, &message, sizeof(message.mtext), 0);
            } else {
                printf("[Waste Management] Waste level normal (%d%% full). No action needed.\n", waste);
            }
        }

        sleep(3);  // Delay to simulate processing time
    }
    
    
    return 0;
}


            

