#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h>
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

//message struct
struct message_s {
  long type;
  char keyword[MAXKEYWORD];
  char dirpath[MAXDIRPATH];
};

int count;

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
        printf("no such file.");
        exit(1);
    }

    //variables to search through line  
    char *words = malloc(MAXLINESIZE);
    char *search = malloc(MAXLINESIZE);

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
          
          //if word token matches keyword, print line
          if(strcmp(words, keyword) == 0){
              printf("%s:%s", keyword, line);
              // printf("%s\n", line);
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
      printf("Error opening directory" ); 
      return; 
  } 

  
  char *File = malloc(MAXDIRPATH);

  //readdir gets next directory entry
  while ((de = readdir(dir)) != NULL){
    //Don't include parent paths
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
        continue;
    }
    count++;
    // printf("%s\n", de->d_name); 

    //Get full file path
    sprintf(File, "%s%s", dirpath, de->d_name);
    printf("%s\n", File);
    readFile(File, keyword);

  }
  // printf("%d\n", count);

  //close and free
  closedir(dir);     
} 

void *reading(void *param){
  //have it read the file

  //close the thread
  return NULL;
}

int main(void) {
  struct message_s message;
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
    //create a child when a message is received
    x = fork();
    
    //create the threads in the child for amount of files in directory
    if(x == 0){ //child
      for(int i = 0; i < count; i++){
        pthread_attr_init(&attr);
        //create a thread with the thread id, default attributes, do the reader routine, and with message contents
        pthread_create(&tid, &attr, reading, message.keyword); 
        pthread_join(tid, NULL);
      }
    }
    if(x > 0){ //this is the parent
      printf("This is the parent process");
      wait(NULL); //wait for the child to complete
      printf("%s %s\n\n", message.keyword, message.dirpath);
    }
    
    
    //Read the folder passed in message
    readFolder(message.dirpath, message.keyword);
  }

  // if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
  //     perror("msgctl");
  //     exit(1);
  // }

  

  return 0;
}
