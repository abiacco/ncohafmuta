#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/telnet.h>
*/

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

unsigned char off_seq[4]       = {IAC, WILL, TELOPT_ECHO, '\0'};
unsigned char off_seq_cr[5]    = {IAC, WILL, TELOPT_ECHO, '\n',0};
unsigned char on_seq[4]        = {IAC, WONT, TELOPT_ECHO, '\0'};
unsigned char on_seq_cr[5]     = {IAC, WONT, TELOPT_ECHO, '\n', 0};
/*
unsigned char on_seq2[5]       = {IAC, DO,   TELOPT_ECHO, '\n', 0};
*/
unsigned char eor_seq[4]       = {IAC, WILL, TELOPT_EOR, '\0'};
unsigned char eor_seq2[3]      = {IAC, EOR, '\0'};
unsigned char go_ahead_str[2]  = {IAC, GA};
/* unsigned char go_ahead_str[3]  = {IAC, GA, '\0'}; */
unsigned char do_ttype[4]      = {IAC, DO, TELOPT_TTYPE, '\0'};
unsigned char ask_ttype[]      = {IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE, 0};
/* unsigned char neg_tuid[]   = {IAC, SB, TELOPT_TUID,    IAC, SE, 0}; */
/* unsigned char neg_tlocl[]  = {IAC, SB, TELOPT_TTYLOC,  IAC, SE, 0}; */

/* Telnet option definitions if the OS doesn't define them */
#if !defined(TELOPT_NEW_ENVIRON)
#define TELOPT_NEW_ENVIRON      39
#define IS                      0
#define SEND                    1
#define INFO                    2
#define VAR                     0
#define VALUE                   1
#define ESC                     2
#define USERVAR                 3
#endif
#if !defined(TELQUAL_IS)
#define TELQUAL_IS		0
#endif
#if !defined(TELQUAL_SEND)
#define TELQUAL_SEND		1
#endif

/*----------------------------------------------*/
/* fix for telnet ctrl-c and ctrl-d *arctic9*   */
/*----------------------------------------------*/
void will_time_mark(int user)
{  
char seq[4];

 sprintf(seq,"%c%c%c", IAC, WILL, TELOPT_TM);
 write_str(user,seq);
}

/*----------------------------------------*/
/* send telnet control iac echo-off       */
/*----------------------------------------*/
void telnet_echo_off(int user)
{
 if (ustr[user].passhid == 1)
	write_raw(user, off_seq, 4);
}

/*----------------------------------------*/
/* send telnet control iac echo-on        */
/*----------------------------------------*/
void telnet_echo_on(int user)
{
 if (ustr[user].passhid == 1) {
	if (ustr[user].promptseq > 0)
	    write_raw(user, on_seq, 4);
	else
	    write_raw(user, on_seq_cr, 5);
  }
}

/*----------------------------------------*/
/* send telnet control iac send user id   */
/*----------------------------------------*/
void telnet_ask_tuid(int user)
{
 /*  write_raw(user, neg_tuid, 6); */
}

/*----------------------------------------*/
/* Negotiate a terminal type              */
/*----------------------------------------*/
void telnet_neg_ttype(int user, int mode)
{

   if (!mode)
    write_raw(user, do_ttype, 4);
   else if (mode==1)
    write_raw(user, ask_ttype, 6);
}

/*----------------------------------------*/
/* send telnet control iac will eor       */
/*----------------------------------------*/
void telnet_ask_eor(int user)
{
    write_raw(user, eor_seq, 4);
}

