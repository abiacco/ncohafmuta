#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_CONFIG_FILE "running_config.dat"

#define GLOBALS		0
#define ROOMS		1
#define RESTRICTS	2
#define RANKS		3

struct rank_struct {
	char name[40];
	char shortname[20];
	int level;
	int odds;
	struct rank_struct *prev,*next;
};
struct rank_struct *rank_first,*rank_last;

struct restrict_struct {
	int type;
	int subtype;
	char match[128];
	char doneby[21];
	char banner[256];
	char comment[256];
	time_t timestamp;
	struct restrict_struct *prev,*next;
	};
struct restrict_struct *restrict_first,*restrict_last;

struct room_struct_exits {
	char name[80];
	struct room_struct_exits *prev,*next;
	};
struct room_struct_exits *exit_first,*exit_last;

struct room_struct_flags {
	char name[25];
	struct room_struct_flags *prev,*next;
	};
struct room_struct_flags *flag_first,*flag_last;

struct room_struct {
	char name[40];
	char owner[40];
	char descfile[256];
	struct room_struct_exits *exit;
	struct room_struct_flags *flag;
	int security;
	int boardsecurity;
	int atmos_timer;
	int atmos_factor;
	struct room_struct *prev,*next;
	};
struct room_struct *room_first,*room_last;

struct config_struct {
	char name[256];
	int type;
	int valuedata;
	char stringdata[256];
	struct config_struct *prev,*next;
	};
struct config_struct *run_config_first,*run_config_last;

void init_config_struct(struct config_struct *ptr);
void init_room_struct(struct room_struct *ptr);
void init_room_struct_exits(struct room_struct_exits *ptr);
void init_room_struct_flags(struct room_struct_flags *ptr);
void init_restrict_struct(struct restrict_struct *ptr);
void init_rank_struct(struct rank_struct *ptr);
void remove_first(char *inpstr);
int check_config_opt(int mode, char *inpstr);

