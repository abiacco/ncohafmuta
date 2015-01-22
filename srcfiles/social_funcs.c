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
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */
extern struct command_struct sys[];


/*---------------------------------------------------------*/
/*    SOCIAL SECTION                                       */
/*---------------------------------------------------------*/
/*** List social commands ***/
void list_socs(int user)
{
int c=0;
int nl=0;

write_str(user,"+-------------------+");
write_str(user," Socials Available");
write_str(user,"+-------------------+");

nl = 0;

/* All socials have a type of NONE in the commands structure, so only
   show those */

for (c=0; sys[c].su_com != -1 ;++c) {
   if (sys[c].type==NONE) {
        sprintf(mess,"%-11.11s",sys[c].command);
        mess[0]=' ';
        if (nl== -1)
          {write_str_nr(user, "  ");
           nl=0;
          }
        write_hilite_nr(user,mess);
        ++nl;
        if (nl==5)
          {
            write_str(user," "); 
            nl= 0;
          }
       }   /* end of if */
   else continue;
  }   /* end of for */
write_str(user," ");
write_str(user," ");

}

/*** Function for all socials based on type ***/
void socials(int user, char *inpstr, int type)
{
int u;
int in_room=0;
char name[SAYNAME_LEN+10],nametemp[100];
char other_user[ARR_SIZE];

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

if (!strlen(inpstr)) {
       switch(type) {
	case 1: emote(user,"hugs everyone.\0");  break;
	case 2: emote(user,"laughs!\0");  break;
	case 3: emote(user,"pokes everyone.\0");  break;
	case 4: emote(user,"tickles everyone!\0");  break;
	case 5: emote(user,"blows everyone a big kiss!\0");  break;
	case 6: emote(user,"thwaps everyone in the room!\0");  break;
	case 7: emote(user,"bops everyone in the room!\0");  break;
	case 8: emote(user,"tackles everyone in a tackle-frenzy!\0"); break;
	case 9: emote(user,"smirks at everyone.\0");  break;
	case 10: emote(user,"licks everyone in sight! Dog imitation!\0"); break;
	case 11: emote(user,"smiles brightly at everyone\0");  break;
        case 12: emote(user,"chuckles insanely at everyone\0"); break;
        case 13: emote(user,"coughs up a lung in everyone's general direction\0"); break;
        case 14: emote(user,"dances with everyone in the room!\0"); break;
        case 15: emote(user,"DOH's!\0"); break;
        case 16: emote(user,"turns to everyone in the room and says \"Hey baby\"\0"); break;
        case 17: emote(user,"gooses everyone in the room..pervert!\0"); break;
        case 18: emote(user,"grabs everyone and SCREAMS!\0"); break;
        case 19: emote(user,"growls at everyone\0"); break;
        case 20: emote(user,"hisses at everyone like a snake, hisssssss\0"); break;
        case 21: emote(user,"turns to everyone and says \"Yo mama sleeps with my dog!\"\0"); break;
        case 22: emote(user,"kicks everyone in the room!\0"); break;
        case 23: emote(user,"laughs out loud!\0"); break;
        case 24: emote(user,"shakes their head(s) at everyone\0"); break;
        case 25: emote(user,"shoves everyone to the ground!\0"); break;
        case 26: emote(user,"slaps everyone in the room!\0"); break;
        case 27: emote(user,"whines to everyone..big baby.\0"); break;
        case 28: emote(user,"winks at everyone in the room.\0"); break;
        case 29: emote(user,"WOOHOO's!\0"); break;
        case 30: emote(user,"starts flinging chicken at everyone!\0"); break;
        case 31: emote(user,"puts everyone in a headlock and gives them a noogie\0"); break;
        case 32: emote(user,"runs around the room giving atomic wedgies!\0"); break;
       }  /* end of switch */
   }

/* One case for every social..check if victim is in room, then check
   is victim is user, if not, emote to the room..if victim is not in the
   room, semote the action */

else {
	sscanf(inpstr,"%s ",other_user);
	strtolower(other_user);
	if ((u=get_user_num(other_user,user)) == -1 ) {
		not_signed_on(user,other_user);
		return;
            }
            if (ustr[user].area==ustr[u].area) in_room=1;

	if (!ustr[u].vis) {
	strcpy(nametemp,INVIS_ACTION_LABEL);
	nametemp[0]=tolower((int)nametemp[0]);
	strcpy(name,nametemp);
	}
	else {
	strcpy(name,ustr[u].say_name);
	}

       switch(type) {
           case 1: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"hugs theirself.");
                   else
   		    sprintf(t_mess,"hugs %s warmly",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," hugs you warmly");
		   semote(user,other_user);
		   }
		   break;
           case 2: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"laughs at themselves.");
                   else
   		    sprintf(t_mess,"laughs at %s.",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," laughs!");
		   semote(user,other_user);
		   }
		   break;
           case 3: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"pokes themselves.");
                   else
   		    sprintf(t_mess,"pokes %s in some choice spots",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," pokes you.");
		   semote(user,other_user);
		   }
		   break;
           case 4: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"tickles themselves for some strange reason");
                   else
   		    sprintf(t_mess,"tickle-attacks %s!",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," tickles you!");
		   semote(user,other_user);
		   }
		   break;
           case 5: if (in_room) {
                   if (user==u) {
                     write_str(user,"You cant kiss yourself!");
                     }
                   else {
  		    sprintf(t_mess,"kisses %s!",name);
		    emote(user,t_mess);
                    }
		   }
		   else {
		   strcat(other_user," kisses you!");
		   semote(user,other_user);
		   }
		   break;
           case 6: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"thwaps theirself just for the hell of it!");
                   else
   		    sprintf(t_mess,"thwaps %s into the ground!",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," thwaps you!");
		   semote(user,other_user);
		   }
		   break;
           case 7: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"bops theirself on the head!");
                   else
   		    sprintf(t_mess,"bops %s on the head!",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," bops you on the head!");
		   semote(user,other_user);
		   }
		   break;
           case 8: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"tried to tackle themselves! Call the men in white.");
                   else
   		    sprintf(t_mess,"tackles %s to the ground!",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," tackles you to the ground!");
		   semote(user,other_user);
		   }
		   break;
           case 9: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"tried to smirk at themselves! Call the men in white.");
                   else
   		    sprintf(t_mess,"smirks at %s",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," smirks at you");
		   semote(user,other_user);
		   }
		   break;
           case 10: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"sticks their tongue out and tries to lick themselves! Oooooook.");
                   else
   		    sprintf(t_mess,"licks %s on the cheek! Ewwww!",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," licks you on the cheek!");
		   semote(user,other_user);
		   }
		   break;
           case 11: if (in_room) {
                   if (user==u) 
                    strcpy(t_mess,"tried to smile at themselves! Oooooook.");
                   else
   		    sprintf(t_mess,"smiles innocently at %s",name);

		   emote(user,t_mess);
		   }
		   else {
		   strcat(other_user," smiles innocently");
		   semote(user,other_user);
		   }
		   break;
           case 12: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"chuckles to themself.");
                   else   
                    sprintf(t_mess,"chuckles at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," chuckles at ya");
                   semote(user,other_user);
                   }
                   break;
           case 13: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"coughs on themself.");        
                   else
                    sprintf(t_mess,"coughs on %s, how disgusting",name);       

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," coughs in your direction");   
                   semote(user,other_user);
                   }
                   break;
           case 14: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"dances around by themself.");        
                   else
                    sprintf(t_mess,"dances around the room with %s",name);       

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," grabs your hand and dances with you");
                   semote(user,other_user);
                   }
                   break;
           case 15: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"doh's to themself.");        
                   else
                    sprintf(t_mess,"looks at %s and doh's",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," doh's!");
                   semote(user,other_user);
                   }
                   break;
           case 16: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"flirts with themself. Makes ya wonder.");
                   else
                    sprintf(t_mess,"turns to %s and says \"Hey baby\"",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," turns to you and says \"Hey baby\"");
                   semote(user,other_user);
                   }
                   break;
           case 17: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"gooses themself. Yeah.. ok. Weirdo!");
                   else
                    sprintf(t_mess,"gooses %s!",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," gooses ya!");
                   semote(user,other_user);
                   }
                   break;
           case 18: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"grabs themself like Michael Jackson!");
                   else
                    sprintf(t_mess,"grabs %s and SCREAMS!",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," grabs you and screams!");     
                   semote(user,other_user);
                   }
                   break;
           case 19: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"growls at themself.");
                   else
                    sprintf(t_mess,"growls at %s..kinky!",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," growls at you");
                   semote(user,other_user);
                   }
                   break;
           case 20: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"hisses at themself.");
                   else
                    sprintf(t_mess,"hisses at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," hisses at you");
                   semote(user,other_user);
                   }
                   break;
           case 21: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"insults themself. Yep, must be bored.");
                   else
                    sprintf(t_mess,"looks at %s and says \"Yo mama sleeps with my dog!\"",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," looks at you and says \"Yo mama sleeps with my dog!\"");
                   semote(user,other_user);
                   }
                   break;
           case 22: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"kicks themself.");
                   else
                    sprintf(t_mess,"kicks %s! OUCH!",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," kicks you! OUCH!");
                   semote(user,other_user);
                   }
                   break;
           case 23: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"laughs out loud at themself!");    
                   else
                    sprintf(t_mess,"laughs out loud at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," laughs out loud at you!");
                   semote(user,other_user);
                   }
                   break;
           case 24: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"shakes their head(s) at themself.");    
                   else
                    sprintf(t_mess,"shakes their head(s) at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," shakes their head(s) at you");
                   semote(user,other_user);
                   }
                   break;
           case 25: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"shoves themself to the ground. Idiot.");    
                   else
                    sprintf(t_mess,"shoves %s to the ground!",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," shoves you to the ground!");
                   semote(user,other_user);
                   }
                   break;
           case 26: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"slaps themself.");    
                   else
                    sprintf(t_mess,"slaps %s! *SMACK*",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," slaps you!");
                   semote(user,other_user);
                   }
                   break;
           case 27: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"whines to themself.");    
                   else
                    sprintf(t_mess,"whines to %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," whines to you");
                   semote(user,other_user);
                   }
                   break;
           case 28: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"winks at themself.");    
                   else
                    sprintf(t_mess,"winks at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," winks at you");
                   semote(user,other_user);
                   }
                   break;
           case 29: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"WOOHOO's to themself.");    
                   else
                    sprintf(t_mess,"WOOHOO's at %s",name);

                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," WOOHOO's at you");
                   semote(user,other_user);
                   }
                   break;
           case 30: if (in_room) {
                   if (user==u)
                    strcpy(t_mess,"examines a piece of chicken then hits themselves in the face with it repeatedly.");
                   else  
                    sprintf(t_mess,"flings a piece of chicken at %s!",name);
                    
                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," flings a piece of chicken at you!");
                   semote(user,other_user);
                   }
                   break;
           case 31: if (in_room) {
                   if (user==u)
                     strcpy(t_mess,"puts themselves in a headlock and tries to perform a self-noogie. What's this person on?");
                   else
                     sprintf(t_mess,"puts %s in a headlock and gives them a noogie!",name);
                   
                   emote(user,t_mess);
                   }
                   else {
                   strcat(other_user," gives you a noogie!");
                   semote(user,other_user);
                   }
                   break;
           case 32: if (in_room) {
                   if (user==u)
                     strcpy(t_mess,"twists around, grabs some underwear, and pulls them straight up..going head over heals in the process!");
                   else
                     sprintf(t_mess,"runs behind %s and pulls their underwear up over their head. Atomic wedgie!",name);
                   
                   emote(user,t_mess);
                   my_echo(user,"Now doesn't that feel goooooood?\0");
                   }
                   else {
                   strcat(other_user," pulls your underwear up over your head for a patented atmoic wedgie! Now doesn't that feel goooooood?");
                   semote(user,other_user);
                   }   
                   break;
           default: return;  break;
       }  /* end of switch */
   }  /* end of else */

}

