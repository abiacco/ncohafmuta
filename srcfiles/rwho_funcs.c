#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*--------------------------------------------------------*/
/* Talker-related include files                           */ 
/*--------------------------------------------------------*/
#include "../hdrfiles/osdefs.h"
/*
#include "../hdrfiles/authuser.h"
#include "../hdrfiles/text.h"
*/
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern char mess[ARR_SIZE+25];
extern fd_set readmask;
extern fd_set writemask;
extern int listen_sock[4];

/*------------------------------------------------*/
/* Check the .who input string for a rwho request */
/*------------------------------------------------*/
int check_rwho(int user, char *inpstr)
{
int portnum;
int type=1;  /* 1 - Iforms or Ncohafmuta  0 - Nuts or other */
char addy[64];
char info[100];
char filename[FILE_NAME_LEN];
FILE *fp;

if (inpstr[0]=='@') {
  midcpy(inpstr,inpstr,1,ARR_SIZE);
  if (!strlen(inpstr)) {
   strcpy(filename,get_temp_file());
   if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     return 1;
     }
    fputs("Who listings are available from these talkers:\n",fp);
    fputs("^Ncohafmuta Talkers:^\n",fp);
    fputs("  after   - After Dark              aot      - Aotearoa\n",fp);
    fputs("  blue    - The Blue Note           dark     - Dark Tower\n",fp);
    fputs("  emerald - The Emerald Isle        flirt    - Flirt Town\n",fp);
    fputs("  globe   - The Globe               grease   - Grease\n",fp);
    fputs("  invas   - Invasions               jour     - Journeys\n",fp);
    fputs("  jungle  - The Jungle              mega     - Mega MoviePlex\n",fp);
    fputs("  merlin  - Merlin\'s Hideaway       north    - North Woods\n",fp);
    fputs("  path    - Path Of The Unicorn     pw       - PinWHeeLs\n",fp);
    fputs("  scu     - StateOfConfusionUni     sommer   - Sommerland\n",fp);
    fputs("  spell   - Spellbinder             wjjm     - The WJJM\n",fp);
    fputs("  xonia   - Xonia\n",fp);
    fputs("^Iforms or other type talkers:^\n",fp);
    fputs("  beach   - Davenport Beach         breck    - Brecktown\n",fp);
    fputs("  cactus  - Cactus                  chakrams - Chakrams and Scrolls\n",fp);
    fputs("  light   - LightHouse              ocean    - Oceanhome\n",fp);
    fputs("  quest   - Vision Quest            trek     - Enterprise\n",fp);
    fputs("  village - The Village\n",fp);
    fclose(fp);
    cat(filename,user,0);
    return 1;
   }
  if (!strcmp(inpstr,"beach")) {
    strcpy(info,"Davenport Beach (nowaksg.chem.nd.edu 3371)");
    strcpy(addy,"129.74.80.116");
    portnum=3372;
    }
  else if (!strcmp(inpstr,"quest")) {
    strcpy(info,"Vision Quest (talker.com 6000)");
    strcpy(addy,"66.101.11.58");
    portnum=6001;
    }
  else if (!strcmp(inpstr,"spell")) {
    strcpy(info,"Spellbinder (talker.com 9999)");
    strcpy(addy,"66.101.11.58");
    portnum=9990;
    }
  else if (!strcmp(inpstr,"trek")) {
    strcpy(info,"Enterprise (linex1.linex.com 5000)");
    strcpy(addy,"199.4.98.11");
    portnum=5001;
    }
  else if (!strcmp(inpstr,"cactus")) {
    strcpy(info,"Cactus (cactus.ecpi.com 5000)");
    strcpy(addy,"208.21.246.2");
    portnum=5001;
    }
  else if (!strcmp(inpstr,"blue")) {
    strcpy(info,"The Blue Note (unknown 4000)");
    strcpy(addy,"24.158.1.114");
    portnum=3998;
    }
  else if (!strcmp(inpstr,"breck")) {
    strcpy(info,"BreckTown (talker.com 5000)");
    strcpy(addy,"66.101.11.58");
    portnum=5001;
    }
  else if (!strcmp(inpstr,"light")) {
    strcpy(info,"LightHouse (mookie.com 6969)");
    strcpy(addy,"199.217.223.69");
    portnum=6970;
    type=0;
    }
  else if (!strcmp(inpstr,"globe")) {
    strcpy(info,"The Globe (talker.com 9050)");
    strcpy(addy,"66.101.11.58");
    portnum=9051;
    }
  else if (!strcmp(inpstr,"grease")) {
    strcpy(info,"Grease (talker.com 9500)");
    strcpy(addy,"66.101.11.58");
    portnum=9510;
    }
  else if (!strcmp(inpstr,"jour")) {
    strcpy(info,"Journeys (talker.com 9000)");
    strcpy(addy,"66.101.11.58");
    portnum=8999;
    }
  else if (!strcmp(inpstr,"jungle")) {
    strcpy(info,"The Jungle (talker.com 3050)");
    strcpy(addy,"66.101.11.58");
    portnum=3051;
    }
  else if (!strcmp(inpstr,"pw")) {
    strcpy(info,"PinWHeeLs (unknown 5000)");
    strcpy(addy,"24.158.1.114");
    portnum=5001;
    }
  else if (!strcmp(inpstr,"scu")) {
    strcpy(info,"State of Conf. Uni. (talker.com 3500)");
    strcpy(addy,"66.101.11.58");
    portnum=3501;
    }
  else if (!strcmp(inpstr,"sommer")) {
    strcpy(info,"Sommerland (talker.com 5555)");
    strcpy(addy,"66.101.11.58");
    portnum=5560;
    }
  else if (!strcmp(inpstr,"village")) {
    strcpy(info,"The Village (village.pb.net 5000)");
    strcpy(addy,"204.117.211.9");
    portnum=5001;
    }
  else if (!strcmp(inpstr,"wjjm")) {
    strcpy(info,"The WJJM (unknown 6666)");
    strcpy(addy,"24.158.1.114");
    portnum=6668;
    }
  else if (!strcmp(inpstr,"after")) {
    strcpy(info,"After Dark (barney.gonzaga.edu 8000)");
    strcpy(addy,"147.222.2.1");
    portnum=8001;
    }
  else if (!strcmp(inpstr,"aot")) {
    strcpy(info,"Aotearoa (lunasoleil.com 4444)");
    strcpy(addy,"24.158.1.114");
    portnum=4445;
    }
  else if (!strcmp(inpstr,"chakrams")) {
    strcpy(info,"Chakrams and Scrolls (talker.com 8000)");
    strcpy(addy,"66.101.11.58");
    portnum=8001;
    }
  else if (!strcmp(inpstr,"dark")) {
    strcpy(info,"Dark Tower (talker.com 5432)");
    strcpy(addy,"66.101.11.58");
    portnum=5433;
    }
  else if (!strcmp(inpstr,"flirt")) {
    strcpy(info,"Flirt Town (talker.com 3200)");
    strcpy(addy,"66.101.11.58");
    portnum=3201;
    }
  else if (!strcmp(inpstr,"invas")) {
    strcpy(info,"Invasions (talker.com 9786)");
    strcpy(addy,"66.101.11.58");
    portnum=9788;
    }
  else if (!strcmp(inpstr,"mega")) {
    strcpy(info,"Mega MoviePlex (talker.com 7000)");
    strcpy(addy,"66.101.11.58");
    portnum=7002;
    }
  else if (!strcmp(inpstr,"merlin")) {
    strcpy(info,"Merlin\'s Hideaway (talker.com 9876)");
    strcpy(addy,"66.101.11.58");
    portnum=9873;
    }
  else if (!strcmp(inpstr,"north")) {
    strcpy(info,"North Woods (unknown 6000)");
    strcpy(addy,"24.158.1.114");
    portnum=6001;
    }
  else if (!strcmp(inpstr,"ocean")) {
    strcpy(info,"Oceanhome (talker.com 4000)");
    strcpy(addy,"66.101.11.58");
    portnum=4005;
    }
  else if (!strcmp(inpstr,"path")) {
    strcpy(info,"The Path Of The Unicorn (talker.com 4444)");
    strcpy(addy,"66.101.11.58");
    portnum=4446;
    }
  else if (!strcmp(inpstr,"emerald")) {
    strcpy(info,"The Emerald Isle (unknown 8000)");
    strcpy(addy,"24.158.1.114");
    portnum=8002;
    }
  else if (!strcmp(inpstr,"xonia")) {
    strcpy(info,"Xonia (unknown 3000)");
    strcpy(addy,"24.158.1.114");
    portnum=3001;
    }
  else if (!strcmp(inpstr,"test")) {
    strcpy(info,"Test (www.atomic.org 5001)");
    strcpy(addy,"209.42.10.17");
    portnum=5001;
    }
  else {
   strcpy(filename,get_temp_file());
   if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     return 1;
     }
    fputs("Who listings are available from these talkers:\n",fp);
    fputs("^Ncohafmuta Talkers:^\n",fp);
    fputs("  after   - After Dark              aot      - Aotearoa\n",fp);
    fputs("  blue    - The Blue Note           dark     - Dark Tower\n",fp);
    fputs("  emerald - The Emerald Isle        flirt    - Flirt Town\n",fp);
    fputs("  globe   - The Globe               grease   - Grease\n",fp);
    fputs("  invas   - Invasions               jour     - Journeys\n",fp);
    fputs("  jungle  - The Jungle              mega     - Mega MoviePlex\n",fp);
    fputs("  merlin  - Merlin\'s Hideaway       north    - North Woods\n",fp);
    fputs("  path    - Path Of The Unicorn     pw       - PinWHeeLs\n",fp);
    fputs("  scu     - StateOfConfusionUni     sommer   - Sommerland\n",fp);
    fputs("  spell   - Spellbinder             wjjm     - The WJJM\n",fp);
    fputs("  xonia   - Xonia\n",fp);
    fputs("^Iforms or other type talkers:^\n",fp);
    fputs("  beach   - Davenport Beach         breck    - Brecktown\n",fp);
    fputs("  cactus  - Cactus                  chakrams - Chakrams and Scrolls\n",fp);
    fputs("  light   - LightHouse              ocean    - Oceanhome\n",fp);
    fputs("  quest   - Vision Quest            trek     - Enterprise\n",fp);
    fputs("  village - The Village\n",fp);
    fclose(fp);
    cat(filename,user,0);
    return 1;
   }

