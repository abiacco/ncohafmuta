#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#if defined(SOL_SYS) || defined(LINUX_SYS) || defined(WINDOWS)
#include <string.h>
#else
#include <strings.h>
#endif
*/

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"
#include "../hdrfiles/resolver_clipon.h"

extern char mess[ARR_SIZE+25];
extern fd_set    readmask;         /* bitmap read set                */
extern fd_set    writemask;        /* bitmap write set               */

int resolver_toclient_pipes[2];
int resolver_toserver_pipes[2];
int resolver_clipon_pid=-1;

int init_resolver_clipon(void) {
int ret,i=0;
fd_set fds;
struct timeval timeout;
char namebuffer[256];
char resolver_binary[256];
char readbuf[256];

sprintf(namebuffer,"Resolver clip-on for %s",SYSTEM_NAME);

if (pipe(resolver_toclient_pipes) == -1) {
	write_log(SYSTEMLOG,YESTIME,"Couldn't create resolver clip-on client pipes %s\n",get_error());
	write_log(RESOLVELOG,YESTIME,"TALKER: Couldn't create resolver clip-on client pipes %s\n",get_error());
	cleanup_resolver();
	return 0;
}
if (pipe(resolver_toserver_pipes) == -1) {
	write_log(SYSTEMLOG,YESTIME,"Couldn't create resolver clip-on server pipes %s\n",get_error());
	write_log(RESOLVELOG,YESTIME,"TALKER: Couldn't create resolver clip-on server pipes %s\n",get_error());
	cleanup_resolver();
	return 0;
}

ret = fork();
switch(ret) {
	case -1: /* error */
		write_log(SYSTEMLOG,YESTIME,"Bad resolver clip-on fork: %s\n",get_error());
		write_log(RESOLVELOG,YESTIME,"TALKER: Bad resolver clip-on fork: %s\n",get_error());
		cleanup_resolver();
		return 0;
	case 0: /* child */
		CLOSE(FROM_CLIENT_READ); /* close read to client */
		CLOSE(FROM_CLIENT_WRITE); /* close write to server */
		CLOSE(0);
		dup(FROM_SERVER_READ); /* dupe read to server */
		CLOSE(FROM_SERVER_READ);
		CLOSE(1);
		dup(FROM_SERVER_WRITE); /* dupe write to server */
		CLOSE(FROM_SERVER_WRITE);
                        /* Close all user sockets in this child before	*/
			/* we do anything. otherwise, if we get killed	*/
			/* and restart while users are online, users	*/
			/* that logoff will have their connection hang	*/
			/* right at the very end because they will have	*/
			/* multiple sockets open.			*/
                        for (i=0;i<MAX_USERS;++i) {
                        if (ustr[i].sock != -1) {
                        queue_flush(i);
			SHUTDOWN(ustr[i].sock, 2);
                        while (CLOSE(ustr[i].sock) == -1 && errno == EINTR)
                                ; /* empty while */
                        FD_CLR(ustr[i].sock,&readmask);
                        FD_CLR(ustr[i].sock,&writemask);
                        }
                        }
		/* this method MAY change */
		snprintf(resolver_binary,256,"./%s",RESOLVER_BINARY);
		execlp(resolver_binary, namebuffer, (char *)NULL);
		write_log(SYSTEMLOG,YESTIME,"Failed to exec resolver clip-on binary %s %s\n",resolver_binary,get_error());
		write_log(RESOLVELOG,YESTIME,"TALKER: Failed to exec resolver clip-on binary %s %s\n",resolver_binary,get_error());
		cleanup_resolver();
		return 0;
	default: /* parent */
		resolver_clipon_pid = ret;
		CLOSE(FROM_SERVER_READ);
		CLOSE(FROM_SERVER_WRITE);
		FROM_SERVER_READ = FROM_SERVER_WRITE = -1;
		if (MY_FCNTL(FROM_CLIENT_READ, MY_F_SETFL, NBLOCK_CMD)==SOCKET_ERROR) {
		write_log(RESOLVELOG,YESTIME,"TALKER: Can't set resolver read socket non-blocking: %s\n",get_error());
		cleanup_resolver();
		return 0;
		}
		if (MY_FCNTL(FROM_CLIENT_WRITE, MY_F_SETFL, NBLOCK_CMD)==SOCKET_ERROR) {
		write_log(RESOLVELOG,YESTIME,"TALKER: Can't set resolver write socket non-blocking: %s\n",get_error());
		cleanup_resolver();
		return 0;
		}
	} /* end of switch */


        FD_ZERO(&fds);
        FD_SET(FROM_CLIENT_READ, &fds);
        timeout.tv_sec = 15;
        timeout.tv_usec = 0;
        while (-1 == (ret = select(FD_SETSIZE, &fds, 0, 0, &timeout)))
        {
                if (errno == EINTR || errno == EAGAIN)
                {
                        continue;
                }
		write_log(RESOLVELOG,YESTIME,"TALKER: select() timed out waiting for server: %s\n",get_error());
                kill_resolver_clipon();
                return 0;
	}
	if (FD_ISSET(FROM_CLIENT_READ, &fds)) {
	ret = read(FROM_CLIENT_READ, readbuf, sizeof(readbuf));
	readbuf[ret] = '\0';
	write_log(RESOLVELOG,YESTIME,"TALKER: Read from clip-on: \"%s\"\n",readbuf);
        if (strcmp(readbuf, RESOLVER_SERVER_CONNECT_MSG))
        {
		ret = read(FROM_CLIENT_READ, readbuf, sizeof(readbuf));
		readbuf[ret] = '\0';
		write_log(RESOLVELOG,YESTIME,"TALKER: Read2 from clip-on: \"%s\"\n",readbuf);
	write_log(SYSTEMLOG,YESTIME,"Bad resolver clip-on connect message!\n");
	write_log(RESOLVELOG,YESTIME,"TALKER: Bad resolver clip-on connect message!\n");
                kill_resolver_clipon();
                return 0;
        }
	write_log(SYSTEMLOG,YESTIME,"Resolver clipon running\n");
	write_log(RESOLVELOG,YESTIME,"TALKER: Resolver clipon running\n");
        FD_ZERO(&fds);
	return 1;
	} /* end of if read FD_ISSET */

return 0;
} /* end of init_resolver_clipon */

