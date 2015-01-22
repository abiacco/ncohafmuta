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
#include "../hdrfiles/resolver_clipon.h"

extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */
extern char t_inpstr[ARR_SIZE];	/* functions use t_mess as a buffer    */
extern fd_set    readmask;         /* bitmap read set                */
extern fd_set    writemask;        /* bitmap write set               */
extern int PORT;                  /* main login port for incoming   */
extern int listen_sock[4];        /* 32 bit listening sockets       */
extern int restarting;
extern int sys_access;  /* is the system open for user connections?      */
extern int wiz_access;  /* is the system open for wizard connections?    */
extern int who_access;  /* is the system open for external who listings? */
extern int www_access;  /* is the system open for external web requests? */
extern int resolver_toclient_pipes[2];

/*-----------------------------------------------------------------------*/
/*                       Socket Functions Follow                         */
/*-----------------------------------------------------------------------*/

/* Read user input from a socket */
int get_input(int user, char port, int mode)
{
int len=0;
int complete_line;
int gotcr=0;
int sock=-1;
int buff_size;
char *dest;
char astring[ARR_SIZE];
unsigned char inpchar[2];   /* read() data from socket into this */
int char_buffer_size = 0;
char char_buffer[MAX_CHAR_BUFF];

char_buffer[0]=0;
t_inpstr[0]=0;

                /* if (ustr[user].area== -1 && !ustr[user].logging_in) */
                /* continue; */
if (mode==0) sock=ustr[user].sock;
else if (mode==1) sock=wwwport[user].sock;
else if (mode==2) sock=miscconn[user].sock;

                if (sock == -1) return -1;

                /* see if any data on socket else continue */
                if (!FD_ISSET(sock,&readmask)) return -2;

               /*--------------------------------------------*/
               /* reset the user input space                 */
               /*--------------------------------------------*/
               
                 astring[0]  = 0;
                 inpchar[0]  = 0;
                 dest        = astring;

                 /*-----------------------------------------*/
                 /* see if the user is gone or has input    */
                 /*-----------------------------------------*/

                 len = S_READ(sock, inpchar, 1);

		if (!len) {
                 if (mode==0) user_quit(user,1);
                 else if (mode==1) free_sock(user,port);
 	         else if (mode==2) {
			if (miscconn[user].type==1)
				write_str(miscconn[user].user,"^Remote who finished.^");
			else if (miscconn[user].type==2) {
				FCLOSE(miscconn[user].fd);
				requeue_smtp(user);
			}
			free_sock(user,port);
		 }
                 return -7;
		} /* end of if */
		else if (len==SOCKET_ERROR) {
#if defined(QFLUSH_DEBUG)
		 if (mode==0)
		  write_log(DEBUGLOG,YESTIME,"Bad read() from user \"%s\" ret %d! %s\n",ustr[user].name,len,get_error());
		 else if (mode==1)
		  write_log(DEBUGLOG,YESTIME,"Bad www read() from socket %d ret %d! %s\n",wwwport[user].sock,len,get_error());
		 else if (mode==2)
		  write_log(DEBUGLOG,YESTIME,"Bad misconn read() from socket %d ret %d! %s\n",miscconn[user].sock,len,get_error());
#endif
                 if (errno!=EAGAIN && errno!=EINTR) {
			if (mode==0) user_quit(user,0);
			else if (mode==1) free_sock(user,port);
 	                else if (mode==2) {
				if (miscconn[user].type==1)
					write_str(miscconn[user].user,"^HRRemote who finished. Talker down or unreachable.^");
				else if (miscconn[user].type==2) {
					FCLOSE(miscconn[user].fd);
					requeue_smtp(user);
				}
			free_sock(user,port);
			}
			return -7;
		 }
                 	return -3;
                } /* end of else if */


                 /*-------------------------------------------*/
                 /* if there is input pending, read it        */
                 /*  (stopping on <cr>, <EOS>, or <EOF>)      */
                 /*-------------------------------------------*/
                 complete_line = 0;

if (mode==0) {
                 while ((inpchar[0] != 0)  &&
                        (len != EOF)       &&
                        (len != -1)        &&
                        (complete_line ==0 )  )
                   {
                    /*----------------------------------------------*/
                    /* process input                                */
                    /*----------------------------------------------*/
                    switch (inpchar[0])
                     {
                       case IAC:     do_telnet_commands(user);
                                     break;

                       case '\001':  user_quit(user,1); break;    /* soh  */
                       case '\002':  user_quit(user,1); break;    /* stx */
                       case '\003':  user_quit(user,1); break;    /* etx */
                       case '\004':  user_quit(user,1); break;
                       case '\005':  user_quit(user,1); break;    /* enq  */
                       case '\006':  user_quit(user,1); break;    /* ack */

                       case 127:                             /* delete */
                       case '\010':  ustr[user].char_buffer_size--;
                                     if (ustr[user].char_buffer_size < 0)
                                       {
                                        ustr[user].char_buffer_size = 0;
                                       }
                                      else
                                       {
                                        write_str_nr(user, " \b");
                                       }
                                     break;


                       case '\013':  user_quit(user,1); break;    /* enq */
                       case '\014':  user_quit(user,1); break;    /* enq */

                       case '\016':  user_quit(user,1); break;    /* enq */
                       case '\017':  user_quit(user,1); break;    /* enq */

                       case '\020':  user_quit(user,1); break;    /* dle */
                       case '\021':  user_quit(user,1); break;    /* dc1 */
                       case '\022':  user_quit(user,1); break;    /* dc2 */
                       case '\023':  user_quit(user,1); break;    /* dc3 */
                       case '\024':  user_quit(user,1); break;    /* dc4 */
                       case '\025':  user_quit(user,1); break;    /* nak */
                       case '\026':  user_quit(user,1); break;    /* syn */
                       case '\027':  user_quit(user,1); break;    /* etb */

                       case '\030':  user_quit(user,1); break;    /* can */
                       case '\031':  user_quit(user,1); break;    /* em  */
                       case '\032':  user_quit(user,1); break;    /* sub */
                       case '\033':  ; break;                   /* esc */
                       case '\034':  user_quit(user,1); break;    /* fs  */
                       case '\035':  user_quit(user,1); break;    /* gs  */
                       case '\036':  user_quit(user,1); break;    /* rs  */
                       case '\037':  user_quit(user,1); break;    /* us  */

                       default:
                           ustr[user].char_buffer[ustr[user].char_buffer_size++] = inpchar[0];
                           break;
                     } /* end of switch */

	/* We need this here when we get the IAC back with our GA or EOR */
	/* answer. This writes out the GA or EOR after the login prompt */
	if (ustr[user].logging_in && ustr[user].promptseq==2) {
		ustr[user].promptseq=1;
		telnet_write_eor(user);
		}

		    if (inpchar[0] == '\015') {
			/* Got a CR - Carriage return..possibly part of a CRLF combo	*/
			/* if it is, the next check will take care of it		*/
			/* if not, we set this variable so we can tell if its just a CR	*/
			gotcr=1;
		    }

                    if (inpchar[0] == '\012')
                        {
			/* Got a LF - Line Feed..possibly part of a previous CRLF combo	*/
			/* if it is or is just a LF ending, we take it the same way	*/
                         complete_line = 1;
			 if (gotcr) gotcr=0;
                         ustr[user].char_buffer[ustr[user].char_buffer_size++] = 0;
                        }
                      else
                        {
                         if (ustr[user].char_buffer_size > (MAX_CHAR_BUFF-4) )
                           {
                  ustr[user].char_buffer[ustr[user].char_buffer_size++] = '\n';
                  ustr[user].char_buffer[ustr[user].char_buffer_size++] = 0;
                            complete_line = 1;
                           }
                        }

               inpchar[0]=0;

                    if (complete_line == 0)
                      {
                       len = S_READ(ustr[user].sock, inpchar, 1);
			if (len==-1 && gotcr==1) {
			/* this is just a CR line termination */
				complete_line = 1;
				gotcr=0;
				ustr[user].char_buffer[ustr[user].char_buffer_size++] = 0;
			}
                      }
                   } /* end of while */

                /*--------------------------------------*/
                /* terminate the line                   */
                /*--------------------------------------*/

                ustr[user].char_buffer[ustr[user].char_buffer_size] = 0;

                /*------------------------------------------------*/
                /* check for complete line (terminated by \n)     */
                /*------------------------------------------------*/

                if (!complete_line)
                 {
    /*-----------------------------------------------------*/
    /* need to support char mode, no local echo, some time */
    /*-----------------------------------------------------*/
    /* write_str_nr(user,ustr[user].char_buffer[ustr[user].char_buffer_size]); */
    /*-----------------------------------------------------*/
                   return -4;
                  }

                /*--------------------------------------------*/
                /* copy the user buffer to the input string   */
                /*--------------------------------------------*/

                strcpy(astring, ustr[user].char_buffer);
                buff_size = strlen(astring);
                ustr[user].char_buffer_size = 0;

                if ((astring[0] == '\012') && (ustr[user].logging_in)
		    && (ustr[user].logging_in < 11)) return -5;

                /*----------------------------------------------------*/
                /* some nice users were doing some things that would  */
                /* intentionally kill the system.  This should trap   */
                /* that and report such incidents.                    */
                /*----------------------------------------------------*/
                
                if (buff_size > 8000)
                  {
                    write_log(WARNLOG,YESTIME,"HACK flood from site %21.21s possibly as %s\n",
                                  ustr[user].site,
                                  ustr[user].say_name);
                    sprintf(mess,"%s HACK flood from site %21.21s possibly as %s\n",
                                  STAFF_PREFIX,ustr[user].site,
                                  strip_color(ustr[user].say_name));
                    writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0); 
                    
                    if (ustr[user].logging_in)
                      {
                        write_str(user,"----------------------------------------------------------------");
                        write_str(user,"Notice:  You are attempting to use this computer system in a way");
                        write_str(user,"         which is considered a crime under United States federal");
                        write_str(user,"         access laws.  All attempts illegally accessing this site are ");
                        write_str(user,"         logged.  Repeat violators of this offense will be ");
                        write_str(user,"         prosecuted to the fullest extent of the law.");
                        write_str(user,"----------------------------------------------------------------");
                        
                        /*-----------------------------------------*/
                        /* during logins, auto restrict the site   */
                        /*-----------------------------------------*/
                    
                        auto_restrict(user);    

                        user_quit(user,1);
                        return -6;
                       }
                      else
                       {
                        if (ustr[user].locked == 0)
                          {
                           write_str(user,"Notice: Buffer data has been lost. ");
                           write_str(user,"        Further lose of data will result in connection termination.");
                           ustr[user].locked = 1;
                           return -3;
                          }
                         else
                          {
                           write_str(user,"Notice: Connection terminated due to loss of data.\n");
                           user_quit(user,1);
                           return -6;
                          }

                       }

                  } /* end of if buff size */

} /* end of if mode USER */
else if (mode==1 || mode==2) {
                 while ((inpchar[0] != 0)  &&
                        (len != EOF)       &&
                        (len != -1)        &&
                        (complete_line ==0 )  )
                   {
                    /*----------------------------------------------*/
                    /* process input                                */
                    /*----------------------------------------------*/
                    switch (inpchar[0])
                     {
                       case IAC:     break;

                       case '\001':  free_sock(user,port); break;
                       case '\002':  free_sock(user,port); break;
                       case '\003':  free_sock(user,port); break;
                       case '\004':  free_sock(user,port); break;
                       case '\005':  free_sock(user,port); break;
                       case '\006':  free_sock(user,port); break;

                       case 127:                             /* delete */
                       case '\010':  char_buffer_size--;
                                     if (char_buffer_size < 0 )
                                       {
                                        char_buffer_size = 0;
                                       }
                                     break;


                       case '\013':  free_sock(user,port); break;
                       case '\014':  free_sock(user,port); break;

                       case '\016':  free_sock(user,port); break;
                       case '\017':  free_sock(user,port); break;

                       case '\020':  free_sock(user,port); break;
                       case '\021':  free_sock(user,port); break;
                       case '\022':  free_sock(user,port); break;
                       case '\023':  free_sock(user,port); break;
                       case '\024':  free_sock(user,port); break;
                       case '\025':  free_sock(user,port); break;
                       case '\026':  free_sock(user,port); break;
                       case '\027':  free_sock(user,port); break;

                       case '\030':  free_sock(user,port); break;
                       case '\031':  free_sock(user,port); break;
                       case '\032':  free_sock(user,port); break;
                       case '\033':  ; break;               
                       case '\034':  free_sock(user,port); break;
                       case '\035':  free_sock(user,port); break;
                       case '\036':  free_sock(user,port); break;
                       case '\037':  free_sock(user,port); break;

                       default:
                           char_buffer[char_buffer_size++] = inpchar[0];
                           break;
                     } /* end of switch */

if (mode==1) {
                    if (inpchar[0] == '\012' || inpchar[0] == '\015')
                        {
                         complete_line = 1;
                         char_buffer[char_buffer_size++] = 0;
                        }
                      else
                        {
                         if (char_buffer_size > (MAX_CHAR_BUFF-4) )
                           {
                  char_buffer[char_buffer_size++] = '\n';
                  char_buffer[char_buffer_size++] = 0;
                            complete_line = 1;
                           }
                        }

               inpchar[0]=0;

                    if (complete_line == 0)
                      {
                       len = S_READ(sock, inpchar, 1);
		      }
}
else if (mode==2) {
/* 012 = LF = \n */
/* 015 = CR = \r */

                    if (inpchar[0] == '\012' || inpchar[0] == '\015')
                        {
#if defined(MISC_DEBUG)
 write_log(DEBUGLOG,YESTIME,"Got end of line\n");
 if (inpchar[0] == '\012') write_log(DEBUGLOG,YESTIME,"endline is an n\n");
 else if (inpchar[0] == '\015') write_log(DEBUGLOG,YESTIME,"endline is an r\n");
 else write_log(DEBUGLOG,YESTIME,"endline is %c\n",inpchar[0]);
#endif
                         complete_line = 1;
                         char_buffer[char_buffer_size++] = 0;
                        }
                      else
                        {
                         if (char_buffer_size > (MAX_CHAR_BUFF-4) )
                           {
                  char_buffer[char_buffer_size++] = '\n';
                  char_buffer[char_buffer_size++] = 0;
                            complete_line = 1;
                           }
                        }

               inpchar[0]=0;

                    if (complete_line == 0)
                      {
                       len = S_READ(sock, inpchar, 1);
		      }

} /* mode == 2 */
                   } /* end of while */

                /*--------------------------------------*/
                /* terminate the line                   */
                /*--------------------------------------*/

#if defined(MISC_DEBUG)
 write_log(DEBUGLOG,YESTIME,"\nBuffer is \"%s\" \n",char_buffer);
#endif

                char_buffer[char_buffer_size] = 0;

                /*------------------------------------------------*/
                /* check for complete line (terminated by \n)     */
                /*------------------------------------------------*/

                if (!complete_line)
                 {
    /*-----------------------------------------------------*/
    /* need to support char mode, no local echo, some time */
    /*-----------------------------------------------------*/
    /* write_str_nr(user,char_buffer[char_buffer_size]); */
    /*-----------------------------------------------------*/
		  if (mode==1) { /* wwwport */
		   if (strlen(char_buffer)!=wwwport[user].req_length) {
			/* we check for this because IE is a bastard child */
			/* of a skinny, standard-deviating, nerdy twit */
		   /* write_log(DEBUGLOG,YESTIME,"length doesn't match!\n"); */
                   return -4;
		   } /* strlen */
		  } /* mode */
                 } /* complete_line */

                /*--------------------------------------------*/
                /* copy the user buffer to the input string   */
                /*--------------------------------------------*/

                strcpy(astring, char_buffer);
                buff_size = strlen(astring);
                char_buffer_size = 0;

		if (mode==1) {
                if (astring[0] == '\012') return -5;
		}
		else if (mode==2) {
#if defined(MISC_DEBUG)
 if (astring[0] == '\015') write_log(DEBUGLOG,YESTIME,"astring is a single r\n");
 else if (astring[0] == '\012') write_log(DEBUGLOG,YESTIME,"astring is a single n\n");
#endif
                 if (astring[0] == '\012' && miscconn[user].last_return[0] == '\015') {
			/* \r\n */
#if defined(MISC_DEBUG)
			write_log(DEBUGLOG,YESTIME,"matched CRLF\n");
#endif
			miscconn[user].last_return[0]='\r'; /* set to \r */
			return -5;
		 }
                 else if (astring[0] == '\015' && miscconn[user].last_return[0] == '\012') {
			/* \n\r */
#if defined(MISC_DEBUG)
			write_log(DEBUGLOG,YESTIME,"matched LFCR\n");
#endif
			miscconn[user].last_return[0]='\n'; /* set to \n */
			return -5;
		 }
		}
			 miscconn[user].last_return[0]=astring[buff_size-1];
#if defined(MISC_DEBUG)
		if (miscconn[user].last_return[0]=='\015')
 write_log(DEBUGLOG,YESTIME,"our endline is an r\n");
		else if (miscconn[user].last_return[0]=='\012')
 write_log(DEBUGLOG,YESTIME,"our endline is an n\n");
		else
 write_log(DEBUGLOG,YESTIME,"our endline is a %c\n",miscconn[user].last_return[0]);
#endif

                /*----------------------------------------------------*/
                /* some nice users were doing some things that would  */
                /* intentionally kill the system.  This should trap   */
                /* that and report such incidents.                    */
                /*----------------------------------------------------*/
                
                if (buff_size > 8000)
                  {
                    write_log(WARNLOG,YESTIME,"%s HACK flood from site %21.21s\n",
                                  mode==1?"WWW":"RWHO",mode==1?wwwport[user].site:miscconn[user].site);
		    free_sock(user,port);
                    sprintf(mess,"%s %s HACK flood from site %21.21s\n",
                                  STAFF_PREFIX,mode==1?"WWW":"RWHO",mode==1?wwwport[user].site:miscconn[user].site);
                    writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0); 
                    
                    return -6;
                  } /* end of if buff size */

} /* end of if mode 1 or 2 */
                 
                /*-------------------------------------*/
                /* terminate the string                */
                /*-------------------------------------*/

