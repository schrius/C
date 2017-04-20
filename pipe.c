/*
 * pipe.c
 *
 *  Created on: Mar 31, 2017
 *      Author: dathan
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// declare static Child_pid pointer, hold all childpid created by popen.
static pid_t *child_pid = NULL;
static int Max_fd;  // maximum file descriptor

#define	BUFSIZE	255

FILE * popen(const char *command, const char *mode);
int pclose(FILE *stream);

int main(int argc, char *argv[]){
	FILE	*fp, *fp1, *wfp; // file stream
	int status;
	char buff[BUFSIZE];
	// declare 10 Greek letter which will pass to file stream.
char *Greek[10] = { "Lambda", "echo", "beta", "alpha", "Omega", "delta", "theta", "psi", "epsilon", "Sigma"};

	// create pipe to display file list of the current directory with some options
	if((fp = popen("ls -sail", "r")) == NULL){
        perror("popen()");
		exit(1);
		  }

	while(fgets(buff, sizeof(buff), fp)!=NULL){
		  printf("%s", buff);
	}
	// Close the pipe
	status = pclose(fp);
    if (status == -1)
    	perror("pclose() failed.\n");

    // create pipe to display file list of the current directory without options
	if((fp1 = popen("ls","r")) == NULL){
        perror("popen()");
		exit(1);
	}

	while(fgets(buff, BUFSIZE, fp1)!= NULL)
		  printf("LS-> %s", buff);

		// Close the pipe
	status = pclose(fp1);
     if (status == -1)
        perror("pclose() failed.\n");

        // Create pipe, sorting all string in decending order.
        if (( wfp = popen("sort", "w")) == NULL)
        {
                perror("popen()");
                exit(1);
        }

        // write the strings to the pipe
        for(int i = 0; i < 10; i++) {
                fputs(Greek[i], wfp);
                fputc('\n', wfp);
        }

        // Close the pipe
        status = pclose(wfp);
        if (status == -1)
        	perror("pclose() failed.\n");

        // trying to close a file stream that is already close will cause error.
    /*    status = pclose(wfp);
        if (status == -1)
        	perror("pclose() failed.\n");*/

	return 0;
}

FILE * popen(const char *command, const char *mode){
	int pipefd[2]; // file descriptor for pipe
	pid_t pid;
	FILE *fp;  // file pointer
	// check read or write
	if(*mode != 'w' && *mode !='r' || mode[1] != 0) {
		errno = EINVAL; // set errno if mode is not 'r' or 'w' and return NULL
		return NULL;
	}

	// allocate child_pid if popen is used at first time
	if (child_pid == NULL) {
		errno = 0; // clear errono before call sysconf
		Max_fd = sysconf(_SC_OPEN_MAX); // get the limit
			if(Max_fd == -1){
				if(errno == 0)
				Max_fd = 1024;//if limit is not determinate, used 1024 instead.
				else {
					errno = EMFILE;
					return NULL; // return NULL if errno is set.
				}
			}
	        if ((child_pid = calloc(Max_fd, sizeof(pid_t))) == NULL)
	            return NULL; //return NULL if failed to allocate memory.
	    }

	// return NULL if cannot create pipe.
	if(pipe(pipefd) == -1)return NULL;

	// return NULL if failed to fork process
	if((pid = fork()) == -1){
		return NULL;
	}
	// child process
	else if(pid == 0){
		if(*mode == 'w'){
            close(pipefd[1]); // close output in child process
            dup2(pipefd[0], STDIN_FILENO); // redirect input file descriptor
            close(pipefd[0]);	// close the old file descriptor
        }
		else {
			close(pipefd[0]); // close input in child process
			dup2(pipefd[1], STDOUT_FILENO); // redirect output file descriptor
			close(pipefd[1]); // close the old file descriptor
    		}

        // close all unneeded file descriptors before excel()
        for (int child = 0; child < Max_fd; child++)
            if (child_pid[child] > 0){
                close(child);
            }
		// execute the program specified by command, invoked Shell utility
        execl("/bin/sh", "sh", "-c",  command, (char *)0);
        // return command not found if failed to execute program
        exit(127);
	}
	// parent process
	else {
        if (*mode == 'w') {
            close(pipefd[0]);  // close input for write mode
            if ((fp = fdopen(pipefd[1], mode)) == NULL)// convert file descriptor to file stream
                return NULL;  // return NULL if failed
        }
        else {
        	close(pipefd[1]); // close output for read mode
            if ((fp = fdopen(pipefd[0], mode)) == NULL) // convert file descriptor to file stream
                return NULL;	// return NULL if failed
        }
        // record child process for the file descriptor
        child_pid[fileno(fp)] = pid;
        // return file stream
        return(fp);
}
}

int pclose(FILE *stream){
    int     fd, status; // file descriptor and return status
    pid_t   pid;

    // if child_pid is NULL which means popen never call, set errno and return -1
    if(child_pid == NULL){
    	errno = EINVAL;
    	return -1;
    }

    // get the file descriptor associated with the file pointer
    if((fd = fileno(stream)) == -1){
        errno = EINVAL; //if invalid FILE stream, set errno and return -1
        return -1;
    }

    // if file stream is not opened by calling popen return -1
    if ((pid = child_pid[fd]) == 0) {
        errno = EINVAL;
        return -1;
    }

    //reset the corresponding child_pid
    child_pid[fd] = 0;
    //if fclose return EOF, error is detected.
    if (fclose(stream) == EOF)
        return -1;

    // wait for child process to return
    while (waitpid(pid, &status, 0) == -1){
    // if waitpid() reutrn error other than EINTR, return -1 to indicate failure.
        if (errno != EINTR)
            return-1 ;
    }
    // return the terminated status
    return(status);
}