void kill_resolver_clipon(void) {
int status;

if (resolver_clipon_pid != -1) {
        CLOSE(FROM_CLIENT_READ);
        CLOSE(FROM_CLIENT_WRITE);
        FROM_CLIENT_READ = FROM_CLIENT_WRITE = -1;
        FROM_SERVER_READ = FROM_SERVER_WRITE = -1;
	write_log(SYSTEMLOG,YESTIME,"Killing resolver clip-on\n");
	write_log(RESOLVELOG,YESTIME,"TALKER: Killing resolver clip-on\n");
        kill(resolver_clipon_pid, SIGTERM);
        waitpid(-1, &status, WNOHANG);
	resolver_clipon_pid = -1;
}

}

void send_resolver_request(int user, char *site, char *bad) {
int bytes;
char sendbuf[1000];

if (user==-1)
        snprintf(sendbuf,999,"%s",RESOLVER_CLIENT_SEND_SUSPEND);
else if (user==-2)
        snprintf(sendbuf,999,"%s",RESOLVER_CLIENT_SEND_RESUME);
else
	snprintf(sendbuf,999,"%s %d %s %s",RESOLVER_CLIENT_SEND_REQUEST,user,site,bad);
write_log(RESOLVELOG,YESTIME,"TALKER: send_resolver_request: Send to server \"%s\"\n",sendbuf);
bytes = write(FROM_CLIENT_WRITE,sendbuf,strlen(sendbuf));
if (bytes < strlen(sendbuf)) {
	sprintf(mess,"Client failed to write resolver request %d:%d",bytes,(int)strlen(sendbuf));
	write_str(user,mess);
/* Restart and try to write again */
kill_resolver_clipon();
sleep(2);
init_resolver_clipon();
bytes = write(FROM_CLIENT_WRITE,sendbuf,strlen(sendbuf));

	return;
} /* end of bad write */
else {
/*	write_str(user,"Wrote resolver request"); */
} /* end of else good write */

}

void read_resolver_reply(void) {
int i;
int bytes;
int userfind;
char site[21];
char host[64];
char readbuf[256];

bytes = read(FROM_CLIENT_READ, readbuf, sizeof(readbuf));
readbuf[bytes] = '\0';
if (bytes <= 0) {
/* resolver probably died, let's try to restart */
kill_resolver_clipon();
sleep(2);
init_resolver_clipon();
return;
}
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"TALKER: read_resolver_reply: \nRead from server: %d bytes \"%s\"\n",bytes,readbuf);
#endif

    while (strlen(readbuf)) {

/* dont write to log if just checkpoint */
while (!strncmp(readbuf,RESOLVER_SERVER_SEND_CHECKPOINT,strlen(RESOLVER_SERVER_SEND_CHECKPOINT))) {
	remove_first(readbuf);
/*
	write_log(RESOLVELOG,YESTIME,"TALKER: read_resolver_reply: stripped: now %s\n",readbuf);
*/
}

/* just a checkpoint */
if (!strlen(readbuf)) return;

sscanf(readbuf,"%d",&userfind);
remove_first(readbuf);
sscanf(readbuf,"%s ",site);
remove_first(readbuf);
sscanf(readbuf,"%s ",host);
remove_first(readbuf);
if (!strcmp(host,"*")) strcpy(host,SYS_LOOK_FAILED);

	/* find user that should have this hostname */
	for (i=0;i<MAX_USERS;++i) {
	  if (userfind==i && ustr[i].sock!=-1 && !strcmp(ustr[i].site,site)) {
		strncpy(ustr[i].net_name,host,64);
		if (resolve_names==2) {
			del_from_resolver_cache(i);
			add_to_resolver_cache(i);
		}
		write_log(RESOLVELOG,YESTIME,"TALKER: Copied %s to user %d %s\n",host,i,ustr[i].name);
/*
		sprintf(readbuf,"Your resolve result came back as %s",ustr[i].net_name);
		write_str(i,readbuf);
*/
             /*-----------------------------------*/
             /* Check for totally restricted host */
             /*-----------------------------------*/
             if (check_restriction(i, ANY, THEIR_HOST) == 1)
               {
                write_log(BANLOG,YESTIME,"MAIN: Connection attempt, RESTRICTed host %s:%s:sck#%d:slt#%d\n",
		ustr[i].site,ustr[i].net_name,ustr[i].sock,i);
                user_quit(i,1);
               }

		break;
	  } /* if match */
	} /* user for loop */
    } /* readbuf while loop */
}


void cleanup_resolver(void) {

		CLOSE(FROM_CLIENT_READ);
		CLOSE(FROM_CLIENT_WRITE);
		CLOSE(FROM_SERVER_READ);
		CLOSE(FROM_SERVER_WRITE);
		FROM_CLIENT_READ = FROM_CLIENT_WRITE = -1;
		FROM_SERVER_READ = FROM_SERVER_WRITE = -1;
		resolver_clipon_pid = -1;
}