#if defined(MISC_DEBUG)
 write_log(DEBUGLOG,YESTIME,"astring1 is %s\n",astring);
#endif

		/* misc. operations */
		terminate(user, astring);
#if defined(MISC_DEBUG)
 write_log(DEBUGLOG,YESTIME,"astring2 is %s\n",astring);
#endif

strcpy(t_inpstr,astring);
return 0;
}


/* free a misc. sockets structures */
void free_sock(int user, char port)
{
                                  
if (port=='3') {
	if (whoport[user].sock != -1) {
		 SHUTDOWN(whoport[user].sock, 2);
                 while (CLOSE(whoport[user].sock) == -1 && errno == EINTR)
                        ; /* empty while */
                 FD_CLR(whoport[user].sock,&readmask);
                 FD_CLR(whoport[user].sock,&writemask);
	}
   whoport[user].sock=-1;
   whoport[user].site[0]=0;
   whoport[user].net_name[0]=0;
  }               
else if (port=='4') {
	if (wwwport[user].sock != -1) {
		 SHUTDOWN(wwwport[user].sock, 2);
                 while (CLOSE(wwwport[user].sock) == -1 && errno == EINTR)
                        ; /* empty while */
                 FD_CLR(wwwport[user].sock,&readmask);
                 FD_CLR(wwwport[user].sock,&writemask);
	}
   wwwport[user].sock=-1;
   wwwport[user].method=-1;
   wwwport[user].req_length=0;
   wwwport[user].keypair[0]=0;
   wwwport[user].file[0]=0;   
   wwwport[user].site[0]=0;   
   wwwport[user].net_name[0]=0;
   if (wwwport[user].output_data) free(wwwport[user].output_data);
   wwwport[user].output_data=NULL;
   wwwport[user].write_offset=0;  
   wwwport[user].alloced_size=0;  
  }
else if (port=='5') {
	if (miscconn[user].sock != -1) {
		 SHUTDOWN(miscconn[user].sock, 2);
                 while (CLOSE(miscconn[user].sock) == -1 && errno == EINTR)
                        ; /* empty while */
                 FD_CLR(miscconn[user].sock,&readmask);
                 FD_CLR(miscconn[user].sock,&writemask);
	}
   if (miscconn[user].type == 1) ustr[miscconn[user].user].rwho=1;
   miscconn[user].sock=-1;
   miscconn[user].user=-1;
   miscconn[user].type=-1;
   miscconn[user].stage=-1;
   miscconn[user].option=-1;
   miscconn[user].port=-1;
   miscconn[user].time=0;
   miscconn[user].fd=NULL;
   miscconn[user].ready=0;
   miscconn[user].queuename[0]=0;
   miscconn[user].last_return[0]=0;
   miscconn[user].site[0]=0;   
  }
   
}  