if (ustr[user].rwho > 1) {
  write_str(user,"You already have a remote who connection opened. Wait for it to finish or timeout first.");
  return 1;
  }

sprintf(mess,"^Getting listing from %s..^",info);
write_str(user,mess);
write_str(user,"^This MAY take time. You may play through the wait^");
queue_flush(user);

/* Here is where we break the remote who connection off, so we dont hang the talker */
/* if the connection fails. We do this with fork() to create a child process       */
/* Things inherited by the child: process credentials, environment, memory, stack, */
/* open file descriptors, close-on-exec flags, signal handling, nice value, sched- */
/* uler class, process group ID, session ID, cwd, root dir, umask, reources limits */
/* , controlling terminal.                                                         */
/* Things NOT inherited by the child: process ID, different parent process ID, own */
/* copy of file descriptors and dir. streams, process, data, text, memory locks,   */
/* process times, pending signals init'd to the empty set, timers created by       */
/* timer_create, async input/output operations, resource utilizations are set to 0 */


			get_rwho(user,addy,portnum,info,type);

   return 1;
 } /* end of if @ */
else return 0;
}

/*------------------------*/
/* Go get the who listing */
/*------------------------*/
void get_rwho(int user, char *host, int port, char *info, int type)
{
	struct	sockaddr_in	raddr;
	struct	hostent		*hp;
	int			i=0;
	int			fd;
	int			size=sizeof(struct sockaddr_in);
	char			buffer[FILE_NAME_LEN];
	char			*p;

/* Zero out memory for address */
memset((char *)&raddr, 0, size);

	p = host;
	while(*p != '\0' && (*p == '.' || isdigit((int)*p)))
		p++;

	/* not all digits or dots */
	if(*p != '\0') {
		if((hp = gethostbyname(host)) == (struct hostent *)0) {
			sprintf(buffer,"^HRUnknown hostname %s, can't get rwho list^",host);
			write_str(user,buffer);
			write_log(ERRLOG,YESTIME,"RWHO: Unknown hostname %s, can't get rwho list\n",host);
			return;
		}

		(void)bcopy(hp->h_addr,(char *)&raddr.sin_addr,hp->h_length);
	}
	else {
		unsigned long	f;

		if((f = inet_addr(host)) == -1L) {
			sprintf(buffer,"^HRUnknown ip address %s, can't get rwho list^",host);
			write_str(user,buffer);
			write_log(ERRLOG,YESTIME,"RWHO: Unknown ip address %s, can't get rwho list\n",host);
			return;
			}
		(void)bcopy((char *)&f,(char *)&raddr.sin_addr,sizeof(f));
	}

	raddr.sin_port = htons((unsigned short)port);
	raddr.sin_family = AF_INET;

	if ((fd = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET) {
		write_str(user,"^HRRwho socket cannot be made!^");

		write_log(ERRLOG,YESTIME,"RWHO: Socket creation failed for %s %d! %s\n",host,port,get_error());
#if !defined(WINDOWS)
		reset_alarm();
#endif
		return;
	}

	if (MY_FCNTL(fd,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
		write_log(ERRLOG,YESTIME,"BLOCK: ERROR setting rwho socket to non-blocking %s\n",get_error());
		SHUTDOWN(fd, 2);
		CLOSE(fd);
		return;
	}

/*
        sprintf(buffer,"^Getting listing from %s..^",info);
	write_str(user,buffer);
	write_str(user,"^This MAY take time. You may play through the wait^");
        buffer[0]=0;
*/

        if ( (i = find_free_slot('5') ) == -1 ) { 
                write_log(ERRLOG,YESTIME,"RWHO: Can't find free slot for connection!\n");
		write_str(user,"^HRCan't find free slot for connection!^\n");
		SHUTDOWN(fd, 2);
		CLOSE(fd);
                return;
        }

miscconn[i].sock=fd;
miscconn[i].user=user;
miscconn[i].type=1;
miscconn[i].port=port;
miscconn[i].time=time(0);
ustr[miscconn[i].user].rwho=2;

              if (log_misc_connect(i,raddr.sin_addr.s_addr,3) == -1) {
		 write_log(ERRLOG,YESTIME,"RWHO: Can't write connection to log!\n");
 		 write_str(user,"^HRCan't write connection to log!^\n");
                 free_sock(i,'5');
                 return;
                }

/* we may not even get any error except for EINPROGRESS since we set the socket non-blocking */
/* get_input will be the function that tells us when this stuff happens */
	if ((connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) == SOCKET_ERROR) &&
		(errno != EINPROGRESS)) {
	   if (errno == ECONNREFUSED)
		write_str(user,"^HRConnection refused to talker!  Talker may be down. Try again in a few minutes.^");
	   else if (errno == ETIMEDOUT)
		write_str(user,"^HRConnection timed out to talker!  Talker or internet route may be down. Try again in a few minutes.^");
	   else if (errno == ENETUNREACH)
		write_str(user,"^HRNetwork remote talker is on is unreachable! Try later.^");
	   else
		write_str(user,"^HRUnknown problem. Try later.^");

		/* Uncomment if you want connection errors logged
		write_log(ERRLOG,YESTIME,"RWHO: Connection failed to %s %d %s\n",host,port,get_error());
		*/

		free_sock(i,'5');
	}

		return;


/*
	while((red = S_READ(fd ,&rbuf, 1)) > 0) {
		flag=0;
		if ((unsigned char)rbuf==255) continue;


                switch(rbuf) {
		case '\n': buffer[point]   = 0;
			   point = 0;
			   break;
		case '\r': flag=1;
			   break;
		case '\t': buffer[point] = ' ';
			   point++;
			   break;
		default: buffer[point] = rbuf;
			 point++;
			 break;
		}
		if (flag) continue;

	if (!done) {
		if (!point) {
		 if (type==0) {
			if (linenum < 2) { linenum++; continue; }
		  }
		 else if (type==1) {
			if (linenum < 4) { linenum++; continue; }
		  }

		  if (strstr(buffer,"Total of")) {
			write_str(user,buffer);
			buffer[0]=0;
			done = 1;
		    }
                  else {
			write_str(user,buffer);
			buffer[0]=0;
		    }
		  linenum++;
                  continue;
		 }
		else continue;
	  }

	}
*/

}

