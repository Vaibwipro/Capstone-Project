#include <stdio.h> // for printf , snprintf
#include <stdlib.h> 
#include <string.h> // for strlen() , strcpy.
#include <unistd.h> // for sleep()
#include <sys/msg.h> // to use system message queues(msgget , msgsnd).
#include<fcntl.h>
#include<sys/stat.h>

#define MSG_KEY 1206
#define MSG_TYPE 1

struct msg_buffer {
    long msg_type;
    char data[100];
};

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666); // creates or access the message queue with key 4525.
    if (msgid < 0) {
        perror("msgget");
        return 1;
    }
    
    struct msg_buffer msg;
     msg.msg_type = MSG_TYPE;

    int traffic = 6, air = 12, waste = 18; // initialize sensor values.

    while (1) // infinite loop - keeps sending new data every few seconds.
    {
        snprintf(msg.data, sizeof(msg.data), "Traffic:%d Air:%d Waste:%d", traffic, air, waste); // formats the string like "Traffic:5 Air:15 Waste:25" and stores in msg.data.
        
        if(msgsnd(msgid, &msg, sizeof(msg.data), 0) < 0)  // sends message to message queue. synatx- int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
        {
        perror("msgsnd failed");
        }
        else
        {
        printf("Sensor Sent: %s\n", msg.data);
        }
        traffic += 6;      // simulating increasing sensor values on each loop iteration.
        air += 3; 
        waste += 4;

        sleep(6); // waits or sleeps for 3 seconds before sending the next data.
    }
    return 0;
}	

