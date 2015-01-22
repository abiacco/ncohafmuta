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
*/
#include "../hdrfiles/text.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern char mess[ARR_SIZE+25];
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */
extern struct command_struct sys[];

/** quote function for fortunes **/
void quotes(int user)
{
char line[257];
FILE *pp;

 if (!strlen(FORTPROG)) {
  return;
 }

 if (!ustr[user].quote) {
  return;
 }
 else {
  write_str(user,"+---- Quote for this login --------------(type .quote to turn these off)----+");
  sprintf(t_mess,"%s -s 2> /dev/null",FORTPROG);
	if (!(pp=popen(t_mess,"r"))) {
		write_str(user,"No quote.");
		return;
	}
	while (fgets(line,256,pp) != NULL) {
		line[strlen(line)-1]=0;
		write_str(user,line);
	} /* end of while */
  pclose(pp);
 }

return;
}


/*** nerf command for use in nerf arena only ***/
void nerf(int user, char *inpstr)
{
int user2;
int i=0;
char name[ARR_SIZE];
int success;

if (!strlen(inpstr)) {
  write_str(user, "Usage: .nerf <user>");
  return;
  }

sscanf(inpstr, "%s", name);
strtolower(name);

user2=get_user_num(name, user);

    if (user2 == -1 )
     {
      not_signed_on(user,name);
      return;
     }

     if (!user_wants_message(user2,NERFS)) {
	write_str(user,"User is ignoring nerfs right now.");
	return;
	}

        /* See if user2 has user gagged */
        for (i=0; i<NUM_IGN_FLAGS; ++i) {
           if (NERFS==gagged_types[i]) {
		if (!check_gag(user,user2,0)) return;
             }
          }
        i=0;
     
    if (!ustr[user].vis || !ustr[user2].vis) {
       write_str(user,"Both of you must be visible first.");
       return;
       }

    if (ustr[user2].afk) 
     {
    if (ustr[user2].afk == 1) {
      if (!strlen(ustr[user2].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[user2].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[user2].say_name,ustr[user2].afkmsg);
      }
     else {
      if (!strlen(ustr[user2].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[user2].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[user2].say_name,ustr[user2].afkmsg);
      }

       write_str(user,t_mess);
       return;
     }
    
    if (strcmp(ustr[user2].name,name)) {
       write_str(user,"Name of person you are nerfing must be typed in full!");
       return;
       }

if (user==user2) {
  write_str(user, "You aren't supposed to nerf yourself!");
  return;
  }

/* can the person you're nerfing nerf back? */
for (i=0;sys[i].jump_vector!=-1;++i) {
	if (!strcmp(sys[i].command,".nerf")) break;
	}

if (ustr[user2].super < sys[i].su_com) {
  write_str(user,"That user doesn't have the .nerf command yet!");
  return;
  }

/*** change this bit to only use ppl in the nerf room ***/
if ( strcmp(astr[ustr[user].area].name,NERF_ROOM) ) {
           sprintf(t_mess,"You must be in the %s to do nerfs.",NERF_ROOM);
           write_str(user,t_mess);
           return;
        }

       if (ustr[user2].area != ustr[user].area)
         {
           sprintf(t_mess,"%s is not here to nerf with you!",ustr[user2].say_name);
           write_str(user,t_mess);
           return;
         }

if (ustr[user].nerf_shots==0) {
        write_hilite(user, "Your nerf-gun is empty!!!");
        sprintf(t_mess, "%s's nerf-gun clicks as the trigger is pulled - it's empty!!!", ustr[user].say_name);
        writeall_str(t_mess,1,user,0,user,NORM,NERFS,0);
        return;
        }
        
success = (((unsigned short) rand()) > 32000); /* should be # between 0-65535 */
         
if (!success) { /* beep here */
        sprintf(t_mess, "You fire at %s but miss", ustr[user2].say_name);
        write_str(user, t_mess);
        sprintf(t_mess, "-> %s tries to nerf %s, but misses", ustr[user].say_name, ustr[user2].say_name);
        writeall_str(t_mess,1,user,0,user,NORM,NERFS,0);
        }
else {  
        ustr[user2].nerf_energy--;
        if (ustr[user2].nerf_energy == 0) {
                ustr[user].nerf_energy=10;      /* full health to survivor */
                ustr[user].nerf_kills++;
                sprintf(t_mess, "You destroy %s - your health has been restored to FULL\n", ustr[user2].say_name);
                write_hilite(user, t_mess);
                sprintf(t_mess, "%s destroys %s\n", ustr[user].say_name, ustr[user2].say_name);
                writeall_str(t_mess,1,user,0,user,BOLD,NERFS,0);
                write_str(user2, "You have been destroyed. But don't worry - you can log back in and seek revenge");
                sprintf(t_mess, "*** %s was nerfed by %s ***\n", ustr[user2].say_name, ustr[user].say_name);
                writeall_str(t_mess,0,user,0,user,NORM,NERFS,0);
                ustr[user2].nerf_killed++;
                user_quit(user2,1);
                }
        else {
                sprintf(t_mess, "You nerf %s", ustr[user2].say_name);
                write_hilite(user, t_mess);
                sprintf(t_mess, "-> %s nerfs %s\n", ustr[user].say_name, ustr[user2].say_name);
                writeall_str(t_mess,1,user,0,user,BOLD,NERFS,0);
                if (ustr[user2].nerf_energy == 2) {
                        write_str(user2, "You will be destroyed after 2 more hits");
                        sprintf(t_mess, "%s has almost been destroyed", ustr[user2].say_name);
                        writeall_str(t_mess,1,user,0,user,NORM,NERFS,0);
                        }
                }
        }
                
ustr[user].nerf_shots--;
                
return;
}


/*** User to reload his or her nerf gun ***/
void reload(int user)
{
if ( strcmp(astr[ustr[user].area].name,NERF_ROOM) ) {
           sprintf(t_mess,"You must be in the %s before you can reload.",NERF_ROOM);
           write_str(user,t_mess);
           return;
        }
         
if (ustr[user].nerf_shots > 0) {
        sprintf(t_mess, "You have %d rounds left before you can reload",
                ustr[user].nerf_shots);
        write_str(user, t_mess);
        }
else {
        ustr[user].nerf_shots = 5;
        write_str(user, "You reload your nerf-gun with another 5 rounds");
        sprintf(t_mess, "An empty clip bounces on the ground by %s's feet", ustr[user].say_name);
        writeall_str(t_mess,1,user,0,user,NORM,NERFS,0);
        }

return;
}

/*----------------------------------------------------------*/
/* Here is where we begin, my hope to incorporate           */
/* Tic-tac-toe into the talker.                             */
/*----------------------------------------------------------*/
int ttt_is_end(int user)
{
        int i, board[9], draw = 1;
                        
        for (i = 0; i < 9; i++) {
                board[i] = (ustr[user].ttt_board>>(i*2))%4;   
                if (!(board[i]))
                        draw = 0;
        }

        if (board[0] && (((board[0] == board[1]) && (board[0] == board[2])) ||
                        ((board[0] == board[3]) && (board[0] == board[6]))))
                        return board[0];
        if (board[4] && (((board[4] == board[0]) && (board[4] == board[8])) ||
                        ((board[4] == board[1]) && (board[4] == board[7])) ||
                        ((board[4] == board[2]) && (board[4] == board[6])) ||
                        ((board[4] == board[3]) && (board[4] == board[5]))))
                        return board[4];
        if (board[8] && (((board[8] == board[2]) && (board[8] == board[5])) ||
                        ((board[8] == board[7]) && (board[8] == board[6]))))
                        return board[8];
        if (draw)
                return 3;
        
        return 0;
}

void ttt_print_board(int user)
{
        int i, temp, gameover = 0;
        char cells[9];
	char cell1[7];
	char cell2[7];
	char cell3[7];

        if (ustr[user].ttt_opponent == -3) {
                write_str(user,"TTT: EEEK.. seems we've lost your opponent!");
                return; 
        }
        if (((ustr[user].ttt_board)%(1<<17)) != ((ustr[ustr[user].ttt_opponent].ttt_board)%(1<<17))) {
                write_str(user,"TTT: EEEK.. seems like you're playing on different boards!");
                ttt_end_game(user, 0);
                return;
        }

        for (i = 0; i < 9; i++) {
                temp = ((ustr[user].ttt_board)>>(i*2))%4;
                switch (temp) {
                case 0: cells[i] = ' ';
                                break;
                case 1: cells[i] = 'O';
                                break;
                case 2: cells[i] = 'X';
                                break;
                default:
                        write_str(user,"TTT: EEEK.. corrupt board file!!");
                        return;
                }
        }

        if (ustr[user].ttt_board & TTT_MY_MOVE){
                write_str(user," It's your move - with the board as:");
                }
        else if (ustr[ustr[user].ttt_opponent].ttt_board & TTT_MY_MOVE){
                sprintf(mess," %s to move - with the board as:", ustr[ustr[user].ttt_opponent].say_name);
                write_str(user,mess);
                }
        else {
                write_str(user," The final board was:");
                gameover++;
        }
        sprintf(mess,"  ^LY+-------+  +-------+^");
        write_str(user,mess);

        for (i = 0; i < 9; i+=3) {
		if (cells[i]=='O') strcpy(cell1,"^HGO^");
		else if (cells[i]=='X') strcpy(cell1,"^HRX^");
		else if (cells[i]==' ') strcpy(cell1," ");
		if (cells[i+1]=='O') strcpy(cell2,"^HGO^");
		else if (cells[i+1]=='X') strcpy(cell2,"^HRX^");
		else if (cells[i+1]==' ') strcpy(cell2," ");
		if (cells[i+2]=='O') strcpy(cell3,"^HGO^");
		else if (cells[i+2]=='X') strcpy(cell3,"^HRX^");
		else if (cells[i+2]==' ') strcpy(cell3," ");

                sprintf(mess,"  ^LY|^ %s %s %s ^LY|^  ^LY|^ %d %d %d ^LY|^",
                cell1, cell2, cell3, i+1, i+2, i+3);
                write_str(user,mess);
        }

        sprintf(mess,"  ^LY+-------+  +-------+^");
        write_str(user,mess);
        if (!gameover) {
                if (ustr[user].ttt_board & TTT_AM_NOUGHT) {
                        write_str(user,"You are playing Os");
                } else {
                        write_str(user,"You are playing Xs");
                }
        }
        
}


void ttt_end_game(int user, int winner)
{
	int plyr = 1;

	if (ustr[user].ttt_opponent == -3) {
		write_str(user,"TTT: EEEK.. seems we've lost your opponent!");
		return;
	}

	if (!(ustr[user].ttt_board & TTT_AM_NOUGHT))
		plyr++;
	
	if (winner == 3) {
		sprintf(mess,TTT_DRAW, ustr[ustr[user].ttt_opponent].say_name);
		write_str(user,mess);
		sprintf(mess,TTT_DRAW, ustr[user].say_name);
		write_str(ustr[user].ttt_opponent,mess);
	} else if (winner && (winner != plyr)) {
		sprintf(mess,TTT_LOST, ustr[ustr[user].ttt_opponent].say_name);
		write_str(user,mess);
		sprintf(mess,TTT_WON, ustr[user].say_name);
		write_str(ustr[user].ttt_opponent,mess);
		ustr[user].ttt_killed = ustr[user].ttt_killed + 1;
		ustr[ustr[user].ttt_opponent].ttt_kills = ustr[ustr[user].ttt_opponent].ttt_kills + 1;
	} else if (winner) {
		sprintf(mess,TTT_WON, ustr[ustr[user].ttt_opponent].say_name);
		write_str(user,mess);
		sprintf(mess,TTT_LOST, ustr[user].say_name);
		write_str(ustr[user].ttt_opponent,mess);
		ustr[user].ttt_kills = ustr[user].ttt_kills + 1;
		ustr[ustr[user].ttt_opponent].ttt_killed = ustr[ustr[user].ttt_opponent].ttt_killed + 1;
	} else {
		sprintf(mess,TTT_ABORT1, ustr[ustr[user].ttt_opponent].say_name);
		write_str(user,mess);
		sprintf(mess,TTT_ABORT2, ustr[user].say_name);
		write_str(ustr[user].ttt_opponent,mess);
	}

	ustr[user].ttt_board &= ~TTT_MY_MOVE;
	ustr[ustr[user].ttt_opponent].ttt_board &= ~TTT_MY_MOVE;

	if (winner) {
	ttt_print_board(user);
	ttt_print_board(ustr[user].ttt_opponent);
	}
	
	ustr[ustr[user].ttt_opponent].ttt_opponent = -3;
	ustr[user].ttt_opponent = -3;
}

void ttt_new_game(int user, char *str)
{
int p2;
int i=0;
char name[ARR_SIZE];

	if (!strlen(str)) {
		write_str(user,"Usage: .ttt <user>");
		return;
	}

	sscanf(str, "%s", name);
	strtolower(name);

	p2=get_user_num(name, user);

        if (p2 == -1 )
     	{
      	not_signed_on(user,name);
      	return;
     	}
	
     if (!user_wants_message(p2,TTTS)) {
	write_str(user,"User is ignoring tic-tac-toes right now.");
	return;
	}

        /* See if p2 has user gagged */
        for (i=0; i<NUM_IGN_FLAGS; ++i) {
           if (TTTS==gagged_types[i]) {
		if (!check_gag(user,p2,0)) return;
             }
          }
        i=0;

	if (ustr[p2].afk) {
	   write_str(user,"User is AFK, wait until they come back.");
   	   return;
   	}

    if (!ustr[user].vis || !ustr[p2].vis) {
       write_str(user,"Both of you must be visible first.");
       return;
       }

    if (ustr[p2].afk)
     {
    if (ustr[p2].afk == 1) {
      if (!strlen(ustr[p2].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[p2].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[p2].say_name,ustr[p2].afkmsg);
      } 
     else {
      if (!strlen(ustr[p2].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[p2].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[p2].say_name,ustr[p2].afkmsg);
      }

       write_str(user,t_mess);
       return;
     }

	if (user==p2) {
		write_str(user,"You're not supposed to play with yourself!");
		return;
	}

/* can the person you're nerfing nerf back? */
for (i=0;sys[i].jump_vector!=-1;++i) {
        if (!strcmp(sys[i].command,".ttt")) break;
        }
     
if (ustr[p2].super < sys[i].su_com) {
  write_str(user,"That user doesn't have the .ttt command yet!");
  return;  
  }
	
	if (ustr[p2].ttt_opponent != -3) {
		sprintf(mess, TTT_PLAYING, ustr[p2].say_name);
		write_str(user,mess);
		return;
	}
	ustr[user].ttt_board = 0;
	ustr[p2].ttt_board = TTT_MY_MOVE + TTT_AM_NOUGHT;
	ustr[user].ttt_opponent = p2;
	ustr[p2].ttt_opponent = user;
	
	sprintf(mess,TTT_OFFERED,ustr[user].say_name);
	write_str(p2,mess);
	write_str(p2," Type: \".ttt abort\" to decline, or \".ttt <first move>\" to start");
	ttt_print_board(p2);
	ttt_print_board(user);
}

void ttt_make_move(int p, char *str)
{
	int winner, temp;
	
	if (!strlen(str)) {
		write_str(p,"You're playing a game, so");
		write_str(p,"Type: .ttt <square # to play>");
		write_str(p,"Type: \".ttt abort\" to abort this game");
		return;
	}
	if (((ustr[p].ttt_board)%(1<<17)) != ((ustr[ustr[p].ttt_opponent].ttt_board)%(1<<17))) {
		write_str(p,"Seems you're playing on different boards!");
		ttt_end_game(p, 0);
	}
	
	temp = atoi(str);

	if ((temp < 1) || (temp > 9)) {
		write_str(p,"Please play to a square on the board!!");
		ttt_print_board(p);
		return;
	}

	temp--;
	temp *= 2;
	
	if (ustr[p].ttt_board & (3<<temp)) {
		write_str(p,"Sorry, that square is already taken.. please choose another");
		ttt_print_board(p);
		return;
	}
	if (ustr[p].ttt_board & TTT_AM_NOUGHT) {
		ustr[p].ttt_board |= (1<<temp);
		ustr[ustr[p].ttt_opponent].ttt_board |= (1<<temp);
	}
	else {
		ustr[p].ttt_board |= (2<<temp);
		ustr[ustr[p].ttt_opponent].ttt_board |= (2<<temp);
	}

	ustr[p].ttt_board &= ~TTT_MY_MOVE;
	ustr[ustr[p].ttt_opponent].ttt_board |= TTT_MY_MOVE;
	
	winner = ttt_is_end(p);
	if (winner) {
		ttt_end_game(p, winner);
	}
	else {
		ttt_print_board(p);
		ttt_print_board(ustr[p].ttt_opponent);
	}
}

/* wrappers */

void ttt_print(int p)
{
	if (ustr[p].ttt_opponent == -3){
		write_str(p,"But you aren't playing a game!");
	}
	else
		ttt_print_board(p);
}

void ttt_abort(int p)
{
	if (ustr[p].ttt_opponent == -3){
		write_str(p,"But you aren't playing a game!");
	}
	else ttt_end_game(p, 0);
}

void ttt_cmd(int p, char *str)
{
	if (ustr[p].ttt_opponent == -3)
		ttt_new_game(p, str);
	else if (!strcmp(str, "abort"))
		ttt_abort(p);
	else if (!strcmp(str, "print"))
		ttt_print(p);
	else if (ustr[p].ttt_board & TTT_MY_MOVE)
		ttt_make_move(p, str);
	else {
		sprintf(mess, "Sorry, but %s gets to make the next move", ustr[ustr[p].ttt_opponent].say_name);
		write_str(p,mess);
		ttt_print_board(p);
	}
}
/*----------------------------------------------------------*/
/* End Tic-Tac-Toe routines				    */
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Begin Hangman Routines			            */
/*----------------------------------------------------------*/
/* lets a user start, stop or check out their status of a game of hangman */
void play_hangman(int user, char *inpstr)
{
int i;

if (!strlen(inpstr)) {
  write_str(user,"Usage: .hangman <start|stop|status>");
  return;
  }
/* srand(time(0)); */
strtolower(inpstr);
i=0;
if (!strcmp("status",inpstr)) {
  if (ustr[user].hang_stage==-1) {
    write_str(user,"You haven't started a game of hangman yet.");
    return;
    }
  write_str(user,"Your current hangman game status is:");
  if (strlen(ustr[user].hang_guess)<1) sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,"None yet!");
  else sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,ustr[user].hang_guess);
  write_str(user,mess);
  return;
  }
if (!strcmp("stop",inpstr)) {
  if (ustr[user].hang_stage==-1) {
    write_str(user,"You haven't started a game of hangman yet.");
    return;
    }
  ustr[user].hang_stage=-1;
  ustr[user].hang_word[0]='\0';
  ustr[user].hang_word_show[0]='\0';
  ustr[user].hang_guess[0]='\0';
  write_str(user,"You stop your current game of hangman.");
  return;
  }
if (!strcmp("start",inpstr)) {
  if (ustr[user].hang_stage>-1) {
    write_str(user,"You have already started a game of hangman.");
    return;
    }
  start_hang_word(ustr[user].hang_word);
  strcpy(ustr[user].hang_word_show,ustr[user].hang_word);
  for (i=0;i<strlen(ustr[user].hang_word_show);++i) ustr[user].hang_word_show[i]='-';
  ustr[user].hang_stage=0;
  write_str(user,"Your current hangman game status is:");
  sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,"None yet!");
  write_str(user,mess);
  return;
  }
write_str(user,"Usage: .hangman <start|stop|status>");
}

/* returns a word from a list for hangman.
   this will save loading words into memory, and the list could be updated as and when
   you feel like it */
char *start_hang_word(char *aword)
{
int lines,cnt,i;
char filename[FILE_NAME_LEN];
FILE *fp;

lines=cnt=i=0;
sprintf(filename,"%s",HANGDICT);
lines=file_count_lines(filename);
/* srand(time(0)); */
cnt=rand()%lines;
if (!(fp=fopen(filename,"r"))) return("hangman");
fscanf(fp,"%s\n",aword);
while (!feof(fp)) {
  if (i==cnt) {
    fclose(fp);
    strtolower(aword);
    return aword;
    }
  ++i;
  fscanf(fp,"%s\n",aword);
  }
fclose(fp);
/* if no word was found, just return a generic word */
return("hangman");
}


/* Lets a user guess a letter for hangman */
void guess_hangman(int user, char *inpstr)
{
int count,i,blanks;

count=blanks=i=0;
if (!strlen(inpstr)) {
  write_str(user,"Usage: .guess <letter|word>");
  return;
  }
if (ustr[user].hang_stage==-1) {
  write_str(user,"You haven't started a game of hangman yet.");
  return;
  }
if (strlen(inpstr)>1) {
  strtolower(inpstr);
  if (!strcmp(ustr[user].hang_word,inpstr)) {
	strcpy(ustr[user].hang_word_show,ustr[user].hang_word);
	blanks=0;
	goto HDONE;
  }  
  else {
  ustr[user].hang_stage++;
  write_str(user, HANG_BADGUESS);
  if (ustr[user].hang_stage>=7) strcpy(ustr[user].hang_word_show,ustr[user].hang_word);

  sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,ustr[user].hang_guess);
  write_str(user,mess);
  if (ustr[user].hang_stage>=7) {
    write_str(user, HANG_LOST);
    ustr[user].hang_losses++;
    ustr[user].hang_stage=-1;
    ustr[user].hang_word[0]='\0';
    ustr[user].hang_word_show[0]='\0';
    ustr[user].hang_guess[0]='\0';
    }
  else {
    sprintf(mess,"You have ^%d^ guess%s left",7-ustr[user].hang_stage,(7-ustr[user].hang_stage)==1 ? "" : "es");
    write_str(user,mess);
    }
  return;
  }
 }
else {
 strtolower(inpstr);
 }

if (strstr(ustr[user].hang_guess,inpstr)) {
  ustr[user].hang_stage++;
  write_str(user, HANG_GUESSED);
  if (ustr[user].hang_stage>=7) strcpy(ustr[user].hang_word_show,ustr[user].hang_word);

  sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,ustr[user].hang_guess);
  write_str(user,mess);
  if (ustr[user].hang_stage>=7) {
    write_str(user, HANG_LOST);
    ustr[user].hang_losses++;
    ustr[user].hang_stage=-1;
    ustr[user].hang_word[0]='\0';
    ustr[user].hang_word_show[0]='\0';
    ustr[user].hang_guess[0]='\0';
    }
  else {
    sprintf(mess,"You have ^%d^ guess%s left",7-ustr[user].hang_stage,(7-ustr[user].hang_stage)==1 ? "" : "es");
    write_str(user,mess);
    }
  return;
  }
