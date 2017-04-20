/*
 * grep.c
 *
 *  Created on: Apr 10, 2017
 *      Author: dathan
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define BUFSIZE 2048
#define MAXFILE 10

// Text color
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; //stdout mutex
pthread_mutex_t PATTERN_mutex = PTHREAD_MUTEX_INITIALIZER; // PATTERN mutex
int TotalLines = 0;	// total lines found
char *PATTERN;		// PATTERN string

void * searchfile(void *);  // search function

int main(int argc, char *argv[]){
pthread_t thread_id[MAXFILE];  // array of thread id
int Lines;		// line to store return lines

	// at least 2 arguments and no more than 10 files
	  if ((argc < 2) || (argc > MAXFILE+1)) {
	     fprintf(stderr,"%s: [PATTERN],[file1],[file2]...[file%d]\n", *argv, MAXFILE);
	     exit(1);
	  }
	  // assign the pattern value
	  PATTERN = argv[1];

	  // create threads to search each files in the argv
	for(int i = 0; i < argc-2; i++){
	      if (pthread_create(&thread_id[i], NULL, searchfile, (void *) argv[i + 2]) != 0) {
	         fprintf(stderr, "pthread_create failure\n");
		 exit(2);
	     }
	}
	// wait for all thread to join.
	  for (int i = 0; i < argc-2; i++)   {
	      if ( pthread_join(thread_id[i], (void **)&Lines) > 0){
	           fprintf(stderr, "pthread_join failure\n");
	      }
	 // add all lines found in each thread.
	      TotalLines += Lines;
	  }
	 // display total lines and destroy the mutex.
	  printf("Total lines found:" KMAG "%d\n" KWHT, TotalLines);
	  pthread_mutex_destroy(&print_mutex);
	  pthread_mutex_destroy(&PATTERN_mutex);
	return 0;
}

// search thread function
void * searchfile(void * arg){
	  int lines = 0, lineFound = 0;  //line corresponding to the current line and lines found in the files
	  FILE * fp;
	  char str[BUFSIZE];  // a string to store the line

	  // open file; display error and return 0 if failed to open.
	  if((fp = fopen((char *)arg, "r")) == NULL){
	  perror("Failed to open input file");
	  return (void *)(0);
	  }

	  //read the whole file
	  while(fgets(str, BUFSIZE, fp) != NULL){
		  lines++;
		  // lock before search PATTERN in the line.
		  pthread_mutex_lock (&PATTERN_mutex);
		  if(strstr(str, PATTERN) != NULL){
			  // lock this output before display
			  pthread_mutex_lock (&print_mutex);
			  lineFound++;
			  printf("%s:"KGRN "%d" KWHT ":%s", (char *)arg, lines, str);
			  // unlock after print
			  pthread_mutex_unlock (&print_mutex);
		  }
		  // unlock the PATTERN mutex
		  pthread_mutex_unlock (&PATTERN_mutex);
	  }
	  pthread_mutex_lock (&print_mutex);
	  printf("Lines found in %s: " KCYN  "%d\n" KWHT, (char *)arg, lineFound);
	  pthread_mutex_unlock (&print_mutex);
	  if(fclose(fp)==EOF){
		  perror("Failed to close input file");
		  return (void *)(0);
	  }
	  //since my system is 64 bit, void * is 64 bit, so it needs to be cast to long and then void*
	  pthread_exit((void *)(long)lineFound);
}

