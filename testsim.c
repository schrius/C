/*
 * testsim.c
 *
 *  Created on: Mar 7, 2017
 *      Author: dathan
 */
#include <stdio.h> /* fprintf */
#include <stdlib.h> /* atoi */
#include <unistd.h> /* sleep */

int main(int argc, char *argv[]) {
	int stime, repeat;

	if ( argc != 3 ) {
		fprintf(stderr, "Usage %s [sleep time] [loops]\n", argv[0]);
		return 1;
	}

	stime = atoi(argv[1]);

	for (int i = 0; i < repeat; i++) {
		sleep(stime);
		fprintf(stderr, "pid: %d\n", getpid());
	}

	return 0;
}
