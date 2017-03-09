/*-------------------------------------
 * Dathan Huang
 * CISC 3350
 * Date March 9, 2017
 */

#define MAXLIMIT 4096
#define MAXFILE 4096
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[]){
    int pr_limit, pr_count = 0, index = 0;  // process limit and numbers of argument for excvp
    int ret_code;
    pid_t ret_pid, child_pid;
    char* str;
    char* excfile[MAXFILE];
    char* buf;
    int sleep_Time, repeat_Factor;
    char skip[] = " \t\n";	// delimiters
    size_t sl;

    if(argc!=2){
        fprintf(stderr,"Usage: %s string\n", argv[0]);
        return 1;
    }

    pr_limit = atoi(argv[1]);

    if(pr_limit<=0||pr_limit>MAXLIMIT){
        fprintf(stderr,"%s: permissible running processes must greater than 1 or less than %d\n ", argv[1], MAXLIMIT);
    }

    //read stdin to the end
    while(getline(&str, &sl,stdin)!=EOF){
    	if(str == NULL){
    	    	errno = EINVAL;
    	    	perror("Invalid process or cannot get a line.\n");
    	    	exit(EXIT_FAILURE);
    		}
        if( pr_count >= pr_limit){
        	ret_pid = wait(&ret_code);
         if(ret_pid== -1 && errno !=ECHILD){
        	 perror("Wait() failed\n");
        	 exit(EXIT_FAILURE);
         	 }
         else {
        	 printf("Child Process %d finished. Return code: %d\n", ret_pid, ret_code);
        	 pr_count--;
        	}
        	}

        	child_pid = fork();
        	if(child_pid == -1){
        	        		exit(EXIT_FAILURE);
        	        		return 1;
        	        	}
        	else if(child_pid == 0){
        		buf = strtok(str, skip);	//copy string end at the delimiters
        		if(buf!=NULL&&index<MAXFILE){
        			free(excfile[index]);
        			excfile[index] = malloc(strlen(buf));
        			strcpy(excfile[index], buf);
        			index++;
        		while((buf = strtok (NULL,skip))!= NULL){
        			free(excfile[index]);
        			excfile[index] = malloc(strlen(buf));
        			strcpy(excfile[index], buf);
        			index++;
        		}
        		index=0;
				if(execvp(excfile[0], excfile)==-1)
				perror("Child failed to exec");
				return 1;
        	}
        		else {
        			perror("Invalid process.\n");
        			exit(EXIT_FAILURE);
        		}
        	}
        	else {
        		pr_count++;
        		if((ret_pid = waitpid(-1, NULL, WNOHANG)) == -1){
        			perror("Wait() failed");
					return 1;
        		}
        		else if(ret_pid>0){
        			pr_count--;
    				printf("Child %d has returned.\n", ret_pid);
        		}
        	}
        }
    //wait for other children processes to return.
	while(pr_count>0 && (ret_pid = wait(NULL)) !=-1){
	printf("Child %d has returned. Remind %d children process.\n", ret_pid, pr_count);
	pr_count--;
	}
    // last check, ensure no children left behind.
    if(ret_pid==-1 && pr_count!= 0){
    	perror("Wait() failed");
    	exit(EXIT_FAILURE);
    }
    else printf("All Children processes complete.\n");


	return 0;
}
