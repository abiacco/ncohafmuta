/*----------------------------------------------------------------------*/
/* Now Come on Over Here And Fuck Me Up The Ass - Ncohafmuta V 1.4.x    */
/*----------------------------------------------------------------------*/
/*  This code is a collection of software that originally started       */
/*  as a system called:                                                 */
/*                       IFORMS V 1.0                                   */
/*            Interactive FORum Multiplexor Software - (C) Deep         */
/*                 Last update 25/9/94                                  */
/*                                                                      */
/* As a result of extensive changes, it can no longer be considered     */
/* the same code                                                        */
/*                     -Cygnus (Anthony J. Biacco - Ncohafmuta Ent.)    */
/*                                                                      */
/* Legal note:  This code may NOT be freely distributed.  Doing so may  */
/*              be in violation of the US Munitions laws which cover    */
/*              exportation of encoding technology.                     */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* For the programically-challenged..                                   */
/* If you need to put double quotes (") inside a #define string         */
/* i.e. a #define enclosed by quotes, you MUST escape the double quote  */
/* like this:  \"                                                       */
/* THIS IS WRONG: #define VARIABLE "This a my "first" string"           */
/* THIS IS RIGHT: #define VARIABLE "This a my \"first\" string"         */
/*----------------------------------------------------------------------*/
	
/* last modified: Feb 26th, 2002   Cygnus */

#ifndef _TEXT_H
#define _TEXT_H

/*-----------------------------------------------------------------*/
/* constants used for the fight command                            */
/*-----------------------------------------------------------------*/
                    
#define TIE           3
#define BOTH_LOSE     0
#define CLOSE_NUMBER 50   /* make this 100 -no ties, 0- all ties              */
#define FIGHT_ROOM    0   /* used for specifying a room to fight in, -1=any */
                          /* this is the room you CANT fight in */

#define CHAL_LINE "Dude!  You have been challenged to a fight.\07"
#define CHAL_LINE2 "Respond with: .fight [yes | no]"
#define CHAL_ISSUED "## You have issued the challenge..it is time to wait ##"

#ifdef _DEFINING_TEXT
char chal_text[][132] = {
  "%s has challenged %s to a fight.  Call the medics!",
  "%s takes a leather glove and slaps %s in the face...its a challenge to fight.",
  "%s pulls out a wand and says \"%s, make my day\".",
  "%s has had enough of %s...time to put on the gloves and settle this.",
  "%s shouts \"%s, you piece of scum, prepare to die.\"",
  "%s sucker punches %s.  Oh my...looks like a fight.",
  "%s draws a line in the dirt and says: \"%s, go ahead, cross me.\"",
  "%s and %s square off.  They look each in the eyes.  A SHOWDOWN, LOOK OUT",
  "%s grabs %s and says \"HEY Shorty, I'll knock your block off.\"",
  "%s asks %s, \"umm... you wanna step outside...Mister?!\"",
  "%s shouts \"%s's mother wears army boots!\"  Oh NO....a fight!",
  "%s just dropped a spider down %s's pants....looks like a fights about to start.",
  "%s pokes %s and says \"I could kick your butt with one hand behind my back!\""
  };
int num_chal_text = 13;
#else
extern char chal_text[][132];
extern int num_chal_text;
#endif

#ifdef _DEFINING_TEXT
char tie1_text[][132] = {
"There is a mutual agreement struck and both, %s and %s end the fight winners.",
"The crowd restrains both people and they cool off...It is over people, go home.",
"PSYCHED...Y'all thought we were fighting didn't you. %s shakes %s hand. Noway.",
"In a flash, both %s and %s knock each other out.  They awake forgetting to finish it.",
"%s and %s make up and decide to open a magic shop together instead.",
"%s cracks a joke and they both start laughing so hard...they cannot fight"
};
int num_tie1_text = 6;
#else
extern char tie1_text[][132];
extern int num_tie1_text;
#endif