int main(void) {
int aa=0;
char configtype[256];
char configname[256];
char line[512];
char tempvalue[512];
char *tempptr;
FILE *fp;
struct config_struct *config_ptr;
struct room_struct *room_ptr;
struct restrict_struct *restrict_ptr;
struct room_struct_exits *room_exit_ptr;
struct rank_struct *rank_ptr;
struct config_struct *tempconfig;

 if (!(fp=fopen(DEF_CONFIG_FILE,"r"))) {
	printf("Can't open config file!\n");
	return 0;
	}

 while (fgets(line,512,fp) != NULL) {
  line[strlen(line)-1]=0;
  if (line[0]=='#' || !strlen(line)) continue;
  sleep(0);
  printf("READ LINE: %s\n",line);
  sscanf(line,"%s",configtype);
  remove_first(line);

   if (!strcmp(configtype,"value")) {
    sscanf(line,"%s",configname);
    remove_first(line);
    strcpy(tempvalue,line);
    printf("PARSED: %s %s %s\n",configtype,configname,tempvalue);

    if ((config_ptr=(struct config_struct *)malloc(sizeof(struct config_struct)))==NULL) {
        printf("Malloc failed for config_struct\n");
        return 0;
        }

    if (run_config_first==NULL) {
     run_config_first=config_ptr;
     config_ptr->prev=NULL;
    }
    else {
     run_config_last->next=config_ptr;
     config_ptr->prev=run_config_last;
    }
    config_ptr->next=NULL;
    run_config_last=config_ptr;

    init_config_struct(config_ptr);

    strncpy(config_ptr->name,configname,sizeof(config_ptr->name));
    config_ptr->type=1;
    config_ptr->valuedata=atoi(tempvalue);

    printf("VALUES: NAME %s TYPE %d VALUE %d\n",config_ptr->name,config_ptr->type,config_ptr->valuedata);
	for (tempconfig=run_config_first;tempconfig!=NULL;tempconfig=tempconfig->next) {
	 if (tempconfig->type==1) printf("V %s %d\n",tempconfig->name,tempconfig->valuedata);
	 else if (tempconfig->type==2) printf("S %s %s\n",tempconfig->name,tempconfig->stringdata);
	}
   } /* value */
   else if (!strcmp(configtype,"string")) {
    sscanf(line,"%s",configname);
    remove_first(line);
    strcpy(tempvalue,line);
    printf("PARSED: %s %s %s\n",configtype,configname,tempvalue);

    if ((config_ptr=(struct config_struct *)malloc(sizeof(struct config_struct)))==NULL) {
        printf("Malloc failed for config_struct\n");
        return 0;
        }

    if (run_config_first==NULL) {
     run_config_first=config_ptr;
     config_ptr->prev=NULL;
    }
    else {
     run_config_last->next=config_ptr;
     config_ptr->prev=run_config_last;
    }
    config_ptr->next=NULL;
    run_config_last=config_ptr;

    init_config_struct(config_ptr);

    strncpy(config_ptr->name,configname,sizeof(config_ptr->name));
    config_ptr->type=2;
    strncpy(config_ptr->stringdata,tempvalue,sizeof(config_ptr->stringdata));

    printf("VALUES: NAME %s TYPE %d VALUE %s\n",config_ptr->name,config_ptr->type,config_ptr->stringdata);
	for (tempconfig=run_config_first;tempconfig!=NULL;tempconfig=tempconfig->next) {
	 if (tempconfig->type==1) printf("V %s %d\n",tempconfig->name,tempconfig->valuedata);
	 else if (tempconfig->type==2) printf("S %s %s\n",tempconfig->name,tempconfig->stringdata);
	}
   } /* string */
   else if (!strcmp(configtype,"room")) {
    
    if ((room_ptr=(struct room_struct *)malloc(sizeof(struct room_struct)))==NULL) {
        printf("Malloc failed for room_struct\n");
        return 0;
        }

    if (room_first==NULL) {
     room_first=room_ptr;
     room_ptr->prev=NULL;
    }
    else {
     room_last->next=room_ptr;
     room_ptr->prev=room_last;
    }
    room_ptr->next=NULL;
    room_last=room_ptr;

    init_room_struct(room_ptr);

while (strlen(line)) {
    ROOMSTART:
    sscanf(line,"%s",configname);
    remove_first(line);
    printf("PARSED: %s %s %s\n",configtype,configname,line);
	if (!strcmp(configname,"NAME")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		 remove_first(line);
		 strncpy(room_ptr->name,tempvalue,sizeof(room_ptr->name));
		 printf("ROOM NAME: %s\n",room_ptr->name);
		}
	} /* if NAME */
	else if (!strcmp(configname,"OWNER")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		 remove_first(line);
		 strncpy(room_ptr->owner,tempvalue,sizeof(room_ptr->owner));
		 printf("ROOM OWNER: %s\n",room_ptr->owner);
		}
	} /* if OWNER */
	else if (!strcmp(configname,"DESC")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		 remove_first(line);
		 strncpy(room_ptr->descfile,tempvalue,sizeof(room_ptr->descfile));
		 printf("ROOM DESCFILE: %s\n",room_ptr->descfile);
		}
	} /* if DESC */
	else if (!strcmp(configname,"SECURITY")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		 remove_first(line);
		 if (!strcmp(tempvalue,"public")) room_ptr->security=0;
		 else if (!strcmp(tempvalue,"private")) room_ptr->security=1;
		 else if (!strcmp(tempvalue,"lockedpublic")) room_ptr->security=2;
		 printf("ROOM SECURITY: %d\n",room_ptr->security);
		}
	} /* if SECURITY */
	else if (!strcmp(configname,"EXITS")) {
	   while(sscanf(line,"%s",tempvalue)==1) {
		if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		remove_first(line);
	    if ((room_ptr->exit=(struct room_struct_exits *)malloc(sizeof(struct room_struct_exits)))==NULL) {
	        printf("Malloc failed for room_struct_exits\n");
	        return 0;
            }

	    if (exit_first==NULL) {
	     exit_first=room_ptr->exit;
	     room_ptr->exit->prev=NULL;
	    }
	    else {
	     exit_last->next=room_ptr->exit;
	     room_ptr->exit->prev=exit_last;
	    }
	    room_ptr->exit->next=NULL;
	    exit_last=room_ptr->exit;

	    init_room_struct_exits(room_ptr->exit);

		 strncpy(room_ptr->exit->name,tempvalue,sizeof(room_ptr->exit->name));
		 printf("ROOM EXIT: %s\n",room_ptr->exit->name);
	  } /* while */
	} /* if EXITS */
	else if (!strcmp(configname,"FLAGS")) {
	   while(sscanf(line,"%s",tempvalue)==1) {
		if (check_config_opt(ROOMS,tempvalue)) goto ROOMSTART;
		remove_first(line);
	    if ((room_ptr->flag=(struct room_struct_flags *)malloc(sizeof(struct room_struct_flags)))==NULL) {
	        printf("Malloc failed for room_struct_flags\n");
	        return 0;
            }

	    if (flag_first==NULL) {
	     flag_first=room_ptr->flag;
	     room_ptr->flag->prev=NULL;
	    }
	    else {
	     flag_last->next=room_ptr->flag;
	     room_ptr->flag->prev=flag_last;
	    }
	    room_ptr->flag->next=NULL;
	    flag_last=room_ptr->flag;

	    init_room_struct_flags(room_ptr->flag);

		 strncpy(room_ptr->flag->name,tempvalue,sizeof(room_ptr->flag->name));
		 printf("ROOM FLAG: %s\n",room_ptr->flag->name);
	   } /* while */
	} /* if FLAGS */
	else if (!strcmp(configname,"MESSBOARD")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 remove_first(line);
		 if (!strcmp(tempvalue,"changeable")) room_ptr->boardsecurity=0;
		 else if (!strcmp(tempvalue,"readable")) room_ptr->boardsecurity=1;
		 printf("ROOM BOARDSECURITY: %d\n",room_ptr->boardsecurity);
		}
	} /* if MESSBOARD */
	else if (!strcmp(configname,"ATMOSFACTOR")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 remove_first(line);
		 room_ptr->atmos_factor=atoi(tempvalue);
		 printf("ROOM ATMOSFACTOR: %d\n",room_ptr->atmos_factor);
		}
	} /* if MESSBOARD */

} /* while strlen */

   } /* room */
   else if (!strcmp(configtype,"restrict")) {
   /* restrict TYPE site MATCH x.x.x.x SUBTYPE existing-users|new-users|both 
      BANNER login banner COMMENT restrict comment TIME 1000000000 DONEBY cygnus */
   /* restrict TYPE user MATCH username SUBTYPE exact|partial BANNER login banner
      COMMENT restrict comment TIME 1000000000 DONEBY cygnus */

    if ((restrict_ptr=(struct restrict_struct *)malloc(sizeof(struct restrict_struct)))==NULL) {
        printf("Malloc failed for restrict_struct\n");
        return 0;
        }

    if (restrict_first==NULL) {
     restrict_first=restrict_ptr;
     restrict_ptr->prev=NULL;
    }
    else {
     restrict_last->next=restrict_ptr;
     restrict_ptr->prev=restrict_last;
    }
    restrict_ptr->next=NULL;
    restrict_last=restrict_ptr;

    init_restrict_struct(restrict_ptr);

while (strlen(line)) {
    RESTRICTSTART:
    sscanf(line,"%s",configname);
    remove_first(line);
    printf("PARSED: %s %s %s\n",configtype,configname,line);
    if (!strcmp(configname,"TYPE")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
		  if (!strcmp(tempvalue,"site")) restrict_ptr->type=0;
		  else if (!strcmp(tempvalue,"user")) restrict_ptr->type=1;
                 printf("RESTRICT TYPE: %d\n",restrict_ptr->type);
		} /* else */
    } /* if TYPE */
    else if (!strcmp(configname,"SUBTYPE")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
		 if (restrict_ptr->type==0) {
		  if (!strcmp(tempvalue,"existing-users")) restrict_ptr->subtype=0;
		  else if (!strcmp(tempvalue,"new-users")) restrict_ptr->subtype=1;
		  else if (!strcmp(tempvalue,"all-users")) restrict_ptr->subtype=2;
		 }
		 else if (restrict_ptr->type==1) {
		  if (!strcmp(tempvalue,"exact")) restrict_ptr->subtype=0;
		  else if (!strcmp(tempvalue,"partial")) restrict_ptr->subtype=1;
		 }
                 printf("RESTRICT SUBTYPE: %d\n",restrict_ptr->subtype);
                } /* else */
    } /* if SUBTYPE */
    else if (!strcmp(configname,"MATCH")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
                 strncpy(restrict_ptr->match,tempvalue,sizeof(restrict_ptr->match));
                 printf("RESTRICT MATCH: %s\n",restrict_ptr->match);
                } /* else */
    } /* if SITE or USER */
    else if (!strcmp(configname,"DONEBY")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
                 strncpy(restrict_ptr->doneby,tempvalue,sizeof(restrict_ptr->doneby));
                 printf("RESTRICT DONEBY: %s\n",restrict_ptr->doneby);
                }
    } /* if DONEBY */
    else if (!strcmp(configname,"TIME")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
		 restrict_ptr->timestamp=strtoul(tempvalue, (char **)NULL, 10);
                 printf("RESTRICT TIME: %u\n",restrict_ptr->timestamp);
                }
    } /* if TIME */
    else if (!strcmp(configname,"BANNER")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
                 strncpy(restrict_ptr->banner,tempvalue,sizeof(restrict_ptr->banner));
                 printf("RESTRICT BANNER: %s\n",restrict_ptr->banner);
                }
    } /* if BANNER*/
    else if (!strcmp(configname,"COMMENT")) {
                if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
                else {
                 if (check_config_opt(RESTRICTS,tempvalue)) goto RESTRICTSTART;
                 remove_first(line);
                 strncpy(restrict_ptr->comment,tempvalue,sizeof(restrict_ptr->comment));
                 printf("RESTRICT COMMENT: %s\n",restrict_ptr->comment);
                }
    } /* if COMMENT */

} /* while strlen */

   } /* restrict */
   else if (!strcmp(configtype,"rank")) {
    
    if ((rank_ptr=(struct rank_struct *)malloc(sizeof(struct rank_struct)))==NULL) {
        printf("Malloc failed for rank_struct\n");
        return 0;
        }

    if (rank_first==NULL) {
     rank_first=rank_ptr;
     rank_ptr->prev=NULL;
    }
    else {
     rank_last->next=rank_ptr;
     rank_ptr->prev=rank_last;
    }
    rank_ptr->next=NULL;
    rank_last=rank_ptr;

    init_rank_struct(rank_ptr);

while (strlen(line)) {
    RANKSTART:
    sscanf(line,"%s",configname);
    remove_first(line);
    printf("PARSED: %s %s %s\n",configtype,configname,line);
	if (!strcmp(configname,"NAME")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(RANKS,tempvalue)) goto RANKSTART;
		 remove_first(line);
		 strncpy(rank_ptr->name,tempvalue,sizeof(rank_ptr->name));
		 printf("RANK NAME: %s\n",rank_ptr->name);
		}
	} /* if NAME */
	else if (!strcmp(configname,"SHORTNAME")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(RANKS,tempvalue)) goto RANKSTART;
		 remove_first(line);
		 strncpy(rank_ptr->shortname,tempvalue,sizeof(rank_ptr->shortname));
		 printf("RANK SHORTNAME: %s\n",rank_ptr->shortname);
		}
	} /* if SHORTNAME */
	else if (!strcmp(configname,"LEVEL")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(RANKS,tempvalue)) goto RANKSTART;
		 remove_first(line);
		 rank_ptr->level=atoi(tempvalue);
		 printf("RANK LEVEL: %d\n",rank_ptr->level);
		}
	} /* if LEVEL */
	else if (!strcmp(configname,"ODDS")) {
		if (sscanf(line,"%s",tempvalue) != 1) printf("BAD!\n");
		else {
		 if (check_config_opt(RANKS,tempvalue)) goto RANKSTART;
		 remove_first(line);
		 rank_ptr->odds=atoi(tempvalue);
		 printf("RANK ODDS: %d\n",rank_ptr->odds);
		}
	} /* if ODDS */

} /* while strlen */

  } /* rank */

 } /* while fgets */

