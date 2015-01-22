#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern char mess[ARR_SIZE+25];
extern char t_mess[ARR_SIZE+25];
extern int PORT;
extern int NUM_AREAS;
extern char web_opts[11][64];
extern char thishost[101];     /* FQDN were running on */
extern int num_of_users;       /* total number of users online */


/*** prints who is on the system to requesting user ***/
void external_who(int as)
{
int    s,u,vi,min,idl,invis=0;
char   ud[100],un[100],an[ROOM_LEN],und[200];
char   temp[256];
char   i_buff[5];
time_t tm;

/*-------------------------------------------------------------------------*/
/* write out title block                                                   */
/*-------------------------------------------------------------------------*/
if (EXT_WHO1) S_WRITE(as,EXT_WHO1, strlen(EXT_WHO1) );
if (EXT_WHO2) S_WRITE(as,EXT_WHO2, strlen(EXT_WHO2) );
if (EXT_WHO3) {
  sprintf(mess,EXT_WHO3,SYSTEM_NAME,PORT+WHO_OFFSET);
  S_WRITE(as,mess, strlen(mess) );        
  }
if (EXT_WHO4) S_WRITE(as,EXT_WHO4, strlen(EXT_WHO4) );

/* display current time */
time(&tm);
sprintf(mess,WHO_PLAIN,ctime(&tm));
strcat(mess,"\n\r");
S_WRITE(as, mess, strlen(mess));

/* Give Display format */
/* sprintf(mess,"Room             Time Stat Idl   Name/Description\n"); */
sprintf(mess,"%-*s  Time Stat Idl   Name/Description\n",ROOM_LEN,"Room");
S_WRITE(as, mess, strlen(mess));

/* display user list */
for (u=0;u<MAX_USERS;++u) {
	if ((ustr[u].area!=-1) && (!ustr[u].logging_in))
	  {
		if (!ustr[u].vis)
	          { 
	            invis++;  
	            continue; 
	          }
			 
		min=(tm-ustr[u].time)/60;
		idl=(tm-ustr[u].last_input)/60;

		strcpy(un,strip_color(ustr[u].say_name));
    
                if (ustr[u].afk==0)
                 strcpy(ud,ustr[u].desc);
                else if ((strlen(ustr[u].afkmsg) > 1) && (ustr[u].afk>=1))
                 strcpy(ud,ustr[u].afkmsg);
                else if (!strlen(ustr[u].afkmsg) && (ustr[u].afk>=1))
                 strcpy(ud,ustr[u].desc);
 		
		strcpy(und,un);
		strcat(und," ");		
		strcat(und,ud);
		
		if (!astr[ustr[u].area].hidden)
		  {
		    strcpy(an,astr[ustr[u].area].name);
		  }
		 else
		  { 
		    strcpy(an, "        ");
		  }
		  
		s=' ';
		   
		if (ustr[u].afk == 1) {
                   strcpy(i_buff,"AFK ");
                  }
		 else if (ustr[u].afk == 2) {
                      strcpy(i_buff,"BAFK");
                     }
                 else if (ustr[u].pro_enter) {
                      strcpy(i_buff,"PROF");
                     }
                 else if (ustr[u].vote_enter) {
                      strcpy(i_buff,"VOTE");
                     }
                 else if (ustr[u].roomd_enter) {
                      strcpy(i_buff,"DESC");
                     }
		 else {
                  if (idl < 3)
                    strcpy(i_buff,"Actv");
                  else if (idl >= 3 && idl < 60)
                    strcpy(i_buff,"Awke");
                  else if (idl >= 60 && idl < 180)
		    strcpy(i_buff,"Idle"); 
                  else if (idl >= 180)
                    strcpy(i_buff,"Coma");
                    } 

		if (!ustr[u].vis) vi='_';
                else vi=' ';

	sprintf(mess,"%-*s %-5.5d %s %3.3d %c%c%s\n",ROOM_LEN,an,min,i_buff,idl,s,vi,und);
	        
		strncpy(temp,mess,256);
		strtolower(temp);
		
		mess[0]=toupper((int)mess[0]);
                strcpy(mess, strip_color(mess));
		S_WRITE(as, mess, strlen(mess));
	       }
	}
	
if (invis) 
  {
   sprintf(mess,SHADOW_PLAIN,invis == 1 ? "is" : "are",invis,invis == 1 ? " " : "s");
   S_WRITE(as, mess, strlen(mess));
  }

S_WRITE(as, "\n\n", 1);
sprintf(mess,USERCNT_PLAIN,num_of_users,num_of_users == 1 ? "" : "s");
S_WRITE(as, mess, strlen(mess) );
S_WRITE(as, "\n\n\n", 2);
}


