/*xsh_hello.c - xsh_hello */
#include <xinu.h>
#include <stdio.h>


/*--------------------------------------------
 * xsh_hello - print Hello on command executio
 */
shellcmd xsh_hello(int nargs, char *args[]){
	/* Check argument count */

	if (nargs > 2) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",args[0]);
		return 1;
	}
	if (nargs < 2) {
		fprintf(stderr, "%s: no arguments given\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",args[0]);
		return 1;
	}
	printf("Hello %s, Welcome to the world of Xinu!!", args[1]);
	return 0;
}
