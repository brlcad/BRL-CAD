/*
 *			D B C P . C
 *
 *	Double-buffered copy program for UNIX
 *
 *	Usage:    dbcp {nblocks} < inputfile > outputfile
 *
 *  Author -
 *	Doug Kingston
 *  
 *  Source -
 *	Davis, Polk, and Wardwell
 *	Chase Manhattan Building
 *	New York, NY
 *  
 *  Distribution Status -
 *	Public Domain, Distribution Unlimitied.
 */
#ifndef lint
static char RCSid[] = "@(#)$Header$ (BRL)";
#endif

#include "conf.h"

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "machine.h"
#include "externs.h"			/* For getopt and malloc and atoi */

#define	STOP	0170
#define	GO	0017

#define P_RD	0
#define P_WR	1

typedef int pipefds[2];

static int	pid;
static long	count;

static int	verbose;

static char	errbuf[BUFSIZ];

static char	usage[] = "\
Usage:  dbcp [-v] blocksize < input > output\n\
	(blocksize = number of 512 byte 'blocks' per record)\n";

void prs();

/*
 *			M A I N
 */
main (argc, argv)
int	argc;
char	**argv;
{
	register char	*buffer;
	register unsigned int	size;
	register unsigned int	nread;
	int	rfd;		/* pipe to read message from */
	int	wfd;		/* pipe to write message to */
	int	exitval;
	int	saverrno;
	int	waitcode;
	char	msgchar;
	pipefds par2chld, chld2par;
	int	c;

	while ( (c = getopt( argc, argv, "v" )) != EOF )  {
		switch( c )  {
		case 'v':
			verbose++;
			break;
		default:
			(void)fputs(usage, stderr);
			exit(1);
		}
	}

	if( optind >= argc )  {
		(void)fputs(usage, stderr);
		exit(2);
	}
	size = 512 * atoi(argv[optind]);

	setbuf (stderr, errbuf);
	if ((buffer = malloc(size)) == NULL) {
		fprintf(stderr, "dbcp: Insufficient buffer memory\n");
		exit (88);
	}
	if (pipe (par2chld) < 0 || pipe (chld2par) < 0) {
		perror ("dbcp: Can't pipe");
		exit (89);
	}

	/*
	 * Ignore SIGPIPE, which may occur sometimes when the parent
	 * goes to send a token to an already dead child on last buffer.
	 */
	(void)signal(SIGPIPE, SIG_IGN);

	switch (pid = fork()) {
	case -1:
		perror ("dbcp: Can't fork");
		exit (99);

	case 0:
		/*  Child  */
		close (par2chld[P_WR]);
		close (chld2par[P_RD]);
		wfd = chld2par[P_WR];
		rfd = par2chld[P_RD];
		msgchar = GO;		/* Prime the pump, so to speak */
		goto childstart;

	default:
		/*  Parent  */
		close (par2chld[P_RD]);
		close (chld2par[P_WR]);
		wfd = par2chld[P_WR];
		rfd = chld2par[P_RD];
		break;
	}

	exitval = 0;
	count = 0L;
	while (1) {
		if ((nread = mread (0, buffer, size)) != size) {
			saverrno = errno;
			msgchar = STOP;
		} else
			msgchar = GO;
		if(write (wfd, &msgchar, 1) != 1) {
			perror("dbcp: message send");
			prs("Can't send READ message\n");
		}
		if ((int)nread == (-1)) {
			errno = saverrno;
			perror ("input read");
			prs("read error on input\n");
			break;
		}
		if(nread == 0) {
			if(verbose) prs("EOF on input\n");
			break;
		}
		if(nread != size)
			prs("partial read (nread = %u)\n", nread);
		if (read(rfd, &msgchar, 1) != 1) {
			perror("dbcp: WRITE message error");
			exitval = 69;
			break;
		}
		if (msgchar == STOP) {
			prs("Got STOP WRITE with %u left\n", nread);
			break;
		} else if (msgchar != GO) {
			prs("Got bad WRITE message 0%o\n", msgchar&0377);
			exitval = 19;
			break;
		}
		if (write(1, buffer, nread) != nread) {
			perror("output write");
			msgchar = STOP;
		} else {
			count++;
			msgchar = GO;
		}
		if(verbose>1) prs("wrote %d\n", nread);
		if (nread != size)
			break;
childstart:
		if (write (wfd, &msgchar, 1) != 1) {
			perror("dbcp: message send");
			prs("Can't send WRITE message\n");
			break;
		}
		if (msgchar == STOP) {
			prs ("write error on output\n");
			break;
		}
		if (read(rfd, &msgchar, 1) != 1) {
			perror("dbcp: READ message error");
			exitval = 79;
			break;
		}
		if (msgchar == STOP) {
			if(verbose) prs("Got STOP READ\n");
			break;
		} else if (msgchar != GO) {
			prs("Got bad READ message 0%o\n", msgchar&0377);
			exitval = 39;
			break;
		}
	}

	if(verbose) prs ("%ld records copied\n", count);
	if(pid)
		while (wait(&waitcode) > 0);
	exit(exitval);
}

void
prs (fmt, a, b, c)
char	*fmt, *a, *b, *c;
{
	fprintf(stderr, "dbcp: (%s) ", pid ? "PARENT" : "CHILD");
	fprintf(stderr, fmt, a, b, c);
	fflush(stderr);
}

/*
 *			M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because pipes
 * and network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int
mread(fd, bufp, n)
int	fd;
register char	*bufp;
int	n;
{
	register int	count = 0;
	register int	nread;

	do {
		nread = read(fd, bufp, (unsigned)n-count);
		if(nread < 0)  {
			perror("dbcp: mread");
			return(-1);
		}
		if(nread == 0)
			return((int)count);
		count += (unsigned)nread;
		bufp += nread;
	 } while(count < n);

	return((int)count);
}
