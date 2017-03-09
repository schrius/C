#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFSIZE 1024
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)

int readline(int infd, int outfd, char *buf, int nbytes);
void display(char *val, int numread);
int writefile(char *val, int numread, int targetfile);

int main(int argc, char *argv[]) {
   char buf[BUFSIZE];
   int infile, outfile, numbit;

    if (argc < 1) {
      fprintf(stderr, "Usage: %s from_file to_file\n", argv[0]);
      return 1;
   }

   if ((infile = open(argv[1], READ_FLAGS)) == -1) {
      perror("Failed to open input file");
      return 1;
   }
    if(argc > 2)
   if ((outfile = open(argv[2], WRITE_FLAGS, WRITE_PERMS)) == -1) {
      perror("Failed to create output file");
      return 1;
   }

   readline(infile, outfile, buf, BUFSIZE);

/*
    if (close (infile) == −1)
    perror ("close failed.\n");

    if (close (outfile) == −1)
    perror ("close failed.\n");
*/
   return 0;
}

int readline(int infd, int outfd, char *buf, int nbytes) {
   int numread = 0;
   int returnval;
   char *prev = NULL;

   while (numread < nbytes - 1) {
      returnval = read(infd, buf + numread, 1);
      if ((returnval == -1) && (errno == EINTR))
         continue;
      if ( (returnval == 0) && (numread == 0) )
         return 0;
      if (returnval == 0)
         break;
      if (returnval == -1)
         return -1;
	printf("%c", buf[numread]);
        numread++;

    if(isspace(buf[numread-1])){
    		if(prev == NULL){
    			prev = malloc(BUFSIZE);
    			strcpy(prev, buf);
    			display(buf, numread);
    			if(outfd)
    				writefile(buf, numread, outfd);
    			buf = malloc(BUFSIZE);
    			numread = 0;
    			}
    		else {
    			if(strcmp(prev, buf) != 0){
    					display(buf, numread);
    					if(outfd)
    						writefile(buf, numread, outfd);
    					}
    					strcpy(prev, buf);
    					buf = malloc(BUFSIZE);
    					numread = 0;
    		}
    	}
   }
   errno = EINVAL;
   return -1;
}

void display(char *val, int numread){
    for(int i = 0; i < numread; i++)
    		printf("%c", val[i]);
}

int writefile(char *val, int numread, int outfd){
	int written;
	if((written = write(outfd, val, numread)) == -1)
	return written;
}
