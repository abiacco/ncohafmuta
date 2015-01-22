/*
 *	announce - sits listening on a port, and whenever anyone connects
 *		   announces a message and disconnects them
 *
 *	Usage:	announce [port] < message_file
 *
 *	Author:	Lawrence Brown <lpb@cs.adfa.oz.au> 	Aug 90
 *	Cleaned up and optimized 
 *	by Cygnus <ncohafmuta@asteroid-b612.org> Feb 99
 *
 *	Bits of code are adapted from the Berkeley telnetd sources
 */

#define PORT	5000	/* port to sit on */
#define SHOW_ADDRESS 1	/* show ip address in connection line? */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/signal.h>
#include <ctype.h>

char	*Name;			/* name of this program for error messages */
char	msg[2048];
     
int main(int argc, char **argv)
{
  int s, ns, foo, opt;
  unsigned long addr;
  struct sockaddr_in sin;
  char host[25], *inet_ntoa();
  char tmp[80];
  time_t ct;
  
  Name = argv[0];		/* save name of program for error messages  */
  sin.sin_port = htons((u_short)PORT);  /* Assume PORT */
  argc--, argv++;
  if (argc > 0) {		/*   unless specified on command-line       */
	sin.sin_port = atoi(*argv);
    sin.sin_port = htons((u_short)sin.sin_port);
  }

sin.sin_family      = AF_INET;   /* setup address struct */
sin.sin_addr.s_addr = INADDR_ANY; 

  strcpy(msg, "");
  strcpy(tmp, "");
  while (1) {
    if ((gets(tmp)) == NULL) break;
    strcat(tmp, "\r\n");
    strcat(msg, tmp);
  }
  msg[2048] = '\0';
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  s = socket(AF_INET, SOCK_STREAM, 0); /* get socket */
  if (s < 0) {
    perror("announce: socket");;
    exit(1);
  }
  opt = 1;
  /* set it resusable */
  setsockopt(s,SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt));
  /* bind port to it */
  if (bind(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
    perror("bind");
    exit(1);
  }
  if ((foo = fork()) != 0) {
    fprintf(stderr, "announce: pid %d running on port %d\n", foo,
	    ntohs((u_short)sin.sin_port));
    exit(0);
  }

  if (listen(s, 5) < 0) {		/* start listening on port */
    perror("announce: listen");
    exit(1);
  }
  foo = sizeof sin;
  for(;;) {	/* loop forever, accepting requests & printing msg */
    ns = accept(s, (struct sockaddr *)&sin, &foo);
    if (ns < 0) {
      perror("announce: accept");
      exit(1);
    }
	if (SHOW_ADDRESS==1) {
	 addr=ntohl(sin.sin_addr.s_addr); 
	 host[0]=0;
	 sprintf(host,"%ld.%ld.%ld.%ld", (addr >> 24) & 0xff, (addr >> 16) & 0xff,
         (addr >> 8) & 0xff, addr & 0xff);
	}
	else {
	 strcpy(host,"Unknown"); 
	}

    time(&ct);
    fprintf(stderr, "CONNECTION made from %s at %s",
	    host, ctime(&ct));
    write(ns, msg, strlen(msg));
    sleep(5);
    close(ns);
    sleep(1);
  }
}