#ifdef _DEFINING_TEXT
char tie2_text[][132] = {
"Wow! They killed each other at exactly the same time. How sad. ",
"Both %s and %s die....seems they got stuck together with crazy glue in the fight.",
"%s killed %s, but was lynched by a mob of on-lookers....the irony of it all.",
"A run-away bull runs over %s and %s while they were fighting in the street.  OOPS.",
"%s decides to go suicidal and pulls the pin on the gernade, killing %s as well.",
"%s and %s are mistaken as bloods by a group of crypts...and are vaporized.   OOPS",
"A plane drops out of the sky on top of both %s and %s killing them dead.  What luck."
};
int num_tie2_text = 7;
#else
extern char tie2_text[][132];
extern int num_tie2_text;
#endif

#ifdef _DEFINING_TEXT
char wins1_text[][132] = {
"%s is quick on the draw and pummels %s into the ground.",
"%s demonstrates expertise in martial arts and quickly destroys %s.",
"%s's initial attack must have been enough to allow for a fast kill of %s.",
"%s whips out a lightning bolt and jolts %s into the next universe.",
"%s grabs 2 stone paddles and cracks %s's head (like in the book Congo).",
"%s whips out an assault rifle and shoots %s dead.  How quaint.",
"%s was cleaning a gun and it went off 'accidentally', killing %s (oops).",
"%s shows %s some pet pirranhas ..... the rest is history.",
"%s hires a hitman and has %s killed....but it LOOKS like an accident.",
"%s's flesh melts away revealing a robot. The robot kills %s.",
"%s waits until %s is asleep.....and places a mad rattle snake in the bed.",
"%s ducks just in time to get missed by the sniper that was on the roof.  %s got it instead.",
"%s starts talking about gross stuff...%s had to run to the bathroom to puke.",
"%s pulls the plug on %s's computer.....tsk tsk tsk...no power, no net.",
"%s points and says \"Don't Look!\".  %s turns to look and is killed.  Told you not to look." 
};
int num_wins1_text = 15;
#else
extern char wins1_text[][132];
extern int num_wins1_text;
#endif

#ifdef _DEFINING_TEXT
char wins2_text[][132] = {
"%s accepts the challange to fight and pummels %s...so much for sneak attacks.",
"%s turns into a raving maniac and scares %s to death....good job.",
"%s shows %s a mirror.  Death was the result (embarrassment i think). ",
"%s kicks the crap out of %s....who now has to leave to goto the hospital.",
"%s jumps in a wagon and runs %s over....Road Pizza!!!",
"%s throws a banana peel under %s's foot....who then slips, falls, and dies.",
"%s takes %s swimming (who was just fitted with cement shoes).",
"%s scares %s bad...they have an accident and have to leave to get new clothes.",
"%s calls the gods.  They come and take %s away for threatening people.",
"%s summons unearthly forces and vaporizes %s.....Wow, get the popcorn.",
"%s forces %s to watch a Barney marathon... the result is not pretty.",
"%s introduces %s to their good friend, Bubba...'nough said.",
"%s grabs a steam roller and squishes %s like a pancake.",
"%s drops an anvil on top of %s's head.  OUCH.  That even looked like it hurt.",
"%s watches %s consume two beers and pass out. (cannot handle the stuff eh?)"
};
int num_wins2_text =15;
#else
extern char wins2_text[][132];
extern int num_wins2_text;
#endif

#ifdef _DEFINING_TEXT
char wimp_text[][132] = {
"%s wimps out of the fight.  What a wuss.",
"HEY LOOK!  %s is a total chicken and has backed out of the fight.",
"%s declines the fight...maybe some other day when i feel better.",
"The fight has been cancelled, %s had to goto assertiveness training.",
"%s got scared and called the admins.  No fight today.",
"%s didn't want to fight....must have been cowardice.",
"%s turns yellow with fear and begs forgiveness. (OK..no fight)",
"%s runs with their tail between thier legs...guess that means no fight.",
"%s does a fake ignore of the challenge to the fight..hoping no one sees it.",
"%s suddenly remembers a dental appointment..."
};
int num_wimp_text = 10;
#else
extern char wimp_text[][132];
extern int num_wimp_text;
#endif