for (i=0;i<strlen(ustr[user].hang_word);++i) {
  if (ustr[user].hang_word[i] == inpstr[0]) {
    ustr[user].hang_word_show[i]=ustr[user].hang_word[i];
    ++count;
    }
  if (ustr[user].hang_word_show[i]=='-') ++blanks;
  }
strcat(ustr[user].hang_guess,inpstr);
if (!count) {
  ustr[user].hang_stage++;
  write_str(user, HANG_BADLETTER);
  if (ustr[user].hang_stage>=7) strcpy(ustr[user].hang_word_show,ustr[user].hang_word);

  sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,ustr[user].hang_guess);
  write_str(user,mess);
  if (ustr[user].hang_stage>=7) {
    write_str(user, HANG_LOST);
    ustr[user].hang_losses++;
    ustr[user].hang_stage=-1;
    ustr[user].hang_word[0]='\0';
    ustr[user].hang_word_show[0]='\0';
    ustr[user].hang_guess[0]='\0';
    }
  else {
    sprintf(mess,"You have ^%d^ guess%s left",7-ustr[user].hang_stage,(7-ustr[user].hang_stage)==1 ? "" : "es");
    write_str(user,mess);
    }
  return;
  }
if (count==1) sprintf(mess,HANG_1OCCUR,inpstr);
else sprintf(mess,HANG_MOCCUR,count,inpstr);
write_str(user,mess);
HDONE:
sprintf(mess,hanged[ustr[user].hang_stage],ustr[user].hang_word_show,ustr[user].hang_guess);
write_str(user,mess);
if (!blanks) {
  write_str(user, HANG_WON);
  ustr[user].hang_wins++;
  ustr[user].hang_stage=-1;
  ustr[user].hang_word[0]='\0';
  ustr[user].hang_word_show[0]='\0';
  ustr[user].hang_guess[0]='\0';
  }
else {
    sprintf(mess,"You have ^%d^ guess%s left",7-ustr[user].hang_stage,(7-ustr[user].hang_stage)==1 ? "" : "es");
    write_str(user,mess);
  }
}

