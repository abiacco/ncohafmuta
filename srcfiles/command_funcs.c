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


extern int commands;


/*** COMMAND LIST ***/

/* User command list structure */
struct command_struct sys[] = {
		  {".abbrs",    1,      117,    INFO,""},
		  {".afk",      0,      75,     SETS,""},
                  {".auto",     3,      186,    MISC,""},
		  {".bafk",     0,      99,     SETS,""},
		  {".bop",      1,      172,    NONE,""},
		  {".bubble",   1,      121,    MISC,""},
		  {".call",     1,      124,    COMM,""},
		  {".creply",   1,      125,    COMM,"?"},
		  {".cbuff",    1,      42,     MISC,"*"},
		  {".chicken",  1,      213,    NONE,""},
                  {".chuckle",  1,      192,    NONE,""},
                  {".cough",    1,      193,    NONE,""},
		  {".cls",      0,      84,     MISC,""},
		  {".cmail",    1,      47,     MAIL,""},
		  {".csent",    1,      142,    MAIL,""},
                  {".dance",    1,      194,    NONE,""},
		  {".desc",     0,      37,     SETS,""},
                  {".doh",      1,      195,    NONE,""},
		  {".emote",    1,      11,     COMM,";"},
		  {".entpro",   0,      105,    SETS,""},
		  {".entermsg", 1,      116,    SETS,""},
		  {".examine",  0,      107,    INFO,""},
		  {".exitmsg",  1,      158,    SETS,""},
		  {".faq",      0,      106,    INFO,""},
		  {".fight",    1,      85,     MISC,""},
		  {".find",     1,      123,    INFO,""},
                  {".flirt",    1,      196,    NONE,""},
		  {".fmail",    1,      133,    MAIL,""},
		  {".go",       0,      7,      MOOV,""},
                  {".goose",    1,      197,    NONE,""},
                  {".grab",     1,      198,    NONE,""},
		  {".gripe",    1,      156,    MESG,""},
                  {".growl",    1,      199,    NONE,""},
		  {".guess",    1,      218,    MISC,""},
		  {".guru",     1,      188,    MISC,""},
		  {".help",     0,      25,     INFO,""},
                  {".hiss",     1,      200,    NONE,""},
		  {".home",     1,      159,    MOOV,""},
		  {".hug",      1,      164,    NONE,""},
		  {".ignore",   0,      5,      SETS,""},
                  {".insult",   1,      201,    NONE,""},
		  {".invite",   1,      10,     MOOV,""},
                  {".kick",     1,      202,    NONE,""},
		  {".kiss",     1,      170,    NONE,""},
		  {".last",     1,      119,    INFO,""},
		  {".laugh",    1,      165,    NONE,""},
		  {".lick",     1,      175,    NONE,""},
		  {".listen",   0,      4,      SETS,""},
		  {".log",      0,      137,    INFO,""},
                  {".lol",      1,      203,    NONE,""},
		  {".look",     0,      6,      MISC,"@"},
		  {".macros",   1,      44,     SETS,""},
		  {".map",      0,      126,    MISC,""},
		  {".mutter",   1,      129,    COMM,"#"},
		  {".nerf",     1,      160,    MISC,""},
		  {".news",     0,      27,     INFO,""},
		  {".noogie",   1,      214,    NONE,""},
		  {".password", 0,      67,     MISC,""},
		  {".poke",     1,      166,    NONE,""},
		  {".quit",     0,      0,      MISC,""},
		  {".quote",    1,      136,    SETS,""},
		  {".read",     0,      15,     MESG,""},
		  {".reload",   1,      162,    MISC,""},
		  {".review",   1,      24,     MISC,"<"},
		  {".rmail",    1,      45,     MAIL,""},
		  {".rooms",    1,      12,     INFO,""},
		  {".rsent",    1,      143,    MAIL,""},
		  {".rules",    0,      212,    INFO,""},
		  {".say",      0,      90,     COMM,"\""},
		  {".schedule", 0,      148,    INFO,""},
		  {".semote",   1,      69,     COMM,"/"},
		  {".set",      0,      79,     SETS,""},
		  {".shout",    1,      2,      COMM,"!"},
                  {".shake",    1,      204,    NONE,""},
                  {".shove",    1,      205,    NONE,""},
		  {".sing",     1,      151,    MISC,"%"},
                  {".slap",     1,      206,    NONE,""},
		  {".smail",    1,      46,     MAIL,""},
		  {".smile",    1,      176,    NONE,""},
		  {".smirk",    1,      174,    NONE,""},
		  {".socials",  1,      163,    COMM,""},
		  {".sos",      0,      104,    MISC,""},
		  {".sthink",   1,      122,    COMM,""},
		  {".suggest",  1,      130,    MESG,""},
		  {".suicide",  0,      182,    MISC,""},
		  {".swho",     0,      118,    INFO,""},
		  {".tell",     0,      3,      COMM,","},
		  {".tackle",   1,      173,    NONE,""},
		  {".talker",   1,      120,    MISC,""},
		  {".think",    1,      103,    COMM,""},
		  {".thwap",    1,      171,    NONE,""},
		  {".tickle",   1,      169,    NONE,""},
		  {".time",     0,      76,     MISC,""},
		  {".to",       1,      183,    COMM,"$"},
		  {".version",  0,      111,    MISC,""},
		  {".who",      0,      1,      INFO,"{"},
		  {".wedgie",   1,      43,     NONE,""},
                  {".whine",    1,      207,    NONE,""},
                  {".wink",     1,      208,    NONE,""},
		  {".wlist",    0,      147,    INFO,""},
                  {".woohoo",   1,      209,    NONE,""},
		  {".write",    1,      14,     MESG,""},
		  {".alert",    1,      146,    SETS,""},
		  {".fail",     1,      127,    SETS,""},
		  {".femote",   1,      113,    COMM,""},
		  {".friends",  1,      191,    INFO,""},
		  {".ftell",    1,      161,    COMM,""},
		  {".gag",      1,      145,    SETS,""},
		  {".greet",    1,      40,     MISC,""},
		  {".hangman",  1,      219,    MISC,""},
		  {".knock",    1,      13,     MOOV,""},
		  {".meter",    1,      91,     MISC,""},
		  {".preview",  1,      66,     INFO,""},
		  {".private",  1,      8,      SETS,""},
		  {".public",   1,      9,      SETS,""},
		  {".search",   1,      23,     MESG,""},
		  {".shemote",  1,      112,    COMM,"&"},
		  {".shthink",  1,      181,    COMM,""},
		  {".succ",     1,      128,    SETS,""},
		  {".topic",    1,      18,     MISC,""},
		  {".ttt",      1,      217,    MISC,""},
		  {".ustat",    1,      78,     INFO,""},
		  {".vote",     1,      168,    MISC,""},
		  {".with",     1,      102,    INFO,""},
		  {".wizards",  0,      101,    INFO,""},
		  {".beep",     1,      74,     COMM,""},
		  {".echo",     1,      36,     COMM,""},
		  {".follow",   1,      72,     MOOV,""},
		  {".ptell",    1,      71,     COMM,""},
		  {".ranks",    1,      58,     MISC,""},
		  {".show",     2,      108,    MISC,"\'"},
		  {".anchor",   2,      135,    MOOV,""},
		  {".arrest",   2,      41,     MOOV,""},
		  {".unarrest", 2,      190,    MOOV,""},
		  {".memory",   2,      177,    INFO,""},
                  {".gagcomm",  2,      184,    COMM,""},
		  {".muzzle",   2,      51,     COMM,""},
		  {".system",   2,      28,     INFO,""},
		  {".unmuzzle", 2,      52,     COMM,""},
		  {".wipe",     2,      16,     MESG,""},
		  {".bcast",    2,      26,     COMM,""},
		  {".bring",    2,      54,     MOOV,""},
		  {".clist",    2,      167,    INFO,""},
		  {".hide",     2,      56,     SETS,""},
		  {".monitor",  2,      70,     SETS,""},
		  {".move",     2,      29,     MOOV,""},
		  {".picture",  1,      65,     MISC,""},
		  {".cline",    3,      109,    MISC,""},
		  {".wiztell",    2,    82,     COMM,">"},
		  {".demote",     3,    50,     SETS,""},
		  {".descroom",   3,    155,    SETS,""},
		  {".finger",     3,    140,    INFO,""},
		  {".force",      3,    153,    SETS,""},
                  {".frog",       2,    185,    COMM,""},
		  {".grant",      3,    216,    SETS,""},
		  {".whois",      2,    141,    INFO,""},
		  {".kill",       2,    21,     INFO,""},
		  {".nslookup",   2,    139,    INFO,""},
		  {".permission", 3,    68,     SETS,""},
		  {".promote",    3,    49,     SETS,""},
		  {".realuser",   3,    138,    INFO,""},
		  {".samesite",   2,    144,    INFO,""},
		  {".site",       2,    80,     INFO,""},
		  {".swipe",      3,    134,    MESG,""},
		  {".addatmos",   3,    178,    SETS,""},
		  {".delatmos",   3,    179,    SETS,""},
		  {".listatmos",  3,    180,    SETS,""},
		  {".atmos",      3,    35,     SETS,""},
		  {".bannew",     3,    149,    BANS,""},
		  {".banname",    3,    210,    BANS,""},
		  {".expire",     3,    152,    SETS,""},
		  {".nuke",       3,    83,     MISC,""},
		  {".pcreate",    3,    211,    MISC,""},
		  {".restrict",   3,    59,     BANS,""},
		  {".resolve",    3,    86,     MISC,""},
		  {".revoke",     3,    215,    SETS,""},
		  {".unbannew",   3,    150,    BANS,""},
		  {".unrestrict", 3,    60,     BANS,""},
		  {".users",      3,    77,     MISC,""},
		  {".wlog",       1,    189,    MESG,""},
		  {".wnote",      2,    97,     MESG,""},
		  {".xcomm",      3,    100,    COMM,""},
		  {".suname",     3,    114,    MISC,""},
		  {".supass",     3,    115,    MISC,""},
		  {".allow_new",  4,    38,     BANS,""},
		  {".backup",     4,    220,    SETS,""},
		  {".bbcast",     4,    110,    COMM,""},
		  {".close",      4,    30,     MISC,""},
		  {".gwipe",      4,    157,    MESG,""},
		  {".open",       4,    31,     MISC,""},
		  {".quota",      4,    95,     SETS,""},
		  {".readlog",    3,    154,    INFO,""},
		  {".reinit",     4,    73,     MISC,""},
		  {".shutdown",   4,    22,     MISC,""},
		  {".wwipe",      4,    98,     MESG,""},
		  /*-----------------------------------------------*/
		  /* this item marks the end of the list, do not   */
		  /* remove it                                     */
		  /*-----------------------------------------------*/
		  {"<EOL>",       -1,   -1,     -1,     ""}
		};

