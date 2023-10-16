#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

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

//message struct
struct message_s {
  long type;
  char keyword[MAXKEYWORD];
  char dirpath[MAXDIRPATH];
};

struct threadParams{
  char *file;
  char *keyword;
};

//global count
int count;
//global files array
char* files[10];

//reply struct
struct reply_s {
  long type;
  char reply[MAXOUTSIZE];
  int end;
};


//End client process
void endClient(){
  // printf("END\n");
  //initialize reply queue
  struct reply_s reply;
  int reply_queue_id;
  key_t key_r;
  reply.type = 1;

  //Mark reply to end client process
  reply.end = 1;  
  
  //USE PID as key
  //generate key to send replies to client
  if ((key_r = ftok("ks_client.c", 1)) == -1) {
    perror("ftok");
    exit(1);
  }

  //Use the key_r to enter reply queue
  if ((reply_queue_id = msgget(key_r, 0644 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }  

  //Send reply to end client
  if(msgsnd(reply_queue_id, &reply, MAXOUTSIZE + sizeof(int), 0) == -1) {
      perror("Error in msgsnd");
  } 

  return;
}


//Send reply to client
void sendReply(char *output){

  //initialize reply queue
  struct reply_s reply;
  int reply_queue_id;
  key_t key_r;
  reply.type = 1;
  reply.end = 0;

  //USE PID as key
  //generate key to send replies to client
  if ((key_r = ftok("ks_client.c", 1)) == -1) {
    perror("ftok");
    exit(1);
  }

  //Use the key_r to enter reply queue
  if ((reply_queue_id = msgget(key_r, 0644 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }  

  //Line to send back
  strcpy(reply.reply, output);

  //Send reply to queue
  if(msgsnd(reply_queue_id, &reply, MAXOUTSIZE, 0) == -1) {
      perror("Error in msgsnd");
  } 

  return;
}


//READ FILES
void readFile(char *File, char *keyword){

    //Read in file input
    FILE* ptr = fopen(File, "r");

    //Variables to read line by line
    char *line = malloc(MAXLINESIZE);
    size_t len = 0;
    ssize_t read;

    //Validate file
    if (ptr == NULL) {
        printf("no such file.\n");
        exit(1);
    }

    //variables to search through line  
    char *words = malloc(MAXLINESIZE);
    char *search = malloc(MAXLINESIZE);

    //variable to store output
    char *output = malloc(MAXOUTSIZE);

    //read line by line
    while ((read = getline(&line, &len, ptr)) != -1) {
        // printf("%s\n", line);

        //tokenize a line (delimited by " ")
        strcpy(search, line);
        words = strtok(search, " ");

        //get next word token until none left
        while( words != NULL ) {

          //remove new lines
          words[strcspn(words, "\n")] = '\0';

          // printf("%s %d\n", words, strcmp(words, keyword));
          
          //if word token matches keyword, send line to client
          if(strcmp(words, keyword) == 0){
              // printf("%s:%s", keyword, line);
              // printf("%s\n", line);

              sprintf(output, "%s:%s", keyword, line);

              //SEND REPLY
              sendReply(output);

              break;
          }

          //get next word token
          words = strtok(NULL, " ");
        } 
    }
    printf("\n");

    //close and free
    fclose(ptr);
    free(line);

    return;

}

//READ DIRECTORY
void readFolder(char *dirpath, char *keyword){
  //directory entry 
  struct dirent *de;  

  //directory 
  DIR *dir = opendir(dirpath); 

  //track viable files to open
  count = 0;

  //Validate directory
  if (dir == NULL){
      printf("Error opening directory\n" ); 
      return; 
  } 

  //Vars for file details
  char *File = malloc(MAXDIRPATH);
  struct stat buffer;

  //readdir gets next directory entry
  while ((de = readdir(dir)) != NULL){
    
    //Get full path and file details
    sprintf(File, "%s%s", dirpath, de->d_name);
    stat(File, &buffer);

    //Don't include sub directories
    if(S_ISREG(buffer.st_mode)) {

      count++;
      // printf("%s\n", de->d_name); 

      //Read this file
      printf("%s\n", File);
      readFile(File, keyword);

    }
    
  }
  // printf("%d\n", count);

  //close and free
  closedir(dir);  
  return;   
} 

//where we read the file and send message back to client
void *reading(void *param){
  printf("in the thread\n");
  struct threadParams *data = param;
  //have it read the file
  printf("%s %s\n", data->file, data->keyword);
  readFile(data->file, data->keyword);

  //send the message

  pthread_exit(0);
}


int main(void) {
  //initialize message queue
  struct message_s message;
  struct threadParams parameters; 
  int message_queue_id;
  key_t key;
  pid_t x;

  pthread_t tid; //thread itendifier
  pthread_attr_t attr; //thread attributes

  //generate key to send messages between client and server
  if ((key = ftok("ks_server.c", 1)) == -1) {
    perror("ftok");
    exit(1);
  }

  //Use the key to enter queue
  if ((message_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }

  //Loop forever
  for(;;) {

    //Receive message from queue
    //should fork when we get the message, and then in the child process, create the threads
    if (msgrcv(message_queue_id, &message, MAXKEYWORD + MAXDIRPATH, 0, 0) == -1) {
      perror("msgrcv");
      exit(1);
    }
    
    //Exit if exit keyword
    if(strcmp("exit", message.keyword) == 0){
      break;
    }

    // printf("%s %s\n\n", message.keyword, message.dirpath);
    
    //Read the folder passed in message
    readFolder(message.dirpath, message.keyword);

    //create a child when a message is received
    x = fork();
    
    //create the threads in the child for amount of files in directory
    if(x == 0){ //child
      for(int i = 0; i < count; i++){
        pthread_attr_init(&attr);
        parameters.file = files[count];
        parameters.keyword = message.keyword;
        //create a thread with the thread id, default attributes, do the reader routine, and with message contents
        pthread_create(&tid, &attr, reading, &parameters); 
        pthread_join(tid, NULL);
      }
    }
    if(x > 0){ //this is the parent
      printf("This is the parent process\n");
      wait(NULL); //wait for the child to complete
      printf("%s %s\n\n", message.keyword, message.dirpath);
    }

    //Send reply to end client
    endClient();

  }


  //clear master message queue
  if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
      perror("msgctl");
      exit(1);
  }

  

  return 0;
}
