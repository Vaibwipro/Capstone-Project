 #include <stdio.h>
#include <stdlib.h>
#include <string.h>  // for string functions like strncpy
#include <unistd.h>
#include <fcntl.h>  // for open , o_WRONLY
#include <sys/msg.h>  // for system message queue
#include <sys/stat.h> // for mkfifo
#include <pthread.h>  // for threads and mutex.

#define MSG_KEY 1206
#define MSG_TYPE 1


struct msg_buffer {
long msg_type; 
char data[100];  // Data Content (sensor string like "Traffic:6 Air:12 Waste:18"
};

char shared_data[100];  // Shared memory area for sensor data
pthread_mutex_t lock;

void* handle_traffic(void* arg) {
while (1) {
pthread_mutex_lock(&lock);
int t = 0;  // t is used to store the parsed traffic values.
for (int i = 0; shared_data[i] != '\0'; i++) // loops through the string until the null terminator.
{
if (shared_data[i] == 'T')  // if current character is 'T'(traffic sensor), we enter this block.
{
while (shared_data[i] < '0' || shared_data[i] > '9') i++; // skip all characters that are not digits (like space or :),so we use i++ to move to the digit 
sscanf(&shared_data[i], "%d", &t);  // Extracts the number using sscanf and reads number into t.
break;
}
}
printf("Traffic Handler: Traffic Level = %d\n", t);
pthread_mutex_unlock(&lock);
sleep(2);
}
return NULL;
}

void* handle_air(void* arg) {
while (1) {
pthread_mutex_lock(&lock);
int a = 0;
for (int i = 0; shared_data[i] != '\0'; i++) {
if (shared_data[i] == 'A') {
while (shared_data[i] < '0' || shared_data[i] > '9') i++;
sscanf(&shared_data[i], "%d", &a);
break;
}
}
printf("Air Quality Handler: Air Index = %d\n", a);
pthread_mutex_unlock(&lock);
sleep(2);
}
return NULL;
}

void* handle_waste(void* arg) {
while (1) {
pthread_mutex_lock(&lock);
int w = 0;
for (int i = 0; shared_data[i] != '\0'; i++) {
if (shared_data[i] == 'W') {
while (shared_data[i] < '0' || shared_data[i] > '9') i++;
sscanf(&shared_data[i], "%d", &w);
break;
}
}
printf("Waste Handler: Waste Level = %d\n", w);
pthread_mutex_unlock(&lock);
sleep(2);
}
return NULL;
}

int main() {
int msgid = msgget(MSG_KEY, 0666); // gets the message queue id.
if (msgid < 0) {
perror("msgget");
return 1;
}

if (access("inform_pipe", F_OK) == -1) {
if (mkfifo("inform_pipe", 0666) < 0) {
perror("mkfifo failed");
return 1;
}
}

int pipe_fd = open("inform_pipe", O_WRONLY); // open the named_pipe for writing only
if (pipe_fd < 0) {
perror("open pipe failed");
return 1;
}

struct msg_buffer msg;

pthread_t traffic_thread, air_thread, waste_thread;  // 3 threads created to handle different sensor types.
pthread_mutex_init(&lock, NULL);   // initialization 
pthread_create(&traffic_thread, NULL, handle_traffic, NULL); 
pthread_create(&air_thread, NULL, handle_air, NULL);
pthread_create(&waste_thread, NULL, handle_waste, NULL);

while (1) {
printf("Waiting for message from queue...\n");
 

if (msgrcv(msgid, &msg, sizeof(msg.data), MSG_TYPE, 0) < 0)  // waits for a message from the queue of type MSG_TYPE,When recieved it copies data into msg.
{
perror("msgrcv");
continue;
}

pthread_mutex_lock(&lock);
strncpy(shared_data, msg.data, sizeof(shared_data)); // Copying message data into shared memory so thread can use it.
pthread_mutex_unlock(&lock);


printf("Main Thread: Received = %s\n", shared_data);



if (write(pipe_fd, msg.data, strlen(msg.data) + 1) < 0) // writes the data to the named pipe by doing +1 the null terminator is sent too.
{
perror("write to pipe");
}
sleep(5);
}

pthread_mutex_destroy(&lock); // cleanup is done.
close(pipe_fd);
return 0;
}

	