/*----------------------------------------*/
/* send telnet control iac eor            */
/*----------------------------------------*/
void telnet_write_eor(int user)
{
	if (ustr[user].promptseq==1) {
		write_raw(user, eor_seq2, 3);
#if defined(IAC_DEBUG)
		write_log(DEBUGLOG,YESTIME,"writing EOR for prompts\n");
#endif
	}
	else if (ustr[user].promptseq==0) {
		write_raw(user, go_ahead_str, 2);
#if defined(IAC_DEBUG)
		write_log(DEBUGLOG,YESTIME,"writing GA for prompts\n");
#endif
	}
}
    
    
/*------------------------------------------------------------*/
/* this section designed to process telnet commands           */
/* NOTE: to get here, we have already read an IAC command     */
/*------------------------------------------------------------*/
void do_telnet_commands(int user)
{
 unsigned char inpchar[2];
 
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;

#if defined(IAC_DEBUG)
	write_log(DEBUGLOG,YESTIME,"NOTE: got IAC MAIN %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif

 switch(inpchar[0])
   {
    case BREAK:
    case IP:     user_quit(user,1); break;   /* user sent cntrl-c */

    case DONT:   proc_dont(user); break;
    case DO:     proc_do(user);   break;
    case WONT:   proc_wont(user); break;
    case WILL:   proc_will(user); break;

    case SB:     proc_sb(user);   break;
    case GA:     break;
    case EL:     break; /* erase line */
    case EC:     break; /* erase character */
 
    case AYT:    write_hilite(user,"\n[Yes]\n");
                 break; /* are you there? */
    
    case AO:     break; /* abort output */
    case DM:     break; /* urgent data? */
    case SE:     break;
    case EOR:    break;
    
    case 123:    break;   /* no idea what this command is */
    case 0:      break;   /* no idea what this command is */
    
                 
    default:
#if defined(IAC_DEBUG)
	write_log(DEBUGLOG,YESTIME,"NOTE: IAC MAIN %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}
    
void proc_dont(int user)
{
 unsigned char inpchar[2];
    
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;  

#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE: got IAC DONT  %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
    
 switch(inpchar[0])
   {

    case ECHO:		break;
    case TELOPT_ECHO:	break;
    case TELOPT_EOR:	ustr[user].promptseq=0; break;
 
    default:
#if defined(IAC_DEBUG)
	write_log(DEBUGLOG,YESTIME,"NOTE:     IAC DONT  %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}   
 
void proc_do(int user)
{
 unsigned char inpchar[2];
    
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;  

#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE: got IAC DO    %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
    
 switch(inpchar[0])
   {
    case ECHO:		break;
    case TELOPT_ECHO:	break;
 
    case TELOPT_TM:	will_time_mark(user);	break;
    case TELOPT_EOR:	ustr[user].promptseq=2;	break;

    default:
#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE:     IAC DO    %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}
    
void proc_wont(int user)
{
 unsigned char inpchar[2];
    
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;  

#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE: got IAC WONT  %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
    
 switch(inpchar[0])
   {
    case ECHO:   break; 
    case TELOPT_ECHO:	break;
    case TELOPT_TTYPE:	ustr[user].term_type = 2; break; /* the client wont respond */
			/* with a terminal type, set the default */
 
    default:
#if defined(IAC_DEBUG)
       write_log(DEBUGLOG,YESTIME,"NOTE:     IAC WONT  %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}

void proc_will(int user)
{   
 unsigned char inpchar[2];
    
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;  

#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE: got IAC WILL  %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif

 switch(inpchar[0])
   {
    case ECHO:   break; 
    case TELOPT_ECHO:	break;
    case TELOPT_TTYPE:	if (ustr[user].term_type==-1) telnet_neg_ttype(user, 1);
			break;
    
    default:
#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE:     IAC WILL  %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}   


void proc_sb(int user)
{
 unsigned char inpchar[2];
 char terminaltype[10];
 int count=0,found=0;
    
 if (S_READ(ustr[user].sock, inpchar, 1) != 1)
    return;  

terminaltype[0]=0;

#if defined(IAC_DEBUG)
        write_log(DEBUGLOG,YESTIME,"NOTE: got IAC SB  %d/%s (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
    
 switch(inpchar[0])
   {

    case TELOPT_TTYPE:	
			if (S_READ(ustr[user].sock, inpchar, 1) != 1) return; /* IS */
			while ((S_READ(ustr[user].sock, inpchar, 1) == 1) &&
				inpchar[0] != IAC) {
			 if (count < (sizeof(terminaltype) - 1)) {
			  terminaltype[count] = tolower((int)inpchar[0]);
			  count++;
			 }
			}
			terminaltype[count]='\0';
			#if defined(IAC_DEBUG)
			 write_log(DEBUGLOG,YESTIME,"GOT TTYPE %s FROM CLIENT\n",terminaltype);
			#endif
			if (S_READ(ustr[user].sock, inpchar, 1) != 1) return; /* SE */
			if (!strcmp(ustr[user].prev_term_type,terminaltype)) {
			 /* no more term types! We should pick a default */
			 ustr[user].term_type = 2; /* vt100 default */
			}
			else {
			strcpy(ustr[user].prev_term_type,terminaltype);
			 for (count=0;terms[count].jump_vector!=-1;++count) {
			  if (!strcmp(terms[count].name,terminaltype)) {
			   found = 1;
			   break;
			  }
			 } /* for */
			 if (found) {
			  #if defined(IAC_DEBUG)
			   write_log(DEBUGLOG,YESTIME,"FOUND KNOWN TTYPE %s! SETTING\n",terms[count].name);
			  #endif
			  ustr[user].term_type = count;
			 }
			 else {
			  #if defined(IAC_DEBUG)
			   write_log(DEBUGLOG,YESTIME,"TTYPE %s NOT FOUND! ASKING FOR MORE\n",terminaltype);
			  #endif
			  telnet_neg_ttype(user, 1);
			 }
			} /* else */
/*			if (inpchar[0] == IAC) do_telnet_commands(user); */
			break;
 
    default:
#if defined(IAC_DEBUG)
	write_log(DEBUGLOG,YESTIME,"NOTE:     IAC SB  %d/%s not recognized (site:%s)\n",(int)inpchar[0],get_iac_string(inpchar[0]),ustr[user].site);
#endif
       break;
   }
}   


char *get_iac_string(unsigned char code)
{

switch(code) {
    case BREAK:  return "BREAK";
    case IP:     return "IP";
 
    case DONT:   return "DONT";
    case DO:     return "DO";
    case WONT:   return "WONT";
    case WILL:   return "WILL";
 
    case SB:     return "SB";
    case GA:     return "GA";
    case EL:     return "EL";
    case EC:     return "EC";

    case AYT:    return "AYT";

    case AO:     return "AO";
    case DM:     return "DM";
    case SE:     return "SE";
    case EOR:    return "EOR";
        
    case ABORT:  return "ABORT";
    case SUSP:   return "SUSP";
    case xEOF:   return "xEOF";

/* telopts */
    case TELOPT_BINARY:	return "_BINARY";
    case TELOPT_ECHO:   return "_ECHO";
    case TELOPT_SGA:    return "_SGA";
    case TELOPT_STATUS:	return "_STATUS";
    case TELOPT_TM:	return "_TM";
    case TELOPT_RCTE:	return "_RCTE";
    case TELOPT_LOGOUT:	return "_LOGOUT";
    case TELOPT_TTYPE:	return "_TTYPE";
    case TELOPT_EOR:	return "_EOR";
    case TELOPT_NAWS:	return "_NAWS";
    case TELOPT_TSPEED:	return "_TSPEED";
    case TELOPT_LFLOW:	return "_LFLOW";
    case TELOPT_LINEMODE:	return "_LINEMODE";
}

return "UNK";
}
