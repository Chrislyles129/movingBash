#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAXDIRPATH 1024
#define MAXKEYWORD 256
#define MAXLINESIZE 1024
#define MAXOUTSIZE 2048

//message struct
struct message_s {
  long type;
  char keyword[MAXKEYWORD];
  char dirpath[MAXDIRPATH];
  pid_t pid;
};

//reply struct
struct reply_s {
  long type;
  char reply[MAXOUTSIZE];
  int end;

};



int main(int argc, char *argv[]) {

    //keyword, dirpath as arguments
    char *keyword = argv[1];
    char *dirpath= argv[2];

    //create message queue
    struct message_s message;
    int message_queue_id;
    key_t key;


    message.pid = getpid();

    //create reply queue
    struct reply_s reply;
    int reply_queue_id;
    key_t key_r;


    //generate key to send messages to server
    if ((key = ftok("ks_server.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    //Use the key to enter message queue
    if ((message_queue_id = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }

    //USE PID AS KEY
    //generate key to send replies back to client
    if ((key_r = ftok("ks_client.c", message.pid)) == -1) {
        perror("ftok");
        exit(1);
    }

    //Use the key_r to enter reply queue
    if ((reply_queue_id = msgget(key_r, 0644 | IPC_CREAT)) == -1) {
      perror("msgget");
      exit(1);
    }

    //Initialize message struct details
    message.type = 1;
    reply.end = 0;
    strcpy(message.keyword, keyword);
    strcpy(message.dirpath, dirpath);

    // printf("%s", message.content);

    //Send message to queue
    if(msgsnd(message_queue_id, &message, MAXKEYWORD + MAXDIRPATH + sizeof(pid_t), 0) == -1) {
        perror("Error in msgsnd");
    }    

    //Wait until all files are searched + end if exit keyword
    while(reply.end == 0 && strcmp("exit", message.keyword) != 0){

      //Receive reply from file search
      if (msgrcv(reply_queue_id, &reply, MAXOUTSIZE+sizeof(int), 0, 0) == -1) {
        perror("msgrcv");
        exit(1);
      }

      //Print out the line found
      if(reply.end == 0){
        printf("%s",reply.reply);
      }

    }

    //Clear this client's queue
    if (msgctl(reply_queue_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}