/* Bot command list structure */
struct botcommand_struct botsys[] = {
		  {"_who",    0,    INFO},
		  /*-----------------------------------------------*/
		  /* this item marks the end of the list, do not   */
		  /* remove it                                     */
		  /*-----------------------------------------------*/
		  {"<EOL>",	-1,	-1}
		};



/*** COMMAND FUNCTIONS ***/

/*** Call command function or execute command directly ***/
void exec_com(int com_num, int user, char *inpstr)
{
int z=0,gravoked=0;
char temp_mess[ARR_SIZE+65];
char filename[FILE_NAME_LEN];
FILE *fp;

/* See if user is suspended from using all commands */
if (ustr[user].suspended)
  {
    return;
  }

/* Check if command was granted to user - UNDER CONSTRUCTION */
for (z=0;z<MAX_GRAVOKES;++z) {
	if (!is_grant(ustr[user].revokes[z])) continue;
	if (strip_com(ustr[user].revokes[z])==sys[com_num].jump_vector) {
		gravoked=1; break;
	  }
  }
if (gravoked==1) {
gravoked=0;
ustr[user].tempsuper=strip_level(ustr[user].revokes[z]);
} /* end of granted if */
else {
 /* See if user has the required rank for this command */
 if (ustr[user].super < sys[com_num].su_com) 
   {
    write_str(user,NOT_WORTHY);
    return;
   }

 /* Check if command was revoked from user - UNDER CONSTRUCTION */
 for (z=0;z<MAX_GRAVOKES;++z) {
	if (!is_revoke(ustr[user].revokes[z])) continue;
	if (strip_com(ustr[user].revokes[z])==sys[com_num].jump_vector) { gravoked=1; break; }
   }
 if (gravoked==1) {
    write_str(user,NOT_WORTHY);
    gravoked=0; z=0;
    return;
    }

ustr[user].tempsuper=ustr[user].super;
} /* end of not granted else */

sprintf(filename,"%s/%s",LOGDIR,LASTFILE);
if (!(fp=fopen(filename,"w"))) {
   sprintf(temp_mess,"Couldn't write last command from %s, to file! %s",strip_color(ustr[user].say_name),get_error());
   btell(user,temp_mess); 
   write_log(ERRLOG,YESTIME,"%s\n",temp_mess);
   }
else {
fprintf(fp,"%s: Command < %s > from %s\n",get_time(0,0),inpstr,strip_color(ustr[user].say_name));
FCLOSE(fp);
temp_mess[0]=0;
}

if (ustr[user].numcoms==10000000) ustr[user].numcoms=1;
ustr[user].numcoms++;
commands++;
  
remove_first(inpstr);  /* get rid of commmand word */

switch(sys[com_num].jump_vector) {
	case 0 : user_quit(user,1); break;
	case 1 : t_who(user,inpstr,0); break;
	case 2 : shout(user,inpstr); break;
	case 3 : tell_usr(user,inpstr,0); break;
	case 4 : user_listen(user,inpstr); break;
	case 5 : user_ignore(user,inpstr); break;
	case 6 : look(user,inpstr);  break;
	case 7 : go(user,inpstr,0);  break;
	case 8 : room_access(user,1);  break; /* private */
	case 9 : room_access(user,0);  break; /* public */
	case 10: invite_user(user,inpstr);  break;
	case 11: emote(user,inpstr);  break;
	case 12: rooms(user,inpstr);  break;
	case 13: go(user,inpstr,1);  break;  /* knock */
	case 14: write_board(user,inpstr,0);  break; /* normal write */
	case 15: read_board(user,0,inpstr);  break;  /* normal read  */
	case 16: wipe_board(user,inpstr,0);  break;
	case 18: set_topic(user,inpstr);  break;
	case 21: kill_user(user,inpstr);  break;
	case 22: shutdown_d(user,inpstr);  break;
	case 23: search_boards(user,inpstr);  break;
	case 24: review(user);  break;
	case 25: help(user,inpstr);  break;
	case 26: broadcast(user,inpstr);  break;
	case 27: if (!cat(NEWSFILE,user,0)) 
			write_str(user,NO_NEWS);
                        write_str(user,"Ok");
		 break;

	case 28: system_status(user); break;
	case 29: move(user,inpstr);  break;
	case 30: system_access(user,inpstr,0);
	         break;  /* close */

	case 31: system_access(user,inpstr,1);
	         break;  /* open */

	case 35: toggle_atmos(user,inpstr); break;
	case 36: my_echo(user,inpstr);  break;
	case 37: old_func(user,inpstr,1);  break;
	case 38: toggle_allow(user,inpstr); break;
        case 40: greet(user,inpstr); break;
	case 41: arrest(user,inpstr,0); break;
	case 42: cbuff2(user,inpstr); break;
        case 43: socials(user,inpstr,32); break; /* wedgie */
	case 44: macros(user,inpstr); break;
        case 45: read_mail(user,inpstr); break;
        case 46: send_mail(user,inpstr,0); break;
        case 47: ustr[user].clrmail= -1;
                 clear_mail(user, inpstr); 
                 break;
                 
	case 49: promote(user,inpstr); break;
	case 50: demote(user,inpstr); break;
	case 51: muzzle(user,inpstr,0); break;
	case 52: unmuzzle(user,inpstr); break;
	case 54: bring(user,inpstr); break;
	case 56: hide(user,inpstr); break;
	case 58: display_ranks(user); break;
	case 59: restrict(user,inpstr,ANY); break;
	case 60: unrestrict(user,inpstr,ANY); break;
	case 65: picture(user,inpstr); break;                 
	case 66: preview(user,inpstr); break;                
	case 67: password(user,inpstr); break;              
	case 68: permission_u(user,inpstr); break;             
	case 69: semote(user,inpstr); break;               
	case 70: tog_monitor(user); break;                     
	case 71: ptell(user,inpstr); break;                
	case 72: follow(user,inpstr); break;                
	case 73: read_init_data(1); 
	         messcount();
	         write_str(user,"<ok>"); 
	         break;                  
	case 74: beep_u(user,inpstr); break;              
	case 75: set_afk(user,inpstr);  break;
	case 76: systime(user,inpstr);  break;
	case 77: print_users(user,inpstr);  break;
	case 78: usr_stat(user,inpstr,0);  break;
	case 79: set(user,inpstr);  break;
	case 80: swho(user,inpstr); break;
	case 82: btell(user,inpstr); break;
	case 83: nuke(user,inpstr,0); break;
	case 84: cls(user); break;
	case 85: fight_another(user,inpstr); break;
	case 86: resolve_names_set(user,inpstr); break;
	case 90: say(user,inpstr,1); break;
	case 91: meter(user, inpstr); break;
	case 95: set_quota(user, inpstr); break;
	case 96: command_disabled(user); break;
	case 97: write_board(user,inpstr,1);  break; /* wiz_note */
	case 98: wipe_board(user,inpstr,1);  break;  /* wiz wipe */
	case 99: set_bafk(user,inpstr);  break;  
        case 100: xcomm(user,inpstr);  break;  
	case 101: t_who(user,inpstr,1); break;
	case 102: t_who(user,inpstr,2); break;
        case 103: think(user,inpstr); break;
        case 104: sos(user,inpstr); break;
        case 105: old_func(user,inpstr,2); break;
        case 106: if (!cat(FAQFILE,user,0))
                        write_str(user,NO_FAQ);
                        write_str(user,"<ok>");
                        break;
	case 107: usr_stat(user,inpstr,1); break;
        case 108: show(user,inpstr); break;
        case 109: cline(user,inpstr); break;
        case 110: bbcast(user,inpstr); break;
        case 111: version(user); break;
        case 112: shemote(user,inpstr); break; 
        case 113: femote(user,inpstr); break;
        case 114: suname(user,inpstr);  break;
        case 115: supass(user,inpstr);  break;
        case 116: old_func(user,inpstr,5);  break;
        case 117: abbrev(user,inpstr);  break;
        case 118: newwho(user);  break;
        case 119: last_u(user,inpstr);  break;
        case 120: talker(user,inpstr);  break;
        case 121: bubble(user);  break;
        case 122: sthink(user,inpstr);  break;
        case 123: where(user,inpstr);  break;
        case 124: call(user,inpstr);  break;
        case 125: creply(user,inpstr);  break;
        case 126: if (!cat(MAPFILE,user,0))
                        write_str(user,NO_MAP);
                        write_str(user,"Ok");
                  break;
        case 127: old_func(user,inpstr,3);  break;
        case 128: old_func(user,inpstr,4);  break;
        case 129: mutter(user,inpstr);  break;
        case 130: write_board(user,inpstr,3); break; /* suggestions */
        case 131: break;
        case 132: break;
        case 133: fmail(user,inpstr);  break;
        case 134: swipe(user,inpstr);  break;
        case 135: anchor_user(user,inpstr); break;
        case 136: quote_op(user,inpstr);  break;
        case 137: list_last(user,inpstr);  break;
        case 138: real_user(user,inpstr);  break;
        case 139: pukoolsn(user,inpstr);  break;
        case 140: regnif(user,inpstr);  break;
        case 141: siohw(user,inpstr);  break;        
        case 142: ustr[user].clrmail= -1;
                  clear_sent(user,inpstr);  break;
        case 143: read_sent(user,inpstr);  break;
        case 144: same_site(user,inpstr);  break;
        case 145: gag(user,inpstr);  break;
        case 146: alert(user,inpstr);  break;
        case 147: if (!cat(WIZFILE,user,0))   
                        write_str(user,NO_WIZLIST);
                        write_str(user,"Ok");
                  break;
        case 148: schedule(user);  break;
        case 149: restrict(user,inpstr,NEW); break;
        case 150: unrestrict(user,inpstr,NEW); break;
        case 151: sing(user,inpstr); break;
        case 152: show_expire(user,inpstr); break;
        case 153: force_user(user,inpstr); break;
        case 154: readlog(user,inpstr); break;
        case 155: descroom(user,inpstr); break;
        case 156: write_board(user,inpstr,2);  break; /* gripe_note */
        case 157: wipe_board(user,inpstr,2);  break;  /* gripe wipe */
        case 158: old_func(user,inpstr,6);  break;
        case 159: home_user(user);  break;
        case 160: nerf(user,inpstr);  break;
        case 161: frtell(user,inpstr); break;
        case 162: reload(user);  break;
        case 163: list_socs(user);  break;
        case 164: socials(user,inpstr,1);  break;  /* hug */
        case 165: socials(user,inpstr,2);  break;  /* laugh */
        case 166: socials(user,inpstr,3);  break;  /* poke */
        case 167: clist(user,inpstr);  break;
        case 168: vote(user,inpstr);  break;
        case 169: socials(user,inpstr,4);  break;  /* tickle */
        case 170: socials(user,inpstr,5);  break;  /* kiss */
        case 171: socials(user,inpstr,6);  break;  /* thwap */
        case 172: socials(user,inpstr,7);  break;  /* bop */
        case 173: socials(user,inpstr,8);  break;  /* tackle */
        case 174: socials(user,inpstr,9);  break;  /* smirk */
        case 175: socials(user,inpstr,10);  break;  /* lick */
        case 176: socials(user,inpstr,11);  break;  /* smile */
        case 177: check_mem(user);  break;
        case 178: add_atmos(user,inpstr);  break;
        case 179: del_atmos(user,inpstr);  break;
        case 180: list_atmos(user);  break;
        case 181: shout_think(user,inpstr);  break;
        case 182: suicide_user(user,inpstr);  break;
        case 183: say_to_user(user,inpstr);  break;
        case 184: gag_comm(user,inpstr,0);  break;
        case 185: frog_user(user,inpstr);  break;
        case 186: auto_com(user,inpstr);  break;
        case 187: break;
        case 188: eight_ball(user,inpstr);  break;
        case 189: warning(user,inpstr,1);  break;
        case 190: arrest(user,inpstr,1);  break;
        case 191: t_who(user,inpstr,3);  break;
        case 192: socials(user,inpstr,12); break; /* chuckle */
        case 193: socials(user,inpstr,13); break; /* cough */
        case 194: socials(user,inpstr,14); break; /* dance */
        case 195: socials(user,inpstr,15); break; /* doh */
        case 196: socials(user,inpstr,16); break; /* flirt */
        case 197: socials(user,inpstr,17); break; /* goose */
        case 198: socials(user,inpstr,18); break; /* grab */
        case 199: socials(user,inpstr,19); break; /* growl */
        case 200: socials(user,inpstr,20); break; /* hiss */
        case 201: socials(user,inpstr,21); break; /* insult */
        case 202: socials(user,inpstr,22); break; /* kick */
        case 203: socials(user,inpstr,23); break; /* lol */
        case 204: socials(user,inpstr,24); break; /* shake */
        case 205: socials(user,inpstr,25); break; /* shove */
        case 206: socials(user,inpstr,26); break; /* slap */
        case 207: socials(user,inpstr,27); break; /* whine */
        case 208: socials(user,inpstr,28); break; /* wink */
        case 209: socials(user,inpstr,29); break; /* woohoo */
        case 210: banname(user,inpstr); break;
        case 211: player_create(user,inpstr); break;
        case 212: if (!cat(RULESFILE,user,0))   
                        write_str(user,NO_RULESFILE);
                        write_str(user,"Ok");
                  break;
        case 213: socials(user,inpstr,30); break; /* chicken */
        case 214: socials(user,inpstr,31); break; /* noogie */
	case 215: revoke_com(user,inpstr); break;
	case 216: grant_com(user,inpstr); break;
	case 217: ttt_cmd(user,inpstr); break;
	case 218: guess_hangman(user,inpstr);  break;
	case 219: play_hangman(user,inpstr);  break;
	case 220: backup_stuff(user,inpstr);  break;
        default: break;
	}
}

/*** Call command function or execute command directly ***/
void exec_bot_com(int com_num, int user, char *inpstr)
{
char temp_mess[ARR_SIZE+41];
char filename[FILE_NAME_LEN];
FILE *fp;

/* see if su command */
if (ustr[user].suspended)
  {
    return;
  }
  
sprintf(filename,"%s/%s",LOGDIR,LASTFILE);
if (!(fp=fopen(filename,"w"))) {
   sprintf(temp_mess,"Couldn't write last command from bot %s, to file! %s",strip_color(ustr[user].say_name),get_error());
   btell(user,temp_mess); 
   write_log(ERRLOG,YESTIME,"%s\n",temp_mess);
   FCLOSE(fp);
   }

fprintf(fp,"%s: Command < %s > from %s\n",get_time(0,0),inpstr,strip_color(ustr[user].say_name));
FCLOSE(fp);
temp_mess[0]=0;

if (ustr[user].numcoms==10000000) ustr[user].numcoms=1;
ustr[user].numcoms++;
  
remove_first(inpstr);  /* get rid of commmand word */

switch(botsys[com_num].jump_vector) {
	case 0 : bot_whoinfo(user,inpstr); break;
        default: break;
     }
}

