#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

//HELLO
//client sends the keyword and directory path to the server
//server gets the keyword and directory path and creates threads for every file
//the threads browse the lines of the files to get all the instances of the keywords


//this uses fork to create child processes, which will then create threads to  search the files and the directory
//server controls the process queue and has the file "system"
#define MAXDIRPATH 1024
#define MAXKEYWORD 256
#define MAXLINESIZE 1024
#define MAXOUTSIZE 2048

struct message_s {
  long type;
  char content[MAXKEYWORD];
  char dirPath[MAXDIRPATH];
};

//the routine that the threads will be running
void *reader(void *param){

}

int main(void) {
  struct message_s message;
  int message_queue_id;
  key_t key;

  pthread_t tid; //thread itendifier
  pthread_attr_t attr; //thread attributes

  // printf("what");
  if ((key = ftok("ks_server.c", 1)) == -1) {
    perror("ftok");
    exit(1);
  }

  if ((message_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }

  for(;;) {
    if (msgrcv(message_queue_id, &message, MAXKEYWORD, 0, 0) == -1) {
      perror("msgrcv");
      exit(1);
    }
    printf("%s\n", message.content);
  }

  // if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
  //     perror("msgctl");
  //     exit(1);
  // }

  pthread_create(&tid, &attr, reader, message.content); //create a thread with the thread id, default attributes, do the reader routine, and with message contents

  return 0;
}
