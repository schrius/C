#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 1024
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)


int cflag, dflag, wordcount, iof;
int skipflag, fieldflag, fieldcount, skipnum;  // unimplemented flags
int readline(int infd, int outfd, char *buf, int nbytes);
void display(char *val, int numread);
int writefile(char *val, int numread, int targetfile);
void wrongopt();

int main(int argc, char *argv[]) {
   char buf[BUFSIZE];
   char opt;
   int infile, outfile, ret;

    if (argc < 1)
    	wrongopt();

	while ((opt = getopt(argc, argv, "cdf:s:")) != -1)
		switch (opt) {
		case 'c':
			cflag = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 's':	// unimplemented flag
			if(optarg<0)
				wrongopt();
			skipflag = 1;
			skipnum = strtol(optarg, NULL, 10);
			break;
		case 'f':	// unimplemented flag
			if(optarg<0)
				wrongopt();
			fieldflag = 1;
			fieldcount = strtol(optarg, NULL, 10);
			break;
		case '?':
		default:
			wrongopt();
	}

	iof = argc - optind;
	switch(iof){
	case 1:
	   if ((infile = open(argv[optind], READ_FLAGS)) == -1) {
	      perror("Failed to open input file");
	      return 1;
	   }
	   break;
	case 2:;
		if ((infile = open(argv[optind], READ_FLAGS)) == -1) {
		      perror("Failed to open input file");
		      return 1;
		  }
	   if ((outfile = open(argv[optind+1], WRITE_FLAGS, WRITE_PERMS)) == -1) {
	      perror("Failed to create output file");
	      return 1;
	   }
	   break;
	default:
		wrongopt();
		}

   ret = readline(infile, outfile, buf, BUFSIZE);
   if(ret ==-1)
   perror("readline error.\n");

   if( iof>=1)
    if (close (infile) == -1)
    perror ("close failed.\n");

    if(iof>=2)
    if (close (outfile) == -1)
    perror ("close failed.\n");

   return 0;
}

int readline(int infd, int outfd, char *buf, int nbytes) {
   int numread = 0, prevnum = 0;
   int returnval;
   char *prev = NULL;

   while (numread < nbytes - 1 ) {
      returnval = read(infd, buf + numread, 1);

      if ((returnval == -1) && (errno == EINTR))
         continue;
      if ( (returnval == 0)&&( numread == 0)&& (wordcount >=1)){
      			if(strcmp(prev, buf) != 0){
      					if(dflag ==1 && wordcount>=2){
      						display(prev, prevnum);
      						if(iof==2)
        					if(writefile(prev, prevnum, outfd)==-1)
        					      perror("Failed to write to output file");
      					}
      					if(dflag != 1){
        					display(prev, prevnum);
        					if(iof==2)
        					if(writefile(prev, prevnum, outfd)==-1)
        					      perror("Failed to write to output file");
      					}
      					wordcount = 1;
      				}
      			return 0;
      	  	  }
      if (returnval == 0 && numread > 0 && buf[numread] != '\n'){
    		display(buf, numread);
    		if(iof==2)
    		if(writefile(buf, numread, outfd)==-1)
    		      perror("Failed to write to output file");
    	   return 0;
      }
      if ( returnval == EOF)
    	  return 0;
      if (returnval == 0)
			break;
      if (returnval == -1)
          return -1;

        numread++;


    if(buf[numread-1] == '\n' || buf[numread-1] == '\r' ){
    		if(prev == NULL){
    			prev = malloc(BUFSIZE);
    			strcpy(prev, buf);
    			wordcount++;
    			prevnum = numread;
    			buf = malloc(BUFSIZE);
    			numread = 0;
    			}
    		else {
    			if(strcmp(prev, buf) != 0){
    					if(dflag ==1 && wordcount>=2){
    						display(prev, prevnum);
    						if(iof==2)
        					if(writefile(prev, prevnum, outfd)==-1)
        					      perror("Failed to write to output file");
    					}
    					if(dflag != 1){
        					display(prev, prevnum);
        					if(iof==2)
        					if(writefile(prev, prevnum, outfd)==-1)
        					      perror("Failed to write to output file");
    					}
    					wordcount = 1;
    			}
    			else  wordcount++;
    					prevnum = numread;
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
	if(cflag == 1){
		printf("%d ", wordcount);
	}
    for(int i = 0; i < numread; i++)
    	printf("%c", val[i]);
}

int writefile(char *val, int numread, int outfd){
	int written;
	if((written = write(outfd, val, numread)) == -1)
	return written;
}

void wrongopt()
{
	fprintf(stderr, "usage: uniq [-c | -d] [-f fields] [-s chars] [input [output]]\n");
	exit(1);
}