/* Parse web server port input */
void parse_input(int user, char *inpstr)
{
int i=0,found=0,photo=0;
char *query;
char chunk[ARR_SIZE];
char type1[80];
char command[ARR_SIZE];
char request[ARR_SIZE];
char photo_request[300];
char request2[300];
char servtype[ARR_SIZE];
char wusername[ARR_SIZE];
char wpassword[ARR_SIZE];
char filename[FILE_NAME_LEN+50];
time_t tm;
struct stat fileinfo;

time(&tm);
inpstr[3000]=0;
chunk[0]=0;
wusername[0]=0;
wpassword[0]=0;
request[0]=0;
servtype[0]=0;
type1[0]=0;
command[0]=0;

if (wwwport[user].method==1)
 strcpy(command,"POST");
else if (wwwport[user].method==0) {
 strcpy(command,"GET");
}
else if (wwwport[user].method==2) {
 strcpy(command,"HEAD");
}
else
 sscanf(inpstr,"%s ",command);

if (!strcmp(command,"GET")) {
 if (wwwport[user].method==0) {
  /* next GET line */
  if (strlen(inpstr)) return;

sscanf(wwwport[user].file,"%s ",request);
remove_first(wwwport[user].file);
/* rest of file is server type that we catted on perviously */

   if (strstr(request,"..")) {
	web_error(user,YES_HEADER,NOT_FOUND);
	goto FREE;
	}
   if (!strlen(request)) {
	web_error(user,YES_HEADER,BAD_REQUEST);
	goto FREE;
	}
   if (!strlen(wwwport[user].file) || !strstr(wwwport[user].file,"HTTP")) {
	web_error(user,YES_HEADER,BAD_REQUEST);
	goto FREE;
	}

 goto NEXT;
 }
 else {
   wwwport[user].method=0;
   remove_first(inpstr);
   sscanf(inpstr,"%s ",request);
   
 /* Strip leading slash off filename request and truncate */
 midcpy(request,request,0,255);
 strcpy(wwwport[user].file,request);

 /* now get server type and cat on to end for later processing */
   remove_first(inpstr);
   sscanf(inpstr,"%s ",servtype);
   midcpy(servtype,request,0,9);
   strcat(wwwport[user].file," ");
   strcat(wwwport[user].file,servtype);

 return;
 }
} /* end of GET */
else if (!strcmp(command,"POST")) {
 if (wwwport[user].method==1) {
        if (strlen(inpstr)) {
#if defined(POST_DEBUG)
        write_log(DEBUGLOG,NOTIME,"POST line: %s\n",inpstr);
#endif
        sscanf(inpstr,"%s ",request);
        }
        else return;

        if (wwwport[user].req_length) {
          if (!strstr(inpstr,": ")) {
#if defined(POST_DEBUG)
                write_log(DEBUGLOG,NOTIME,"Shortening length from %d to %d..\n",wwwport[user].req_length,wwwport[user].req_length-strlen(inpstr));
#endif
                wwwport[user].req_length-=strlen(inpstr);
#if defined(POST_DEBUG)
                write_log(DEBUGLOG,NOTIME,"Shortened\n");
#endif
                if (!strlen(wwwport[user].keypair))
		  strcpy(wwwport[user].keypair,inpstr);
		else {
		  strcat(wwwport[user].keypair,inpstr);
		  }
#if defined(POST_DEBUG)
                write_log(DEBUGLOG,NOTIME,"Keypair: %s\n",wwwport[user].keypair);
#endif
                if (!wwwport[user].req_length) goto POST;
            }  /* end of if !strstr */
         } /* end of if req_length */
        else if (!strcmp(request,"Content-length:") ||
		 !strcmp(request,"Content-Length:")) {
	/* we check for the latter because IE is a bastard child */
	/* of a skinny, standard-deviating, nerdy twit */
#if defined(POST_DEBUG)
          write_log(DEBUGLOG,NOTIME,"Saving content length\n");
#endif
          remove_first(inpstr);
          wwwport[user].req_length=atoi(inpstr);
#if defined(POST_DEBUG)
          write_log(DEBUGLOG,NOTIME,"Saved content length\n");
#endif
        } /* end of else if */
        return;
   } /* end of method mode */
 else {
   wwwport[user].method=1;
   remove_first(inpstr);
   sscanf(inpstr,"%s ",request);
   remove_first(inpstr);
   sscanf(inpstr,"%s ",servtype);
   if (!strlen(servtype) || !strstr(servtype,"HTTP")) {
        web_error(user,YES_HEADER,NOT_FOUND);
        goto FREE;
        }
   }

  } /* end of POST */
else if (!strcmp(command,"HEAD")) {
 if (wwwport[user].method==2) {
  /* next HEAD line */
  if (strlen(inpstr)) return;

sscanf(wwwport[user].file,"%s ",request);
remove_first(wwwport[user].file);
/* rest of file is server type that we catted on perviously */

   if (strstr(request,"..")) {
	web_error(user,YES_HEADER,NOT_FOUND);
	goto FREE;
	}
   if (!strlen(request)) {
	web_error(user,YES_HEADER,BAD_REQUEST);
	goto FREE;
	}
   if (!strlen(wwwport[user].file) || !strstr(wwwport[user].file,"HTTP")) {
	web_error(user,YES_HEADER,BAD_REQUEST);
	goto FREE;
	}

 goto NEXT;
 }
 else {
   wwwport[user].method=2;
   remove_first(inpstr);
   sscanf(inpstr,"%s ",request);
   
 /* Strip leading slash off filename request and truncate */
 midcpy(request,request,0,255);
 strcpy(wwwport[user].file,request);

 /* now get server type and cat on to end for later processing */
   remove_first(inpstr);
   sscanf(inpstr,"%s ",servtype);
   midcpy(servtype,request,0,9);
   strcat(wwwport[user].file," ");
   strcat(wwwport[user].file,servtype);

 return;
 }
  } /* end of HEAD */
else {
	web_error(user,YES_HEADER,BAD_REQUEST);
	goto FREE;
  }

NEXT:

/* Strip leading slash off filename request and truncate */
midcpy(request,request,1,256);

if (!strlen(request))
 sprintf(filename,"%s/%s",WEBFILES,web_opts[0]);
else
 sprintf(filename,"%s/%s",WEBFILES,request);

/* check to see if the file is a directory */
if (stat(filename,&fileinfo) != -1) {
 /* if (S_ISDIR(fileinfo.st_mode)) { */
 if (((fileinfo.st_mode) & S_IFMT) == S_IFDIR) {
  sprintf(mess,"%s%s%s",filename,filename[strlen(filename)-1]=='/'?"":"/",web_opts[0]);
  strncpy(filename,mess,sizeof(filename));
 }
}

#if defined(POST_DEBUG)
write_log(DEBUGLOG,NOTIME,"Request: %s Filename: %s\n",request,filename);
#endif

if (request[0]=='_') {
	for (i=0;i<strlen(request);++i) {
	   if (request[i]=='\?') { found=1; break; }
	   }
	if (found) midcpy(request,chunk,0,i-1);
	else midcpy(request,chunk,0,257);
   }

#if defined(POST_DEBUG)
write_log(DEBUGLOG,NOTIME,"Request-processed: %s Filename-processed: %s\n",request,filename);
#endif

POST:
/* If POST method */
if (wwwport[user].method==1) {
  if (strlen(wwwport[user].keypair)) {
   /* get the request back from memory to process now */
   strcpy(chunk,wwwport[user].file);
   strcpy(request,wwwport[user].keypair);
#if defined(POST_DEBUG)
	write_log(DEBUGLOG,NOTIME,"Chunk: %s\n",chunk);
	write_log(DEBUGLOG,NOTIME,"Request: %s\n",request);
#endif
   found=1;
   }
  else {
   /* save requested url to memory so we can read later */
   strcpy(wwwport[user].file,chunk);
   return;
   }
  }

if (strcmp(chunk,"_who") && strcmp(chunk,"_users") && strcmp(chunk,"_login") && strcmp(chunk,"_menu")) {
#if defined(POST_DEBUG)
write_log(DEBUGLOG,NOTIME,"Request3: %s Filename3: %s\n",request,filename);
#endif
 if (!check_for_file(filename)) {
   web_error(user,YES_HEADER,NOT_FOUND);
   goto FREE;
   }
 }

write_log(DEBUGLOG,YESTIME,"Responding1 to %s\n",filename);

	write_str_www(user,"HTTP/1.0 200 OK\n", -1);
	sprintf(mess, "Server: %s/1.0\n",SYSTEM_NAME);
	write_str_www(user, mess, -1);
	sprintf(mess,"Date: %s",ctime(&tm));
	write_str_www(user, mess, -1);

if (!strcmp(chunk,"_who")) {
  external_www(user);
  goto FREE;
  }

if (!strcmp(chunk,"_users")) {
	/* If question mark not found, its a request for all users list */
	/* else for a specific user if query is greater than 1 or group */
	/* if equal to 1						*/
   if (!found)
   	external_users(user,3,NULL,photo);
   else {
	if (wwwport[user].method != 1) {
	  midcpy(request,request,i+1,257);
	  found=0; i=0;
	  for (i=0;i<strlen(request);++i) {
	     if (request[i]=='=') { found=1; break; }
	     }
	  if (found) midcpy(request,request,i+1,257);
	  else { web_error(user,YES_HEADER,BAD_REQUEST); goto FREE; }
/* */
	  if (strstr(request,"&")) {
	  found=0; i=0;
	  for (i=0;i<strlen(request);++i) {
	     if (request[i]=='&') { break; }
	     }
	  if (i==0)
	  strcpy(request2,"");
	  else
	  midcpy(request,request2,0,i-1);
	  found=0;
	  for (i=i+1;i<strlen(request);++i) {
	     if (request[i]=='=') { found=1; break; }
	     }
	  if (found) {
	  midcpy(request,photo_request,i+1,257);
	  strtolower(photo_request);
	  if (!strcmp(photo_request,"yes")) photo=1;
	  else photo=0;
	  }
	  else photo=0;
	  strcpy(request,request2);
	  } /* end of if strstr */
/* */
	}
	else {
	  query=NULL;
	  if ((query=get_value(request,"search"))==NULL) {
	   query="\0";
	   /* web_error(user,YES_HEADER,BAD_REQUEST); goto FREE; */
	   }
	  strcpy(request2,query);
	  query=NULL;
	  if ((query=get_value(request,"photofilter"))==NULL) {
	   photo=0;
	   }
	  else {
	  strcpy(photo_request,query);
	  strtolower(photo_request);
	  if (!strcmp(photo_request,"yes")) photo=1;
	  else photo=0;
	  }
	  strcpy(request,request2);
	}

	if (!strlen(request))
   	 external_users(user,0,NULL,photo);
	else if ((strlen(request)==1) && isalpha((int)request[0]))
   	 external_users(user,1,request,photo);
	else if ((strlen(request)==1) && isdigit((int)request[0]))
   	 external_users(user,4,request,photo);
	else
   	 external_users(user,2,request,photo);
	} /* end of else */
   goto FREE;
  } /* end of _users if */

if (!strcmp(chunk,"_login")) {
	if (wwwport[user].method != 1) {
	  external_login(user,NULL,NULL,1);
	  }
	else {
	  query=NULL;
	  if ((query=get_value(request,"username"))==NULL) {
	   external_login(user,NULL," ",1);
	   goto FREE;
	   }
	  strcpy(wusername,query);
	  query=NULL;
	  if ((query=get_value(request,"password"))==NULL) {
	   wusername[0]=0;
	   external_login(user," ",NULL,1);
	   goto FREE;
	   }
	  strcpy(wpassword,query);
	  wusername[NAME_LEN+1]=0;
	  wpassword[NAME_LEN+1]=0;
	  st_crypt(wpassword);
	  external_login(user,wusername,wpassword,1);
	}
	  goto FREE;
} /* end of if _login */

if (!strcmp(chunk,"_menu")) {
	if (wwwport[user].method != 1) {
	  external_login(user,NULL,NULL,1);
	  }
	else {
	  query=NULL;
	  if ((query=get_value(request,"username"))==NULL) {
	   external_login(user,NULL," ",1);
	   goto FREE;
	   }
	  strcpy(wusername,query);
	  query=NULL;
	  if ((query=get_value(request,"password"))==NULL) {
	   wusername[0]=0;
	   external_login(user," ",NULL,1);
	   goto FREE;
	   }
	  strcpy(wpassword,query);
	  wusername[NAME_LEN+1]=0;
	  wpassword[NAME_LEN+1]=0;
	  unescape_url(wpassword);
	  if (!external_login(user,wusername,wpassword,0)) goto FREE;
	  else {
	  query=NULL;
	  if ((query=get_value(request,"option"))==NULL) {
	   wusername[0]=0;
	   wpassword[0]=0;
	   web_error(user,YES_HEADER,BAD_REQUEST);
	   goto FREE;
	   }
	  external_menu(user,wusername,wpassword,query);
	  } /* end of else good login */

	}
	  goto FREE;
} /* end of if _menu */

write_log(DEBUGLOG,YESTIME,"Responding2 to %s\n",filename);

i=get_length(filename);
if (i==-1) {         
 web_error(user,YES_HEADER,BAD_REQUEST);
 goto FREE;
}

write_log(DEBUGLOG,YESTIME,"Responding3 to %s\n",filename);

	 sprintf(mess,"Content-length: %d\n",i);
	 write_str_www(user, mess, -1);

	strcpy(type1,get_mime_type(filename));
	if (!strcmp(type1,"bad")) {
	  web_error(user,YES_HEADER,BAD_REQUEST);
	  goto FREE;
	  }
	sprintf(mess,"Content-type: %s\n\n",type1);
        write_str_www(user, mess, -1);

write_log(DEBUGLOG,YESTIME,"Responding4 to %s\n",filename);
cat_to_www(filename,user);
write_log(DEBUGLOG,YESTIME,"Responding5 to %s\n",filename);

FREE:
return;

}


