#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/file.h> //ch put it into the header
#include <fcntl.h> //ch put it into the header
#define FILEPATH "./inmsg/temp.txt"

#define COL_EXLO "\x1b[31m"
#define COL_DlO  "\x1b[32m"
#define COL_R    "\x1b[0m"

#define SERVER_PORT 8888

#define MAX 40962

#if defined(DEBUG) && DEBUG
#define log(x,...) fprintf(stderr,COL_DlO ">> on the line %d " x "\n" COL_R,__LINE__,##__VA_ARGS__);
#else
#define log(x,...)
#endif

#if defined(EXLOG) && EXLOG
#define exit_ha(x,...) \
		if(errno != 0){int errno_s = errno; fprintf(stderr,COL_EXLO x "on line : %d ; errno = %d : %s  \n" COL_R,##__VA_ARGS__, __LINE__,  errno_s, strerror(errno_s)); }\
		else fprintf(stderr, "on line %d" x "\n", __LINE__,##__VA_ARGS__);\
		exit(1); //ch for now
#else
#define exit_ha(x,...) exit(1);
#endif
#endif