/*----------------------*/
/* Random kill messages */
/*----------------------*/
#define NUM_KILL_MESSAGES 16
#ifdef _DEFINING_TEXT
char kill_text[][100] =
  {
  "A stream of electrons pulse through %s and turn them to vapor!!",
  "%s is destroyed by a god among us!!",
  "A large laser beam hits %s!!  Good bye..you're dead.",
  "Someone cleaning their magic wand accidentaly kills %s!!  OOPS!",
  "A hiding net monster jumps out and eats %s!!  Too bad!",
  "%s had to leave....someone did not like them!!",
  "%s had to go to incontenence training!!  Bye Bye.",
  "%s had to leave to get all gussied up in pink.  How cute!",
  "%s was just put out of our misery!",
  "A 2 ton weight just fell on %s!! Too bad this ain't a cartoon.",
  "%s just net.sexed themself to death!! Will you be next?",
  "One of the roaming children pukes on %s!!",
  "Two men in white arrive and throw %s in a van! You faintly hear, 'I'm not crazy! I'm NOT!!!'",
  "%s's bytes are suddenly scrambled!",
  "The gods have had enough!  %s has been disintegrated!!",
  "A bolt of lightening streaks from the heavens and blasts %s!!"
  };
#else
extern char kill_text[][100];
#endif
  
/*------------------------*/
/*  Random AFK messages   */
/*------------------------*/
#define NUM_IDLE_LINES 10
#ifdef _DEFINING_TEXT
char idle_text[][100] =
  {
  "- %s is knocked out by a low flying tell                     -%s",
  "- %s is hiding from the boss.   SSSSHHHH                     -%s",
  "- %s accidentally knocked themselves out with a tell.  Ouch. -%s",
  "- %s has to rush to the bathroom.. potty break time          -%s",
  "- %s is off doing something they don't want you to see       -%s",
  "- Um...er...%s is a bit pre-occupied at the moment           -%s",
  "- %s is off looking at the 'wild life'                       -%s",
  "- %s is out roaming the halls.                               -%s",
  "- %s fell into a trance. Weird. *snap* *snap* Yep, out cold. -%s",
  "- %s has just spilled their drink on their keyboard          -%s"
  };
#else
extern char idle_text[][100];
#endif

/*--------------------------*/
/*  Random 8ball messages   */
/*--------------------------*/
#define NUM_BALL_LINES 10
#ifdef _DEFINING_TEXT
char ball_text[][132] =
 {
 "No way man!",
 "What kind of moronic question is that?",
 "In a word - yes",
 "Go ask your mother",
 "Don't be silly",
 "Of course not",
 "Without a doubt",
 "Yes, yes.. A thousand times YES!",
 "I don't think so",
 "You waste my time with such petty questions?"
 };
#else
extern char ball_text[][132];
#endif

/* hangman picture for the hangman game */
#ifdef _DEFINING_TEXT
char *hanged[8]={
  "^HY+^^LY---^^HY+^  \n\r^LY|      ^\n\r^LY|^           ^HWWord:^ %s\n\r^LY|^           ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^           ^HWWord:^ %s\n\r^LY|^           ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^           ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^   |       ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^  /|       ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^  /|\\      ^HWLetters guessed:^ %s\n\r^LY|^      \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^  /|\\      ^HWLetters guessed:^ %s\n\r^LY|^  /   \n\r^LY|______^\n\r",
  "^HY+^^LY---^^HY+^  \n\r^LY|   |  ^\n\r^LY|^   O       ^HWWord:^ %s\n\r^LY|^  /|\\      ^HWLetters guessed:^ %s\n\r^LY|^  / \\ \n\r^LY|______^\n\r"
};
#else
extern char *hanged[8];
#endif

#endif /* _TEXT_H */