/*** prints who is on the system to requesting user ***/
void external_www(int user)
{
	int u,min,idl,invis=0;
	time_t tm;
	char an[ROOM_LEN],ud[DESC_LEN+1];
	char i_buff[5];
	char filename[256];
	FILE *fp;

	time(&tm);

        write_str_www(user, "Content-type: text/html\n\n", -1);
	write_str_www(user, "<HTML>\n", -1);
	write_str_www(user, "<HEAD>\n", -1);
	sprintf(mess, "<TITLE>%s WWW port</title>\n",SYSTEM_NAME);
	write_str_www(user, mess, -1);
	write_str_www(user, "</HEAD>\n\r", -1);

	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 sprintf(mess,"<BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 sprintf(mess,"<BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);

	write_str_www(user, mess, -1);

	/* Write our header file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[2]);
	strncpy(filename,mess,FILE_NAME_LEN);

	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWEXT: Couldn't open file(r) \"%s\" in external_www! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
			write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }


	write_str_www(user, "<center>\n", -1);
	sprintf(mess,"<font color=\"%s\"><h1>Users currently logged into %s</h1>\n",
					web_opts[9],SYSTEM_NAME);
	write_str_www(user, mess, -1);
        strcpy(filename,ctime(&tm));
        filename[24]=0;
	sprintf(mess,"<h3>on (%s)</h3>\n",filename);
	write_str_www(user, mess, -1);
        filename[0]=0;
	write_str_www(user, "<br><center></font>\n", -1);
        sprintf(mess,"<A HREF=\"telnet://%s:%d\">Connect to %s</a><br>",thishost,PORT,SYSTEM_NAME);
	write_str_www(user, mess, -1);
        write_str_www(user,"</center>\n", -1);
        write_str_www(user,"<table border>\n\r", -1);
	write_str_www(user, "<tr><th><b>Room</b></th><th><b>Name/Description</b></th><th><b>Time on</b></th><th><b>Status</b></th><th><b>Idle</b></th></tr>\n\r", -1);
	
	for (u=0;u<MAX_USERS;++u) {
		if ((ustr[u].area!=-1) && (!ustr[u].logging_in))  {
			if (!ustr[u].vis) { 
	            invis++;  
	            continue; 
			}
			min=(tm-ustr[u].time)/60;
			idl=(tm-ustr[u].last_input)/60;

                if (ustr[u].afk==0)
		 strcpy(ud,ustr[u].desc);
		else if ((strlen(ustr[u].afkmsg) > 1) && (ustr[u].afk>=1))
		 strcpy(ud,ustr[u].afkmsg);
                else if (!strlen(ustr[u].afkmsg) && (ustr[u].afk>=1))
                 strcpy(ud,ustr[u].desc);

			if (!astr[ustr[u].area].hidden) {
				strcpy(an,astr[ustr[u].area].name);
			}
			else { 
				strcpy(an, "        ");
			}
			if (ustr[u].afk == 1) {
				strcpy(i_buff,"AFK ");
			}
			else if (ustr[u].afk == 2) {
				strcpy(i_buff,"BAFK");
			}
			else {
				strcpy(i_buff,"Idle"); 
			}
				sprintf(mess,"<tr> <td>%s</td> <td><a href=\"http://%s:%d/_users?search=%s\">%s</a> %s</td> <td align=\"center\">%i</td> <td align=\"center\">%s</td> <td align=\"center\">%i</td></tr>\n\r",
			an,thishost,PORT+WWW_OFFSET,strip_color(ustr[u].say_name),ustr[u].say_name,ud,min,i_buff,idl);
                        strcpy(mess, convert_color(mess));
			write_str_www(user, mess, -1);
		}
	}
	if(invis) {
		sprintf(mess,SHADOW_WWW,invis == 1 ? "is" : "are",invis,invis == 1 ? " " : "s");
		write_str_www(user, mess, -1);
	}
	sprintf(mess,USERCNT_WWW,num_of_users == 1 ? "is" : "are",num_of_users,num_of_users == 1 ? "" : "s");
	write_str_www(user, mess, -1);

	write_str_www(user, "</table><br>\n</center>\n", -1);

	/* Write our footer file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[3]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWEXT: Couldn't open file(r) \"%s\" in external_www! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }

	sprintf(mess,"<br><b><center>This web page dynamically generated on %s</b></center><br>",ctime(&tm));
	write_str_www(user, mess, -1);

	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\n", -1);
}


int external_login(int user, char *wusername, char *wpassword, int wlogin)
{
	int bad=0;
	char filename[FILE_NAME_LEN];
	time_t tm;
	FILE *fp;

	time(&tm);

        write_str_www(user, "Content-type: text/html\n\n", -1);
	write_str_www(user, "<HTML>\n\r", -1);
	write_str_www(user, "<HEAD>\n\r", -1);
	if (wusername==NULL && wpassword==NULL)
	write_str_www(user, "<TITLE>Web login</TITLE>\n\r", -1);
	else if (wusername==NULL)
	write_str_www(user, "<TITLE>No username</TITLE>\n\r", -1);
	else if (wpassword==NULL)
	write_str_www(user, "<TITLE>No password</TITLE>\n\r", -1);

	write_str_www(user, "</HEAD>\n\r", -1);

	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 sprintf(mess,"<BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 sprintf(mess,"<BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);

	write_str_www(user, mess, -1);

	/* Write our header file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[2]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWLOGIN: Couldn't open file(r) \"%s\" in external_login! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }

if (wusername==NULL || wpassword==NULL) {
	if (wusername==NULL && wpassword==NULL) {
	/* normal to URL */
	write_str_www(user, "<font size=\"+2\">Web port login</font><br><br>\n\r", -1);
	}
	else if (wusername==NULL) {
	/* POST, no username given in submission */
	write_str_www(user, "<font size=\"+2\">No username given!</font><br><br>\n\r", -1);
	}
	else if (wpassword==NULL) {
	/* POST, no password given in submission */
	write_str_www(user, "<font size=\"+2\">No password given!</font><br><br>\n\r", -1);
	}
	/* write out login form */
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_login\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<b>Username:</b><br>\n\r", -1);
	sprintf(mess,"<INPUT TYPE=\"TEXT\" NAME=\"username\" SIZE=%d MAXLENGTH=%d><br><br>\n\r",NAME_LEN+1,NAME_LEN+1);
	write_str_www(user, mess, -1);
	write_str_www(user, "<b>Password:</b><br>\n\r", -1);
	sprintf(mess,"<INPUT TYPE=\"PASSWORD\" NAME=\"password\" SIZE=%d MAXLENGTH=%d><br><br>\n\r",NAME_LEN+1,NAME_LEN+1);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Login\">", -1);
	write_str_www(user, "</FORM>", -1);
	bad=1;
} /* end of main if */
else {
	/* POST, user trying to login with some thing in	*/
	/* both fields. Check for valid username and password.	*/
	strtolower(wusername);
	if (!check_for_user(wusername)) {
	  bad=1;
	  }
	else {
	  read_user(wusername);
	  if (strcmp(t_ustr.password,wpassword)) bad=2;
	} /* end of else username exists */

	if (bad) {
	if (bad==1) {
	sprintf(mess,"<font size=\"+2\">%s</font><br><br>\n\r",NO_USER_STR);
	write_str_www(user, mess, -1);
	}
	else if (bad==2) {
	write_str_www(user, "<font size=\"+2\">Password incorrect!</font><br><br>\n\r", -1);
	}
	/* write out login form */
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_login\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<b>Username:</b><br>\n\r", -1);
	sprintf(mess,"<INPUT TYPE=\"TEXT\" NAME=\"username\" SIZE=%d MAXLENGTH=%d><br><br>\n\r",NAME_LEN+1,NAME_LEN+1);
	write_str_www(user, mess, -1);
	write_str_www(user, "<b>Password:</b><br>\n\r", -1);
	sprintf(mess,"<INPUT TYPE=\"PASSWORD\" NAME=\"password\" SIZE=%d MAXLENGTH=%d><br><br>\n\r",NAME_LEN+1,NAME_LEN+1);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Login\">", -1);
	write_str_www(user, "</FORM>", -1);   
	} /* end of if bad */
	else {
	if (!wlogin) return 1;
	else {
	write_str_www(user, "<font size=\"+1\"><center>Here's what you can do:</center></font><br><br>\n\r", -1);
	write_str_www(user, "<table border=\"0\" width=\"100%\">\n\r", -1);
	write_str_www(user, "<tr><td width=\"50%\"><center>", -1);
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_menu\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"HIDDEN\" NAME=\"option\" VALUE=\"prefs\">\n\r", -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"username\" VALUE=\"%s\">\n\r",wusername);
	write_str_www(user, mess, -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n\r",wpassword);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Change/View Info\">\n\r", -1);
	write_str_www(user, "</FORM>\n\r", -1);
	write_str_www(user, "</center></td><td width=\"50%\"><center>", -1);
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_menu\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"HIDDEN\" NAME=\"option\" VALUE=\"sets\">\n\r", -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"username\" VALUE=\"%s\">\n\r",wusername);
	write_str_www(user, mess, -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n\r",wpassword);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Change/View Settings\">\n\r", -1);
	write_str_www(user, "</FORM>\n\r", -1);
	write_str_www(user, "</center></td></tr>\n\r", -1);

	write_str_www(user, "<tr><td width=\"50%\"><center>", -1);
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_menu\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"HIDDEN\" NAME=\"option\" VALUE=\"rmail\">\n\r", -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"username\" VALUE=\"%s\">\n\r",wusername);
	write_str_www(user, mess, -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n\r",wpassword);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Read Talker Mail\">\n\r", -1);
	write_str_www(user, "</FORM>\n\r", -1);
	write_str_www(user, "</center></td><td width=\"50%\"><center>", -1);
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_menu\">\n\r",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"HIDDEN\" NAME=\"option\" VALUE=\"smail\">\n\r", -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"username\" VALUE=\"%s\">\n\r",wusername);
	write_str_www(user, mess, -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n\r",wpassword);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Send Talker Mail\">\n\r", -1);
	write_str_www(user, "</FORM>\n\r", -1);
	write_str_www(user, "</center></td></tr>\n\r", -1);
	write_str_www(user, "</table>\n\r", -1);
	}
	} /* end of else good */

} /* end of main else */

	/* Write our footer file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[3]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWEXT: Couldn't open file(r) \"%s\" in external_login! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }

	sprintf(mess,"<br><b><center>This web page dynamically generated on %s</b></center><br>",ctime(&tm));
	write_str_www(user, mess, -1);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);

if (bad) return 0;
return 1;
}

void external_menu(int user, char *wusername, char *wpassword, char *woption)
{
	int z=0,gravoked=0,found=0,new_area;
	char *query2;
	char junk[256];
	char line[257];
	char filename[FILE_NAME_LEN];
	char prof[82*PRO_LINES];
	time_t tm;
	FILE *fp;

	time(&tm);
	junk[0]=0;
	line[0]=0;

/*
	  if ((query=get_value(request,"option"))==NULL) {
	   web_error(user,YES_HEADER,BAD_REQUEST);
	   free_sock(user,'4');
	   return;
	   }
*/

	read_user(wusername);

