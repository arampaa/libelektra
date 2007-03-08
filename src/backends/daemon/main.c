/***************************************************************************
                   kdbd.c  -  The server for the daemon backend
                             -------------------
    begin                : Mon Dec 26 2004
    copyright            : (C) 2005 by Yannick Lecaillez
    email                : yl@itioweb.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/


/* Subversion stuff

$Id$

*/

#if HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for exit() */
#endif

#include <errno.h>
#include <string.h>

#include <pthread.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>

#include "sig.h"
#include "ipc.h"
#include "thread.h"
#include "kdbd.h"

#ifndef LOCALSTATEDIR
#define LOCALSTATEDIR "/var"
#endif

#ifndef KDBD_NAME
#define KDBD_NAME "kdbd"
#endif

#ifndef SOCKET_NAME
#define SOCKET_NAME LOCALSTATEDIR "/run/" KDBD_NAME "/elektra.sock"
#endif

#ifndef PID_NAME
#define PID_NAME LOCALSTATEDIR "/run/" KDBD_NAME "/" KDBD_NAME ".pid"
#endif

int numchildren = 0;
int limit = 20;
char remotepath[512];


void printstatus(void)
{
	fprintf(stdout, "kdbd: status: %d/%d\n", numchildren, limit);
}

int wait_nohang(int *wstat)
{
#ifdef HASWAITPID
	return waitpid(-1,wstat,WNOHANG);
#else
	return wait3(wstat,WNOHANG,(struct rusage *) 0);
#endif
}

void sigterm()
{
	unlink(SOCKET_NAME);
	unlink(PID_NAME);
	exit(0);
}

void sigchld()
{
	int wstat;
	int pid;
	while ((pid = wait_nohang(&wstat)) > 0) {
		fprintf(stdout, "kdbd: end %d status %d\n", pid, wstat);
		if (numchildren) --numchildren; printstatus();
	}
}


int main(int argc, char **argv)
{
	mode_t	m;
	int	t, s;
	int	trunc;
	pid_t pid;

	/* Uncomment setuid() call if the demon executable file is +s */
	/* setuid(0); */
	
	if ((pid=fork())) {
		/* The parent. */
		/* Log child pid and quit. */
		
		FILE *pidf;
		
		if ((pidf=fopen(PID_NAME,"w")) == NULL) {
			fprintf(stderr, "Error opening pid file %s: %s\n", PID_NAME, strerror(errno));
			exit(1);
		}
		fprintf(pidf,"%d",pid);
		fclose(pidf);
		
		exit(0);
	}
	
	/* force a superuniversal modern charset: UTF-8 */
	putenv("LANG=en_US.UTF-8");
	
/*	sig_block(sig_child);
	sig_catch(sig_child,sigchld);
	sig_catch(sig_term,sigterm);
	sig_ignore(sig_pipe); */

	s = ipc_stream();
	if ( s == -1 ) {
		/* perror(argv[0]); */
		return 1;
	}
	
	m = umask(0);
	if ( ipc_bind_reuse(s, SOCKET_NAME) == -1 ) {
		/* perror(argv[0]); */
		return 1;
	}
	umask(m);

	if ( ipc_local(s, 0, 0, &trunc) == -1 ) {
		/* perror(argv[0]); */
		return 1;
	}
	if (ipc_listen(s, 20) == -1) {
		/* perror(argv[0]); */
		return 1;
	}
	ndelay_off(s);

	/* fprintf(stderr,"uid=%d, euid=%d\n",getuid(),geteuid()); */
	
	for(;;) {
		t = ipc_accept(s,remotepath,sizeof(remotepath),&trunc);

		if (t == -1) {
			/* perror("kdbd");*/
			continue;
		}

		threadCreate(t, kdbd);
	}
}

