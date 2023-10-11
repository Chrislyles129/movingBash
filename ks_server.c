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

struct message_s {
  long type;
  char content[MAXKEYWORD];
};

int main(void) {
  struct message_s message;
  int message_queue_id;
  key_t key;

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


  return 0;
}