if (!strcmp(woption,"prefs")) {
	if ((query2=get_value(wwwport[user].keypair,"change"))==NULL) {
	write_str_www(user, "<center><font size=\"+1\">Your info</font></center><br>\n\r", -1);
	/* just display the info change form */
	write_str_www(user, "<font size=\"+1\">\n", -1);
	sprintf(mess,"<FORM METHOD=\"POST\" ACTION=\"http://%s:%d/_menu\">\n",thishost,PORT+WWW_OFFSET);
	write_str_www(user, mess, -1);
	write_str_www(user, "<table border=\"0\" width=\"100%\">\n", -1);
	write_str_www(user, "<tr><td width=\"25%\">Description</td>\n", -1);
	strcpy(junk,t_ustr.desc);
	if ((junk[strlen(junk)-1]=='@') && (junk[strlen(junk)-2]=='@'))
	junk[strlen(junk)-2]=0;
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".desc")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".desc"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",junk);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"desc\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",junk,(int)strlen(junk),DESC_LEN);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	strcpy(junk,t_ustr.sex);
	if ((junk[strlen(junk)-1]=='@') && (junk[strlen(junk)-2]=='@'))
	junk[strlen(junk)-2]=0;
	write_str_www(user, "<tr><td width=\"25%\">Gender</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",junk);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">ICQ #</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.icq);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Email address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.email_addr);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Homepage address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.homepage);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Web picture address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.webpic);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	}
	else {
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"sex\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",junk,(int)strlen(junk),32);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">ICQ #</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"icq\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.icq,(int)strlen(t_ustr.icq),20);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Email address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"email_addr\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.email_addr,(int)strlen(t_ustr.email_addr),EMAIL_LENGTH);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Homepage address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"homepage\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.homepage,(int)strlen(t_ustr.homepage),HOME_LEN);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Web picture address</td>\n", -1);
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"webpic\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.webpic,(int)strlen(t_ustr.webpic),HOME_LEN);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	} /* end of else not revoked or lower rank */
	write_str_www(user, "<tr><td width=\"25%\">Enter room messsage</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".entermsg")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".entermsg"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.entermsg);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"entermsg\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.entermsg,(int)strlen(t_ustr.entermsg),MAX_ENTERM);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Exit room message</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".exitmsg")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".exitmsg"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.exitmsg);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"exitmsg\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.exitmsg,(int)strlen(t_ustr.exitmsg),MAX_EXITM);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Success message</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".succ")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".succ"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.succ);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"succ\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.succ,(int)strlen(t_ustr.succ),MAX_ENTERM);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Fail message</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".fail")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".fail"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.fail);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"fail\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.fail,(int)strlen(t_ustr.fail),MAX_ENTERM);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "<tr><td width=\"25%\">Home room</td>\n", -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",t_ustr.home_room);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><input type=\"text\" name=\"home_room\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></td>\n",t_ustr.home_room,(int)strlen(t_ustr.home_room),NAME_LEN);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	sprintf(filename,"%s/%s",PRO_DIR,t_ustr.name);
	strcpy(prof,"");
	if (!(fp=fopen(filename,"r"))) {
	write_log(ERRLOG,YESTIME,"WWWMENU: Couldn't open file(r) \"%s\" in external_menu! %s\n",filename,get_error());
	write_log(ERRLOG,YESTIME,"WWWMENU: sizeof: %d\n",(int)strlen(prof));
	}
	else {
	fgets(line,256,fp);
	strcpy(prof,line);
	write_log(ERRLOG,YESTIME,"WWWMENU: past 1\n");
	while (fgets(line,256,fp)!=NULL) strcat(prof,line);
	fclose(fp);
	}
	sprintf(mess,"<tr><td width=\"25%%\">Profile (<b>%d</b> lines)<br><br>(PRESS RETURN AFTER YOUR LAST LINE)</td>\n", PRO_LINES);
	write_str_www(user, mess, -1);
 for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".entpro")) { gravoked=1; break; }
   }
	if (gravoked || (t_ustr.super < get_rank(".entpro"))) {
	gravoked=0;
	sprintf(mess,"<td width=\"75%%\">%s</td>\n",prof);
	}
	else
	sprintf(mess,"<td width=\"75%%\"><textarea name=\"profile\" cols=\"80\" rows=\"%d\" MAXLENGTH=\"%d\" WRAP=\"physical\">%s</textarea></td>\n",PRO_LINES,80*PRO_LINES,prof);
	write_str_www(user, mess, -1);
	write_str_www(user, "</tr>", -1);
	write_str_www(user, "</table><br>\n", -1);
	write_str_www(user, "<input type=\"hidden\" name=\"option\" value=\"prefs\">\n\r", -1);
	write_str_www(user, "<input type=\"hidden\" name=\"change\" value=\"yes\">\n\r", -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"username\" VALUE=\"%s\">\n\r",wusername);
	write_str_www(user, mess, -1);
	sprintf(mess, "<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n\r",wpassword);
	write_str_www(user, mess, -1);
	write_str_www(user, "<INPUT TYPE=\"SUBMIT\" VALUE=\"Save Changes\">\n\r", -1);
	write_str_www(user, "</FORM>", -1);
	write_str_www(user, "</font>\n\r", -1);
	} /* end of change if */
	else {
	/* save details */
	query2=NULL;
	/* DESCRIPTION */
	if ((query2=get_value(wwwport[user].keypair,"desc"))==NULL) {
	write_str_www(user, "You must type a description!<br>\n\r", -1);
	goto WEND;
	} /* end of desc if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".desc")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".desc"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
/*	strcat(query2,"@@"); */
	if (strlen(query2) > DESC_LEN-1)
	{
	write_str_www(user, "Description too long<br>\n\r", -1);
	goto WEND;
  	}
	if (strlen(query2)<=2) strcpy(t_ustr.desc,DEF_DESC);
	else strcpy(t_ustr.desc,query2);
	}
	} /* end of desc else */
	query2=NULL;
	/* GENDER */
	if ((query2=get_value(wwwport[user].keypair,"sex"))==NULL) {
	write_str_www(user, "You must type a gender!<br>\n\r", -1);
	goto WEND;
	} /* end of gender if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strlen(query2) > 29)
	{
	write_str_www(user, "Gender truncated, but saved<br>\n\r", -1);
	query2[29]=0;
  	}
/*	strcat(query2,"@@"); */
	if (strlen(query2)<=2) strcpy(t_ustr.sex,DEF_GENDER);
	else strcpy(t_ustr.sex,query2);
	}
	} /* end of gender else */
	query2=NULL;
	/* ICQ */
	if ((query2=get_value(wwwport[user].keypair,"icq"))==NULL) {
	write_str_www(user, "You must type an ICQ number!<br>\n\r", -1);
	goto WEND;
	} /* end of icq if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strstr(query2,"^")) {
	write_str_www(user, "ICQs cannot have color or hilite codes in them.<br>\n\r", -1);
	goto WEND;
	}
	if (strlen(query2) > 20)
	{
	write_str_www(user, "ICQ number truncated, but saved<br>\n\r", -1);
	query2[20-1]=0;
  	}
	if (!strlen(query2)) strcpy(t_ustr.icq,DEF_ICQ);
	else strcpy(t_ustr.icq,query2);
	}
	} /* end of icq else */
	query2=NULL;
	/* EMAIL ADDRESS */
	if ((query2=get_value(wwwport[user].keypair,"email_addr"))==NULL) {
	write_str_www(user, "You must type an email address!<br>\n\r", -1);
	goto WEND;
	} /* end of email_addr if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strpbrk(query2,";/[]\\") ) {
		write_str_www(user, "Illegal email address<br>\n\r", -1);
		goto WEND;
	     }
	if (strstr(query2,"^")) {
	write_str_www(user, "Email cannot have color or hilite codes in them.<br>\n\r", -1);
	goto WEND;
	}
	if (strlen(query2) > EMAIL_LENGTH)
	{
	write_str_www(user, "Email address truncated, but saved<br>\n\r", -1);
	query2[EMAIL_LENGTH-1]=0;
  	}
	strcpy(junk,query2);
	strtolower(junk);

	if (strstr(junk,"whitehouse.gov"))
	{
	write_str_www(user, "Email address not valid.<br>\n\r", -1);
	goto WEND;
	}

	 if (strlen(junk)) {
	  if ((!strstr(junk,".") || !strstr(junk,"@")) && strcmp(junk,DEF_EMAIL)) {
	  write_str_www(user, "Email address not valid.<br>\n\r", -1);
	  goto WEND;
	  }
	  strcpy(t_ustr.email_addr,query2);
	 }
	 else {
	  strcpy(t_ustr.email_addr,DEF_EMAIL);
	 }
	}
	} /* end of email_addr else */
	query2=NULL;
	/* HOMEPAGE */
	if ((query2=get_value(wwwport[user].keypair,"homepage"))==NULL) {
	write_str_www(user, "You must type a homepage address!<br>\n\r", -1);
	goto WEND;
	} /* end of homepage if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strstr(query2,"^")) {
	write_str_www(user, "Homepage cannot have color or hilite codes in it.<br>\n\r", -1);
	goto WEND;
	}
	if (strlen(query2) > HOME_LEN)
	{
	write_str_www(user, "Home page address truncated, but saved<br>\n\r", -1);
	query2[HOME_LEN-1]=0;
  	}
	if (!strlen(query2)) strcpy(t_ustr.homepage,DEF_URL);
	else strcpy(t_ustr.homepage,query2);
	}
	} /* end of homepage else */
	query2=NULL;
	/* WEB PICTURE ADDRESS */
	if ((query2=get_value(wwwport[user].keypair,"webpic"))==NULL) {
	write_str_www(user, "You must type a webpic address!<br>\n\r", -1);
	goto WEND;
	} /* end of webpic if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strstr(query2,"^")) {
	write_str_www(user, "Picture URL address cannot have color or hilite codes in it.<br>\n\r", -1);
	goto WEND;
	}
	if (strlen(query2) > HOME_LEN)
	{
	write_str_www(user, "Picture URL address truncated, but saved<br>\n\r", -1);
	query2[HOME_LEN-1]=0;
  	}
	if (!strlen(query2)) strcpy(t_ustr.webpic,DEF_PICURL);
	else strcpy(t_ustr.webpic,query2);
	}
	} /* end of webpic else */
	query2=NULL;
	/* ENTER MESSAGE */
	if ((query2=get_value(wwwport[user].keypair,"entermsg"))==NULL) {
	write_str_www(user, "You must type an entermessage!<br>\n\r", -1);
	goto WEND;
	} /* end of enterm if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".entermsg")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".entermsg"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);

	if (strlen(query2) > MAX_ENTERM)
	{
	write_str_www(user, "Entermessage too long!<br>\n\r", -1);
	goto WEND;
  	}
	if (!strlen(query2)) strcpy(t_ustr.entermsg,DEF_ENTER);
	else {
	 strcpy(t_ustr.entermsg,query2);
/*	 strcat(t_ustr.entermsg,"@@"); */
	 }
	}
	} /* end of entermsg else */
	query2=NULL;
	/* EXIT MESSAGE */
	if ((query2=get_value(wwwport[user].keypair,"exitmsg"))==NULL) {
	write_str_www(user, "You must type an exitmessage!<br>\n\r", -1);
	goto WEND;
	} /* end of exitm if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".exitmsg")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".exitmsg"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);

	if (strlen(query2) > MAX_EXITM)
	{
	write_str_www(user, "Exitmessage too long!<br>\n\r", -1);
	goto WEND;
  	}
	if (!strlen(query2)) strcpy(t_ustr.exitmsg,DEF_EXIT);
	else {
	 strcpy(t_ustr.exitmsg,query2);
/*	 strcat(t_ustr.exitmsg,"@@"); */
	 }
	}
	} /* end of entermsg else */
	query2=NULL;
	/* SUCC MESSAGE */
	if ((query2=get_value(wwwport[user].keypair,"succ"))==NULL) {
	write_str_www(user, "You must type a success message!<br>\n\r", -1);
	goto WEND;
	} /* end of succ if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".succ")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".succ"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);

	if (strlen(query2) > MAX_ENTERM)
	{
	write_str_www(user, "Success too long!<br>\n\r", -1);
	goto WEND;
  	}
	if (!strlen(query2)) strcpy(t_ustr.succ,"");
	else {
	 strcpy(t_ustr.succ,query2);
/*	 strcpy(t_ustr.succ,"@@"); */
	 }
	}
	} /* end of succ else */
	query2=NULL;
	/* FAIL MESSAGE */
	if ((query2=get_value(wwwport[user].keypair,"fail"))==NULL) {
	write_str_www(user, "You must type a fail message!<br>\n\r", -1);
	goto WEND;
	} /* end of succ if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".fail")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".fail"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);

	if (strlen(query2) > MAX_ENTERM)
	{
	write_str_www(user, "Fail too long!<br>\n\r", -1);
	goto WEND;
  	}
	if (!strlen(query2)) strcpy(t_ustr.fail,"");
	else {
	 strcpy(t_ustr.fail,query2);
/*	 strcpy(t_ustr.fail,"@@"); */
	 }
	}
	} /* end of fail else */
	query2=NULL;
	/* HOME ROOM */
	if ((query2=get_value(wwwport[user].keypair,"home_room"))==NULL) {
	write_str_www(user, "You must type a home room!<br>\n\r", -1);
	goto WEND;
	} /* end of home room if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".set")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".set"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);
	if (strstr(query2,"^")) {
	write_str_www(user, "Room cannot have color or hilite codes in it.<br>\n\r", -1);
	goto WEND;
	}
	if (strlen(query2) > NAME_LEN)
	{
	write_str_www(user, "Room name length too long.<br>\n\r", -1);
	goto WEND;
  	}
	if ( (!strcmp(query2,HEAD_ROOM)) || (!strcmp(query2,ARREST_ROOM)) || (!strcmp(query2,"sky_palace")) ) {
	write_str_www(user, "You cannot make that room your home.<br>\n\r", -1);
	goto WEND;
	}
   /*--------------------*/
   /* see if area exists */
   /*--------------------*/
   found = FALSE;
   for (new_area=0; new_area < NUM_AREAS; ++new_area)
    {
     if (!strcmp(astr[new_area].name, query2) )
       {
         found = TRUE;
         break;
       }
    }

   if (!found)
     {
      write_str_www(user,NO_ROOM,-1);
      goto WEND;
     }
