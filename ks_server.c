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

struct message_s {
  long type;
  char keyword[MAXKEYWORD];
  char dirpath[MAXDIRPATH];
};

//PLACEHOLDER CODE TO READ FILES
void readFile(char *File, char *keyword){

    //Read in file input
    FILE* ptr = fopen(File, "r");
    char *line = malloc(MAXLINESIZE);
    size_t len = 0;
    ssize_t read;

    if (ptr == NULL) {
        printf("no such file.");
        exit(1);
    }


    char *words = malloc(MAXLINESIZE);
    char *search = malloc(MAXLINESIZE);

    while ((read = getline(&line, &len, ptr)) != -1) {
        // printf("%s\n", line);
        strcpy(search, line);
        words = strtok(search, " ");
        while( words != NULL ) {
          words[strcspn(words, "\n")] = '\0';
          // printf("%s %d\n", words, strcmp(words, keyword));
            if(strcmp(words, keyword) == 0){
                printf("%s:%s", keyword, line);
                // printf("%s\n", line);
                break;
            }
            words = strtok(NULL, " ");
        }
        
    }


    printf("\n");
    fclose(ptr);

    free(line);


}

void readFolder(char *dirpath, char *keyword){
  struct dirent *de;  // Pointer for directory entry 

  // opendir() returns a pointer of DIR type.  
  DIR *dir = opendir(dirpath); 
  int count = 0;

  if (dir == NULL)  // opendir returns NULL if couldn't open directory 
  { 
      printf("Error opening directory" ); 
      return; 
  } 

  // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
  // for readdir() 
  char *File = malloc(MAXDIRPATH);
  while ((de = readdir(dir)) != NULL){
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
        continue;
    }
    count++;
    // printf("%s\n", de->d_name); 
    sprintf(File, "%s%s", dirpath, de->d_name);
    printf("%s\n", File);
    readFile(File, keyword);

  }
  // printf("%d\n", count);
  closedir(dir);     
} 



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
    if (msgrcv(message_queue_id, &message, MAXKEYWORD + MAXDIRPATH, 0, 0) == -1) {
      perror("msgrcv");
      exit(1);
    }
    printf("%s %s\n\n", message.keyword, message.dirpath);
    readFolder(message.dirpath, message.keyword);
  }

  // if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
  //     perror("msgctl");
  //     exit(1);
  // }

  pthread_create(&tid, &attr, reader, message.content); //create a thread with the thread id, default attributes, do the reader routine, and with message contents

  return 0;
}