/*-------------------------------------------------------------------*/
/* Set up main listening sockets                                     */
/*-------------------------------------------------------------------*/
void make_sockets()
{
struct sockaddr_in bind_addr;       /* AF_INET sockaddr structure */
int i,open_port=0;
int on=1;
int size=sizeof(struct sockaddr_in);
#if defined(WINDOWS)
unsigned long arg = 1;
#endif

for (i=0;i<4;++i) {

/*---------------------------------------*/
/* status text                           */
/*---------------------------------------*/
if (i==0) {
 open_port = PORT;
 if (!restarting) {
 printf("Main talker port:\n\n");
 printf("   use port: %d\n",open_port);
 }
 else continue;
 }
else if (i==1) {   
 if (WIZ_OFFSET != 0) {
 open_port = PORT + WIZ_OFFSET;
 if (!restarting) {
 printf("Wizard port:\n\n");
 printf("   use port: %d\n",open_port);
 }
 else {
 /* we are soft-booting and want to start this port		*/
 /* was it open? If so, move on to next port, If not, start it	*/
  if (listen_sock[i]!=-1) continue;
 }
 } /* END OF IF OFFSET */
 else {
 /* we are soft-booting and DONT want to start this port	   */
 /* was it open? If so, close it, If not, move on to the next port */
	wiz_access=0;
	if (restarting) {
	if (listen_sock[i]!=-1) {
	SHUTDOWN(listen_sock[i], 2);
	while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
	; /* empty while */
	write_log(BOOTLOG,YESTIME,"Closed WIZ port on soft-reboot\n");
	}
	}
	continue;
 } /* END OF ELSE OFFSET */
} /* END OF ELSE IF */
else if (i==2) {
 if (WHO_OFFSET != 0) {
 open_port = PORT + WHO_OFFSET;
 if (!restarting) {
 printf("External WHO list port:\n\n");
 printf("   use port: %d\n",open_port);
 }
 else {
  if (listen_sock[i]!=-1) continue;
 }
 } /* END OF IF OFFSET */
 else {
	who_access=0;
	if (restarting) {
        if (listen_sock[i]!=-1) {
	SHUTDOWN(listen_sock[i], 2);
        while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
        ; /* empty while */
        write_log(BOOTLOG,YESTIME,"Closed WHO port on soft-reboot\n");
        }
	}
	continue;
 } /* END OF ELSE OFFSET */
} /* END OF ELSE IF */
else if (i==3) {   
 if (WWW_OFFSET != 0) {
 open_port = PORT + WWW_OFFSET;
 if (!restarting) {
 printf("Mini WWW port:\n\n");
 printf("   use port: %d\n",open_port);
 }
 else {
  if (listen_sock[i]!=-1) continue;
 }
 } /* END OF IF OFFSET */
 else {
	www_access=0;
	if (restarting) {
        if (listen_sock[i]!=-1) {
	SHUTDOWN(listen_sock[i], 2);
        while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
        ; /* empty while */
        write_log(BOOTLOG,YESTIME,"Closed WWW port on soft-reboot\n");
        }
	}
	continue;
 } /* END OF ELSE OFFSET */
} /* END OF ELSE IF */

/* Zero out memory for address */
memset((char *)&bind_addr, 0, size);

#if defined(FREEBSD_SYS)        
bind_addr.sin_len = sizeof(bind_addr);
#endif       
bind_addr.sin_family      = AF_INET;   /* setup address struct */
bind_addr.sin_addr.s_addr = strlen(BIND_INTER) ? inet_addr(BIND_INTER):INADDR_ANY; 
bind_addr.sin_port        = htons((unsigned short)open_port); /* with local host info */

 
/*-------------------------------------------------*/
/* Create a socket for use                         */
/*-------------------------------------------------*/

if ((listen_sock[i] = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
 {
  if (!restarting) {
  printf("   ***CANNOT CREATE SOCKET***\n");
  printf("\n   Cannot create socket, aborting startup!\n");
  }
  else {
  write_log(ERRLOG,YESTIME,"Cant create listening socket %d:%d on soft-reboot! %s\n",i,open_port,get_error());
  }
#if defined(WINDOWS)
WSACleanup();
#endif
  exit(0);
 }

/* Set address reusable */
if (setsockopt(listen_sock[i], SOL_SOCKET, SO_REUSEADDR, (char *) &on,
    sizeof(on))== -1) {
   if (!restarting)
    printf("\n   Cannot setsockopt(), aborting startup!\n");
   else
    write_log(ERRLOG,YESTIME,"Cant setsockopt on listening socket %d:%d on soft-reboot! %s\n",i,open_port,get_error());

   SHUTDOWN(listen_sock[i], 2);
   while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
	; /* empty while */
#if defined(WINDOWS)
WSACleanup();
#endif
   exit(0);
   }
  
/*----------------------------*/
/* Set socket to non_blocking */
/*----------------------------*/

/*
#if defined(WINDOWS)
 if (ioctlsocket(listen_sock[i], FIONBIO, &arg) == -1) {
#else
 if (fcntl(listen_sock[i], F_SETFL, NBLOCK_CMD)== -1) {
#endif
*/
 if (MY_FCNTL(listen_sock[i], MY_F_SETFL, NBLOCK_CMD)==SOCKET_ERROR) {
   if (!restarting)
    printf("\n   Cannot set binding socket to non-blocking, aborting startup!\n");
   else
    write_log(ERRLOG,YESTIME,"Cant set listening socket %d:%d non-blocking on soft-reboot! %s\n",i,open_port,get_error());

   SHUTDOWN(listen_sock[i], 2);
   while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
	; /* empty while */
#if defined(WINDOWS)
WSACleanup();
#endif
   exit(0);
  }

/* vax users change the above line after #else to: */
/* socket_ioctl(listen_sock[i], FIONBIO, &arg);    */
/* declare arg int arg = 1                         */
   
/*-------------------------------------------------*/
/* Bind the socket to local machine and port       */
/* giving the socket the local address ADDR        */
/*-------------------------------------------------*/

if (bind(listen_sock[i], (struct sockaddr *)&bind_addr, size) == SOCKET_ERROR)
  {
   if (!restarting) {
   printf("   ***CANNOT BIND TO PORT***\n");
   printf("\n   Cannot bind to port, server may be already running, aborting startup!\n");
   }
   else {
   write_log(ERRLOG,YESTIME,"Cant bind listening socket %d:%d on soft-reboot! %s\n",i,open_port,get_error());
   }
   SHUTDOWN(listen_sock[i], 2);
   while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
	; /* empty while */
#if defined(WINDOWS)
WSACleanup();
#endif
   exit(0);
  }

/*------------------------------------------------------*/ 
/* Listen on the socket                                 */
/* second arg of listen() is the backlog of connections */
/* we can hold. We'll make it 5 to be safe. Linux       */
/* silently limits to 128, SunOS to 5. Go figure        */
/*------------------------------------------------------*/

if (listen(listen_sock[i], 5)==SOCKET_ERROR)
  {
   if (!restarting) {
   printf("   ***LISTEN FAILED ON PORT***\n");
   printf("\n   Cannot listen on port, aborting startup!\n");
   }
   else {
   write_log(ERRLOG,YESTIME,"Cant listen on socket %d:%d on soft-reboot! %s\n",i,open_port,get_error());
   }
   SHUTDOWN(listen_sock[i], 2);
   while (CLOSE(listen_sock[i]) == -1 && errno == EINTR)
	; /* empty while */
#if defined(WINDOWS)
WSACleanup();
#endif
   exit(0);
  }

if (!restarting)
printf("   port created, bound, and listening\n\n");
else
write_log(BOOTLOG,YESTIME,"Started socket %d on port %d on soft-reboot\n",i,open_port);

 } /* end of for */
}


void check_sockets(void) {
int u,ret=0;

for (u=0;u<MAX_WHO_CONNECTS;++u) {
        if (whoport[u].sock != -1) {
        ret=MY_FCNTL(whoport[u].sock,MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for whoport sock %d\n",
        ret,whoport[u].sock);
        if (ret!=0) free_sock(u,'3');
        }
}
for (u=0;u<MAX_WWW_CONNECTS;++u) {
        if (wwwport[u].sock != -1) {
        ret=MY_FCNTL(wwwport[u].sock,MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for wwwport sock %d\n",
        ret,wwwport[u].sock);
        if (ret!=0) free_sock(u,'4');
        }
}
for (u=0;u<MAX_MISC_CONNECTS;++u) {
        if (miscconn[u].sock != -1) {
        ret=MY_FCNTL(miscconn[u].sock,MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for miscconn sock %d\n",
        ret,miscconn[u].sock);
        if (ret!=0) free_sock(u,'5');
        }
}
for (u=0;u<MAX_USERS;++u) {
        if (ustr[u].sock != -1) {
        ret=MY_FCNTL(ustr[u].sock,MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for sock %d user %s\n",
        ret,ustr[u].sock,ustr[u].name);
        if (ret!=0) user_quit(u,0);
        }
}
for (u=0;u<4;++u) {
        if (listen_sock[u] != -1) {
        ret=MY_FCNTL(listen_sock[u],MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for listen_sock %d\n",
        ret,listen_sock[u]);
        }
}
        if (FROM_CLIENT_READ != -1) {
        ret=MY_FCNTL(FROM_CLIENT_READ,MY_F_GETFD,0);
        write_log(DEBUGLOG,YESTIME,"Returned %d for FROM_CLIENT_READ %d\n",
        ret,FROM_CLIENT_READ);
        if (ret!=0) {
		/* kill it */
                kill_resolver_clipon();
		/* if we dont want it, do nothing */
		/* else restart it */
		if (resolve_names!=2 && resolve_names!=3) {
		/* we probably will never get here, but just to be safe */
		}
		else {
	                sleep(2);
        	        init_resolver_clipon();
		}
        } /* ret */
        } /* FROM_CLIENT_READ */

}


int MY_FCNTL(int a, int b, unsigned long c) {
unsigned long fcntl_arg;

fcntl_arg=-1;

#if defined(HAVE_FCNTL_H) || defined(HAVE_SYS_FCNTL_H)
        return fcntl(a,b,c);
#elif defined(HAVE_SYS_IOCTL_H)
        fcntl_arg = c;
        return ioctl(a,b,&fcntl_arg);
#else
        fcntl_arg = c;
        return ioctlsocket(a,b,&fcntl_arg);
#endif
        return -1;
}

int SHUTDOWN(int sock, int how) {

#if defined(CYGWIN_SYS)
	return shutdown(sock, how);
#endif

return 1;
}