/*----------------------------------------------*/
/* check for secure room                        */
/*----------------------------------------------*/
if (astr[new_area].hidden && t_ustr.security[new_area] == 'N')
  {
   write_str_www(user,NO_ROOM,-1);
   goto WEND;
  }
/*-------------------------------------------------------------------*/
/* see if new room has exits, if not and not wizard, no set possible */
/*-------------------------------------------------------------------*/
found = TRUE;
if ( (!strlen(astr[new_area].move)) || (!strcmp(astr[new_area].move,"*")) )
    {
     found = FALSE;
    }
/*--------------------------------------------------------------*/
/* anyone above a 3 can teleport to non-connected rooms         */
/*--------------------------------------------------------------*/

if (!found)
  {
    if ((t_ustr.tempsuper < TELEP_LEVEL) && (t_ustr.security[new_area] == 'N'))
      {
       write_str_www(user,"You cannot make a non-connected room your home.",-1);
       goto WEND;
      }
  }

	if (!strlen(query2)) strcpy(t_ustr.home_room,astr[INIT_ROOM].name);
	else strcpy(t_ustr.home_room,astr[new_area].name);
	}
	} /* end of home room else */
	query2=NULL;
	/* PROFILE */
	if ((query2=get_value(wwwport[user].keypair,"profile"))==NULL) {
	write_str_www(user, "You must type a profile!<br>\n\r", -1);
	goto WEND;
	} /* end of profile if */
	else {
 	for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_revoke(t_ustr.revokes[z])) continue;
        if (strip_com(t_ustr.revokes[z])==get_com_num_plain(".entpro")) { gravoked=1; break; }
   	}
	if (gravoked || (t_ustr.super < get_rank(".entpro"))) {
	gravoked=0;
	}
	else {
	plustospace(query2);
	unescape_url(query2);

	sprintf(filename,"%s/%s",PRO_DIR,t_ustr.name);
	if (!(fp=fopen(filename,"w"))) {
        write_str_www(user,"Couldn't open profile file to write to!<br>\n\r", -1);
        write_log(ERRLOG,YESTIME,"WWWMENU: Couldn't open file(w) \"%s\" in external_menu! %s\n",filename,get_error());
        goto WEND;
        }
	z=0;
	while (*query2) {
	if (*query2=='\n') z++;
	putc(*query2,fp);
	if (z==PRO_LINES) break;
	query2++;
	}
	fclose(fp);
	}
	} /* end of profileelse */
	query2=NULL;


	write_user(t_ustr.name);
	write_str_www(user, "<font size=\"+2\">Info Saved</font><br>\n\r", -1);
	} /* end of change else */
} /* end of prefs if */