printf("--- GLOBAL CONFIG ---\n");
for (config_ptr=run_config_first;config_ptr!=NULL;config_ptr=config_ptr->next) {
	printf("%s %d %d %s\n",config_ptr->name,config_ptr->type,config_ptr->valuedata,config_ptr->stringdata);
}
printf("--- ROOM CONFIG ---\n");
for (room_ptr=room_first;room_ptr!=NULL;room_ptr=room_ptr->next) {
	printf("%s %s\n",room_ptr->name,room_ptr->owner);
	printf("\t--- ROOM EXITS ---\n");
	for (room_ptr->exit=exit_first;room_ptr->exit!=NULL;room_ptr->exit=room_ptr->exit->next)
	 printf("\t%s\n",room_ptr->exit->name);
	printf("\t--- ROOM FLAGS ---\n");
	for (room_ptr->flag=flag_first;room_ptr->flag!=NULL;room_ptr->flag=room_ptr->flag->next)
	 printf("\t%s\n",room_ptr->flag->name);
}
printf("--- RESTRICT CONFIG ---\n");
for (restrict_ptr=restrict_first;restrict_ptr!=NULL;restrict_ptr=restrict_ptr->next) {
	printf("TYPE   %d SUBTYPE %d\n",restrict_ptr->type,restrict_ptr->subtype);
	printf("MATCH  %s\n",restrict_ptr->match);
	printf("DONEBY %s\n",restrict_ptr->doneby);
	printf("TIME   %u\n",restrict_ptr->timestamp);
	printf("BANNER %s\n",restrict_ptr->banner);
	printf("COMENT %s\n",restrict_ptr->comment);
}
printf("--- RANK CONFIG ---\n");
for (rank_ptr=rank_first;rank_ptr!=NULL;rank_ptr=rank_ptr->next) {
	printf("LEVEL %d\n",rank_ptr->level);
	printf("\tNAME      %s\n",rank_ptr->name);
	printf("\tSHORTNAME %s\n",rank_ptr->shortname);
	printf("\tODDS      %d\n",rank_ptr->odds);
}


} /* main */



