#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXDIRPATH 1024
#define MAXKEYWORD 256
#define MAXLINESIZE 1024
#define MAXOUTSIZE 2048

//message struct
struct message_s {
  long type;
  char keyword[MAXKEYWORD];
  char dirpath[MAXDIRPATH];
};




int main(int argc, char *argv[]) {

    //keyword, dirpath as arguments
    char *keyword = argv[1];
    char *dirpath= argv[2];

    //create message queue
    struct message_s message;
    int message_queue_id;
    key_t key;

    //generate key to send messages between client and server
    if ((key = ftok("ks_server.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    //Use the key to enter queue
    if ((message_queue_id = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }

    //Initialize message struct details
    message.type = 1;
    strcpy(message.keyword, keyword);
    strcpy(message.dirpath, dirpath);

    //printf("%s %s\n", message.keyword, message.dirpath);

    //Send message to queue
    if(msgsnd(message_queue_id, &message, MAXKEYWORD + MAXDIRPATH, 0) == -1) {
        perror("Error in msgsnd");
    }



    return 0;
}