WEND:

	/* Write our footer file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[3]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWMENU: Couldn't open file(r) \"%s\" in external_menu! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }

	sprintf(mess,"<br><b><center>This web page dynamically generated on %s</b></center><br>",ctime(&tm));
	write_str_www(user, mess, -1);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);

}

/*** prints list of all users, lettered group users, or specific user data ***/
/* photofilter has effect for modes 0, 1, and 4 */
void external_users(int user, int mode, char *query, int photofilter)
{
	int u,on_now=0,num=0,num_rank=0;
	char letter;
	char small_buffer[64];
	char filename[FILE_NAME_LEN];
	char filename2[FILE_NAME_LEN];
	struct dirent *dp;
	time_t tm;
	time_t tm_then;
	FILE *fp;
	FILE *wfp;
	DIR *dirp;

	time(&tm);

/*
	if (wwwport[user].method==1) {
	  strcpy(username,strip_user(query));
	  strcpy(password,strip_user(query));
	  }
*/

        write_str_www(user, "Content-type: text/html\n\n", -1);
	write_str_www(user, "<HTML>\n\r", -1);
	write_str_www(user, "<HEAD>\n\r", -1);

	if (mode==0)
		sprintf(mess, "<TITLE>All users on %s</title>\n\r",SYSTEM_NAME);
	else if (mode==1)
		sprintf(mess, "<TITLE>\"%s\" users on %s</title>\n\r",query,SYSTEM_NAME);
	else if (mode==2) {
		sprintf(mess, "<TITLE>User info for %s on %s</title>\n\r",query,SYSTEM_NAME);
		}
	else if (mode==3) {
		sprintf(mess, "<TITLE>%s\'s user page</title>\n\r",SYSTEM_NAME);
		}
	else if (mode==4) {
		num_rank=atoi(query);
		sprintf(mess, "<TITLE>%s of %s</title>\n\r",ranks[num_rank].sname,SYSTEM_NAME);
/*
		sprintf(mess, "<TITLE>%ss of %s</title>\n\r",ranks[(int)query[0]].sname,SYSTEM_NAME);
*/
		}


	write_str_www(user, mess, -1);
	write_str_www(user, "</HEAD>\n\r", -1);

if (mode != 3) {
	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 sprintf(mess,"<BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 sprintf(mess,"<BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);

	write_str_www(user, mess, -1);

/* Write our header file
	sprintf(mess,"%s/%s",WEBFILES,web_opts[2]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(r) \"%s\" in external_users! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }
*/
} /* end of !mode is 3 */

	switch(mode) {
	case 0:
		  sprintf(mess,"<font color=\"%s\"><h3>All users<br>%s</h3></font>\n\r",web_opts[8],photofilter==1?"(with photos)":"");
		  write_str_www(user, mess, -1);
		  break;
	case 1:
		  sprintf(mess,"<font color=\"%s\"><h3>\"%s\" users<br>%s</h3></font>\n\r",web_opts[8],query,photofilter==1?"(with photos)":"");
		  write_str_www(user, mess, -1);
		  break;
	case 2:
		  break;
	case 3:
		  break;
	case 4:
		  sprintf(mess,"<font color=\"%s\"><h3>%ss<br>%s</h3></font>\n\r",web_opts[8],ranks[num_rank].sname,photofilter==1?"(with photos)":"");
		  write_str_www(user, mess, -1);
		  break;
	default:  break;
	} /* end of switch */

/* Wont do switch() here because cases get to hairy, esp. with only 80 columns to see */
if (mode==0) {
 /* ALL USERS */
 sprintf(mess,"%s",USERDIR);
 strncpy(filename,mess,FILE_NAME_LEN);
 
 dirp=opendir((char *)filename);
  
 if (dirp == NULL)
   {  
	sprintf(mess,"Can't open directory \"%s\" for external_users(1)! %s\n",filename,get_error());
	write_str_www(user, mess, -1);
	write_log(ERRLOG,YESTIME,mess);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);
      return;
   }

write_str_www(user, "<base target=\"info\">\n", -1);

 while ((dp = readdir(dirp)) != NULL) 
   { 

    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;

	read_user(small_buffer);

if (strstr(t_ustr.webpic,"://")) {
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a>&nbsp;&nbsp;<img align=\"absmiddle\" src=\"pic.gif\" border=0 alt=\"User has a picture\"><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
else {
    if (photofilter==1) continue;
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
    strcpy(mess, convert_color(mess));
    write_str_www(user, mess, -1);
    small_buffer[0]=0;
    num++;
   } /* end of directory while */
 (void)closedir(dirp);

    sprintf(mess,"<br><br><b>%d user%s exist%s</b><br>",num,num==1?"":"s",num==1?"s":"");
    write_str_www(user,mess, -1);
    num=0;


  } /* end of main if */
else if (mode==1) {
 /* ALL USERS THAT START WITH LETTER */
 sprintf(mess,"%s",USERDIR);
 strncpy(filename,mess,FILE_NAME_LEN);
 
 dirp=opendir((char *)filename);
  
 if (dirp == NULL)
   {  
	sprintf(mess,"Can't open directory \"%s\" for external_users(2)! %s\n",filename,get_error());
	write_str_www(user, mess, -1);
	write_log(ERRLOG,YESTIME,mess);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);
      return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 

    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;
        if (small_buffer[0] != tolower((int)query[0]))
         continue;	

	read_user(small_buffer);

if (strstr(t_ustr.webpic,"://")) {
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a>&nbsp;&nbsp;<img align=\"absmiddle\" src=\"pic.gif\" border=0 alt=\"User has a picture\"><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
else {
    if (photofilter==1) continue;
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
num++;
    strcpy(mess, convert_color(mess));
    write_str_www(user, mess, -1);
    small_buffer[0]=0;
   } /* end of directory while */
 (void)closedir(dirp);

if (!num)
    sprintf(mess,"<br><b>No users</b><br>\n\r");
else
    sprintf(mess,"<br><b>%d user%s</b><br>\n\r",num,num==1?"":"s");
write_str_www(user, mess, -1);
  } /* end of else if */
else if (mode==2) {
	/* SPECIFIC USER */
	strtolower(query);
	if (!check_for_user(query)) {
	  sprintf(mess,"<h3>%s</h3><br>",NO_USER_STR);
	  write_str_www(user, mess, -1);
	  }
	else {
	  read_user(query);
	  sprintf(mess,"<font color=\"%s\"><h1>%s</h1></font>\n\r",web_opts[8],t_ustr.say_name);
	  strcpy(mess, convert_color(mess));
	  write_str_www(user, mess, -1);

	  write_str_www(user,"<table width=100% border=0><tr>\n", -1);
	  write_str_www(user,"<td valign=top width=50%>\n", -1);

	  write_str_www(user,"<table valign=top border=0 cellspacing=10><tr valign=top>\n", -1);
	  sprintf(mess,"<td align=left>With %s since</td><td align=left>%s</td>",SYSTEM_NAME,t_ustr.creation);
	  write_str_www(user, mess, -1);
	  write_str_www(user,"</tr>\n", -1);

		/* Is user online? */
		if ((u = get_user_num_exact(query,-1)) != -1) {
		  on_now = 1;
		  }

	if (on_now) {
	  sprintf(mess,"<tr><td align=left>Online Status</td><td align=left><font color=\"%s\"><b><blink>IS ONLINE RIGHT NOW</b></blink></font></td>",web_opts[10]);
	  write_str_www(user,mess, -1);
	  }
	else {
	  tm_then=((time_t) t_ustr.rawtime);
	  sprintf(mess,"<tr><td align=left>Online Status</td><td align=left>On %s ago</td>",converttime((long)((tm-tm_then)/60)));
	  write_str_www(user, mess, -1);
	  }
	  write_str_www(user,"</tr>\n", -1);
	  sprintf(mess,"<tr><td align=left>Gender</td><td align=left>%s</td>",strip_color(t_ustr.sex));
	  write_str_www(user, mess, -1);
	  write_str_www(user,"</tr>\n", -1);
	  sprintf(mess,"<tr><td align=left>ICQ #</td><td align=left><a href=\"http://wwp.icq.com/%s\" target=\"_blank\">%s</a></td>",strip_color(t_ustr.icq),strip_color(t_ustr.icq));
	  write_str_www(user, mess, -1);
	  write_str_www(user,"</tr>\n", -1);

	  if ((!t_ustr.semail) &&
		strstr(t_ustr.email_addr,"@") &&
		strstr(t_ustr.email_addr,".")) {
		sprintf(mess,"<tr><td align=left>Email</td><td align=left><a href=\"mailto:%s\">%s</a></td>",t_ustr.email_addr,t_ustr.email_addr);
		write_str_www(user, mess, -1);
		write_str_www(user,"</tr>\n", -1);
                        }

	  if ((!strstr(t_ustr.homepage,DEF_URL)) && (strstr(t_ustr.homepage,"/"))) {
		if (strstr(t_ustr.homepage,"://")) {
		sprintf(mess,"<tr><td align=left>Homepage</td><td align=left><a href=\"%s\" target=\"_blank\">%s</a></td>",t_ustr.homepage,t_ustr.homepage);
		write_str_www(user, mess, -1);
		write_str_www(user,"</tr>\n", -1);
                           }
	    }

	  sprintf(filename,"%s/%s",PRO_DIR,query);
	  write_str_www(user,"<tr><td colspan=2>\n", -1);

        if (!(fp=fopen(filename,"r"))) {
		write_str_www(user,"No profile. Sorry :)<br>", -1);
	  }
        else {
                while (fgets(mess,256,fp) != NULL) {
                write_str_www(user, convert_color(mess), -1);
		write_str_www(user,"<br>", -1);
                }
                fclose(fp);
             }

	  write_str_www(user,"</td></tr>\n", -1);

	  write_str_www(user,"</table>\n", -1);
	  
	  write_str_www(user,"</td><td width=50%>\n", -1);
	  if (strstr(t_ustr.webpic,"://")) {
	   sprintf(mess,"<img src=\"%s\" border=0 alt=\"%s\'s Picture\">",t_ustr.webpic,strip_color(t_ustr.say_name));
	   write_str_www(user, mess, -1);
	  }
	  else write_str_www(user,"<font size=+1><b><center>No picture available</center></b></font>", -1);

	  write_str_www(user,"</td></tr></table>\n", -1);
	  write_str_www(user,"<br>\n", -1);
	  } /* end of check_for_user else */
  } /* end of else if */
else if (mode==3) {
/* USER SEARCH PAGE */

/* write out static files to reference through our frames */
	/* top frame */
        sprintf(mess,"%s/.header.html",WEBFILES);
        strncpy(filename,mess,FILE_NAME_LEN);
        if (!(wfp=fopen(filename,"w"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(w) \"%s\" in external_users! %s\n",filename,get_error());
		web_error(user,NO_HEADER,WEB_FORBIDDEN);
		return;
	}
        sprintf(mess,"%s/%s",WEBFILES,web_opts[2]);
        strncpy(filename2,mess,FILE_NAME_LEN);
        if (!(fp=fopen(filename2,"r"))) {
		fclose(wfp);
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(r) \"%s\" in external_users! %s\n",filename2,get_error());
		return;
	}
	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 fprintf(wfp,"<HTML><BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 fprintf(wfp,"<HTML><BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
		while (fgets(mess,256,fp) != NULL) {
		fputs(mess,wfp);
		}
	fputs("</BODY></HTML>\n",wfp);
	fclose(fp);
	fclose(wfp);
	/* left frame */
        sprintf(mess,"%s/.left_fr.html",WEBFILES);
        strncpy(filename,mess,FILE_NAME_LEN);
        if (!(wfp=fopen(filename,"w"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(w) \"%s\" in external_users! %s\n",filename,get_error());
		web_error(user,NO_HEADER,WEB_FORBIDDEN);
		return;
	}
        sprintf(mess,"%s/left_fr.html",WEBFILES);
        strncpy(filename2,mess,FILE_NAME_LEN);
        if (!(fp=fopen(filename2,"r"))) {
		fclose(wfp);
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(r) \"%s\" in external_users! %s\n",filename2,get_error());
		return;
	}
	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 fprintf(wfp,"<HTML><BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 fprintf(wfp,"<HTML><BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
		while (fgets(mess,256,fp) != NULL) {
		fputs(mess,wfp);
		}
	fputs("</BODY></HTML>\n",wfp);
	fclose(fp);
	fclose(wfp);
	/* right frame */
        sprintf(mess,"%s/.right_fr.html",WEBFILES);
        strncpy(filename,mess,FILE_NAME_LEN);
        if (!(wfp=fopen(filename,"w"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(w) \"%s\" in external_users! %s\n",filename,get_error());
		web_error(user,NO_HEADER,WEB_FORBIDDEN);
		return;
	}
        sprintf(mess,"%s/right_fr.html",WEBFILES);
        strncpy(filename2,mess,FILE_NAME_LEN);
        if (!(fp=fopen(filename2,"r"))) {
		fclose(wfp);
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(r) \"%s\" in external_users! %s\n",filename2,get_error());
		return;
	}
	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 fprintf(wfp,"<HTML><BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 fprintf(wfp,"<HTML><BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
		while (fgets(mess,256,fp) != NULL) {
		fputs(mess,wfp);
		}
	fputs("</BODY></HTML>\n",wfp);
	fclose(fp);
	fclose(wfp);
	/* dynamic footer frame - dynamic content */
        sprintf(mess,"%s/.footer.html",WEBFILES);
        strncpy(filename,mess,FILE_NAME_LEN);
        if (!(wfp=fopen(filename,"w"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(w) \"%s\" in external_users! %s\n",filename,get_error());
		web_error(user,NO_HEADER,WEB_FORBIDDEN);
		return;
	}
	if ((web_opts[4][0]=='#') || (!strstr(web_opts[4],".")))
	 fprintf(wfp,"<HTML><BODY BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);
	else
	 fprintf(wfp,"<HTML><BODY BACKGROUND=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\">\n\r",web_opts[4],web_opts[5],web_opts[6],web_opts[7]);

	/* dynamic content generated here */

fputs("<table border=\"0\" width=\"100%\" cellpadding=\"1\" cellspacing=\"3\"><tr>",wfp);
sprintf(mess, "<FORM METHOD=\"POST\" TARGET=\"results\" ACTION=\"http://%s:%d/_users\">",thishost,PORT+WWW_OFFSET);
fputs(mess,wfp);
fputs("<td valign=top>",wfp);
fputs("<font size=+1><b>List these users:</b></font> &nbsp;<SELECT NAME=\"search\">\n\r",wfp);
fputs("<OPTION VALUE=\"\" SELECTED>ALL</OPTION>\n",wfp);
/* ranks */
for (u=0;u<MAX_LEVEL+1;u++) {
sprintf(mess,"<OPTION VALUE=\"%d\">%ss</OPTION>",u,ranks[u].sname);
fputs(mess,wfp);
} /* end of for */
/* letters */
for (letter='A';isalpha((int)letter);letter++) {
sprintf(mess,"<OPTION VALUE=\"%c\">%c users</OPTION>\n",letter,letter);
fputs(mess,wfp);
} /* end of for */
fputs("</SELECT><br>\n",wfp);
fputs("<center>Search only users with pics? <INPUT TYPE=\"checkbox\" NAME=\"photofilter\" VALUE=\"yes\"></INPUT></center></td>",wfp);
fputs("<td valign=center><INPUT TYPE=\"SUBMIT\" VALUE=\"Search\"></td></FORM>",wfp);
sprintf(mess, "<FORM METHOD=\"POST\" TARGET=\"info\" ACTION=\"http://%s:%d/_users\">",thishost,PORT+WWW_OFFSET);
fputs(mess,wfp);
fputs("<td valign=center align=right>",wfp);
sprintf(mess,"<font size=+1><b>Find specific user:</b></font> &nbsp;<INPUT TYPE=\"TEXT\" NAME=\"search\" SIZE=%d MAXLENGTH=%d>\n\r",NAME_LEN+1,NAME_LEN+1);
fputs(mess,wfp);
fputs("<INPUT TYPE=\"SUBMIT\" VALUE=\"Search\">",wfp);
fputs("</td></FORM>",wfp);

fputs("</tr></table>",wfp);

/*
sprintf(mess, "<FORM METHOD=\"GET\" ACTION=\"http://%s:%d/_users\">",thishost,PORT+WWW_OFFSET);
write_str_www(user, mess, -1);
sprintf(mess,"<font size=+1><b>Find user</b></font><br><INPUT TYPE=\"TEXT\" NAME=\"search\" SIZE=%d>",NAME_LEN+1);
write_str_www(user, mess, -1);
write_str_www(user, "&nbsp;&nbsp;<INPUT TYPE=\"SUBMIT\" VALUE=\"Search\">", -1);
write_str_www(user, "</FORM>", -1);
*/

	/* end of dynamic content */
	fputs("</BODY></HTML>\n",wfp);
	fclose(wfp);

	/* write out main page and framesets to user */

write_str_www(user,"<FRAMESET ROWS=\"18%,*,12%\" FRAMEBORDER=0 BORDER=0 FRAMESPACING=0>\n", -1);
sprintf(mess,"<FRAME SRC=\"http://%s:%d/.header.html\" NAME=\"header\" MARGINWIDTH=0 MARGINHEIGHT=0 SCROLLING=OFF>\n",thishost,PORT+WWW_OFFSET);
write_str_www(user,mess, -1);
write_str_www(user,"<FRAMESET COLS=\"15%,85%\" FRAMEBORDER=0 BORDER=0 FRAMESPACING=0>\n", -1);
sprintf(mess,"<FRAME SRC=\"http://%s:%d/.left_fr.html\" NAME=\"results\" MARGINWIDTH=0 MARGINHEIGHT=0 SCROLLING=AUTO>\n",thishost,PORT+WWW_OFFSET);
write_str_www(user,mess, -1);
sprintf(mess,"<FRAME SRC=\"http://%s:%d/.right_fr.html\" NAME=\"info\" MARGINWIDTH=0 MARGINHEIGHT=0 SCROLLING=AUTO>\n",thishost,PORT+WWW_OFFSET);
write_str_www(user,mess, -1);
write_str_www(user,"</FRAMESET>\n", -1);
sprintf(mess,"<FRAME SRC=\"http://%s:%d/.footer.html\" NAME=\"footer\" MARGINWIDTH=0 MARGINHEIGHT=0 SCROLLING=OFF>\n",thishost,PORT+WWW_OFFSET);
write_str_www(user,mess, -1);
write_str_www(user,"</FRAMESET>\n", -1);
write_str_www(user,"</HTML>\n", -1);

  } /* end of else if */
else if (mode==4) {
 /* ALL USERS OF CERTAIN RANK */
 sprintf(mess,"%s",USERDIR);
 strncpy(filename,mess,FILE_NAME_LEN);
 
 dirp=opendir((char *)filename);
  
 if (dirp == NULL)
   {  
	sprintf(mess,"Can't open directory \"%s\" for external_users(3)! %s\n",filename,get_error());
	write_str_www(user, mess, -1);
	write_log(ERRLOG,YESTIME,mess);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);
      return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 

    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;

	read_user(small_buffer);

        if (t_ustr.super != num_rank)
         continue;	

if (strstr(t_ustr.webpic,"://")) {
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a>&nbsp;&nbsp;<img align=\"absmiddle\" src=\"pic.gif\" border=0 alt=\"User has a picture\"><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
else {
    if (photofilter==1) continue;
    sprintf(mess,"<a target=\"info\" href=\"http://%s:%d/_users?search=%s\">%s</a><br>\n\r",thishost,
		PORT+WWW_OFFSET,strip_color(t_ustr.say_name),t_ustr.say_name);
    }
num++;
    strcpy(mess, convert_color(mess));
    write_str_www(user, mess, -1);
    small_buffer[0]=0;
   } /* end of directory while */
 (void)closedir(dirp);

if (!num)
    sprintf(mess,"<br><b>No %ss</b><br>\n\r",ranks[num_rank].sname);
else
    sprintf(mess,"<br><b>%d %s%s</b><br>\n\r",num,ranks[num_rank].sname,num==1?"":"s");
write_str_www(user, mess, -1);
  } /* end of else if */


if (mode != 3) {
	/* Write our footer file */
	sprintf(mess,"%s/%s",WEBFILES,web_opts[3]);
	strncpy(filename,mess,FILE_NAME_LEN);
	if (!(fp=fopen(filename,"r"))) {
		write_log(ERRLOG,YESTIME,"WWWUSERS: Couldn't open file(r) \"%s\" in external_users! %s\n",filename,get_error());
	}
	else {
		while (fgets(mess,256,fp) != NULL) {
		write_str_www(user, mess, -1);
		}
		fclose(fp);
	     }

	sprintf(mess,"<br><b><center>This web page dynamically generated on %s</b></center><br>",ctime(&tm));
	write_str_www(user, mess, -1);
	write_str_www(user, "</body>\n\r", -1);
	write_str_www(user, "</html>\n\r\n\r", -1);
} /* end of !mode is 3 */

}

void web_error(int user, int header, int mode)
{
int size=0;
char message[350];
char output[ARR_SIZE];
time_t tm;

time(&tm);

switch(mode) {
	case 0: strcpy(message,""); break;
	case BAD_REQUEST: strcpy(message,"HTTP/1.0 400 Bad request\n"); break;
	case NOT_FOUND: strcpy(message,"HTTP/1.0 404 Not found\n"); break;
	case WEB_FORBIDDEN: strcpy(message,"HTTP/1.0 403 Forbidden\n"); break;
	case WEB_SERVER_ERROR: strcpy(message,"HTTP/1.0 500 Internal Server Error\n"); break;
	default: strcpy(message,"HTTP/1.0 500 Unknown\n"); break;
  } /* end of switch */

if (header==YES_HEADER) {
	write_str_www(user, message, -1);
	sprintf(mess, "Server: %s/1.0\n",SYSTEM_NAME);
	write_str_www(user, mess, -1);
	sprintf(mess,"Date: %s",ctime(&tm));
	write_str_www(user, mess, -1);
	}

if (mode==0) { }
else if (mode==BAD_REQUEST) {
	strcpy(output,"\n\rYour browser sent a message this server could not understand.");
  }
else if (mode==NOT_FOUND) {
	strcpy(output,"<TITLE>Not Found</TITLE><H1>Not Found</H1> The requested object does not exist on this server.");
  }
else if (mode==WEB_FORBIDDEN) {
	strcpy(output,"<TITLE>Forbidden</TITLE><H1>Forbidden</H1> You don't have permission to access that on this server.");
  }
else if (mode==WEB_SERVER_ERROR) {
	strcpy(output,"<TITLE>Internal Server Error</TITLE><H1>Internal Server Error</H1> An internal server has occured.");
  }

sprintf(mess,"<br><br><i>If you feel you've reached this page in error, please contact <a href=\"mailto:%s\">%s</a></i><br>\n",SYSTEM_EMAIL,SYSTEM_EMAIL);
strcat(output,mess);
size=strlen(output);

if ((header==YES_HEADER) && mode) {
	sprintf(mess,"Content-length: %d\n",size);
        write_str_www(user, mess, -1);
        write_str_www(user, "Content-type: text/html\n\n", -1);
	}

write_str_www(user, output, -1);

}


/* convert encoded vals */
char x2c(char *what) {
register char digit;

digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
digit *= 16;
digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
return (digit);
}

/* unescapes the url */
void unescape_url(char *url) {
register int x,y;

for (x=0,y=0;url[y];++x,++y) {
  if ((url[x] = url[y]) == '%') {
    url[x] = x2c(&url[y+1]);
    y+=2;
    }
}
url[x]='\0';
}

void plustospace(char *str)
{
int i=0;

for (i=0;i<strlen(str);++i) {
        if (str[i]=='+') str[i]=' ';
        }
        
}