void init_config_struct(struct config_struct *ptr) {

ptr->name[0]='\0';
ptr->type=0;
ptr->valuedata=0;
ptr->stringdata[0]='\0';

}

void init_room_struct(struct room_struct *ptr) {

ptr->name[0]='\0';
ptr->owner[0]='\0';
ptr->descfile[0]='\0';
ptr->exit=NULL;
ptr->security=0;	/* public */
ptr->boardsecurity=0;	/* changeable */
ptr->atmos_timer=5;
ptr->atmos_factor=5;

}

void init_room_struct_exits(struct room_struct_exits *ptr) {

ptr->name[0]='\0';

}

void init_room_struct_flags(struct room_struct_flags *ptr) {

ptr->name[0]='\0';

}

void init_restrict_struct(struct restrict_struct *ptr) {

ptr->type=0;
ptr->subtype=0;
ptr->match[0]='\0';
ptr->doneby[0]='\0';
ptr->banner[0]='\0';
ptr->comment[0]='\0';
ptr->timestamp=0;

}

void init_rank_struct(struct rank_struct *ptr) {

ptr->name[0]='\0';
ptr->shortname[0]='\0';
ptr->level=0;
ptr->odds=0;

}


/*** removes first word at front of string and moves rest down ***/
void remove_first(char *inpstr)
{
int newpos,oldpos;

newpos=0;  oldpos=0;
/* find first word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find end of first word */
while(inpstr[oldpos]!=' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find second word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
while(inpstr[oldpos]!=0)
        inpstr[newpos++]=inpstr[oldpos++];
inpstr[newpos]='\0';
}

int check_config_opt(int mode, char *inpstr) {

if (mode==GLOBALS) {

} /* GLOBALS */
else if (mode==ROOMS) {
	if (!strcmp(inpstr,"NAME") ||
	 !strcmp(inpstr,"DESC") ||
	 !strcmp(inpstr,"SECURITY") ||
	 !strcmp(inpstr,"MESSBOARD") ||
	 !strcmp(inpstr,"OWNER") ||
	 !strcmp(inpstr,"FLAGS") ||
	 !strcmp(inpstr,"EXITS")) return 1;
} /* ROOMS */
else if (mode==RESTRICTS) {
	if (!strcmp(inpstr,"TYPE") ||
	 !strcmp(inpstr,"SUBTYPE") ||
	 !strcmp(inpstr,"MATCH") ||
	 !strcmp(inpstr,"DONEBY") ||
	 !strcmp(inpstr,"TIME") ||
	 !strcmp(inpstr,"BANNER") ||
	 !strcmp(inpstr,"COMMENT")) return 1;
} /* RESTRICTS */
else if (mode==RANKS) {
	if (!strcmp(inpstr,"NAME") ||
	 !strcmp(inpstr,"SHORTNAME") ||
	 !strcmp(inpstr,"LEVEL") ||
	 !strcmp(inpstr,"ODDS")) return 1;
} /* RANKS */

return 0;
}

