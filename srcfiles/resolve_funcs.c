#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"


/*** Resolve sock address to ip or hostname ***/
void resolve_add(int user_wait, unsigned long addr, int mode)
{
int result=0;
char buf[256];
struct hostent *he;

/* Resolve sock address to hostname, if cant copy failed message */
if (mode==2) {
 if (resolve_names) {
 /* talker is using its own cache files instead */
 /* do the lookup in here                       */
  if (resolve_names!=3 && resolve_names!=4) result = cache_lookup(user_wait, addr);
  else result = -1;
  if (result <= -1) {
    /* this is whatever function you use to resolve a network */
    /* address to a hostname */
   if (resolve_names==2 || resolve_names==3) {
	/* CYGRESOLVE */
	send_resolver_request(user_wait, ustr[user_wait].site, "*");
	strcpy(ustr[user_wait].net_name, SYS_LOOK_PENDING);
	/* CYGRESOLVE */
   }
   else if (resolve_names==1 || resolve_names==4) {
    he = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    if (he && he->h_name)
       strcpy(ustr[user_wait].net_name, he->h_name);
    else
       strcpy(ustr[user_wait].net_name, SYS_LOOK_FAILED);
    if (resolve_names==1) add_to_resolver_cache(user_wait);
   }
  } /* end of result if */
 } /* end of resolve_names if */
} /* end of mode if */
/* Copy ip address to user structure */
else if (mode==1) {
 addr=ntohl(addr);
 buf[0]=0;
 sprintf(buf,"%ld.%ld.%ld.%ld", (addr >> 24) & 0xff, (addr >> 16) & 0xff,
         (addr >> 8) & 0xff, addr & 0xff);
 strcpy(ustr[user_wait].site, buf);
 }
}


/* Cache lookup */
int cache_lookup(int user, unsigned long addr2)
{
int found=0;
long timestamp=0;
char ipsite[30];
char hostsite[257];
char line[257];
char filename[FILE_NAME_LEN];
FILE *fp;

/* ok, are we gonna do a talker-wide or site-wide search? */
if (resolve_names) {
 /* ok, talker-wide */
 /* start with default file */
 sprintf(filename,"%s",SITECACHE_FILE_DEF);
 if (!(fp=fopen(filename,"r"))) { }
 else {
 while(fgets(line,256,fp) != NULL) {
        line[strlen(line)-1]=0;   
        sscanf(line,"%s",ipsite);
        remove_first(line);
	strncpy(hostsite,line,sizeof(hostsite));

      if (!strcmp(ipsite,ustr[user].site)) {
	strcpy(ustr[user].net_name,hostsite);
	found=1;
        system_stats.cache_hits++;
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"TALKER: cache_lookup: Found ip %s in default cache, returning success with %s\n",ipsite,hostsite);
#endif
        break;
        }
   } /* end of while */
 fclose(fp);
 line[0]=0;
 hostsite[0]=0;
 ipsite[0]=0;
 if (found) return 1;
 } /* end of default file lookup else */

 /* now main cache file */
 sprintf(filename,"%s",SITECACHE_FILE);
 if (!(fp=fopen(filename,"r"))) {
    return -1;
  }
 
 while(fgets(line,256,fp) != NULL) {
        line[strlen(line)-1]=0;   
        sscanf(line,"%s",ipsite);
        remove_first(line);
/* NEW */
	/* TTL timestamp */
	timestamp=0;
	sscanf(line,"%ld",&timestamp);
	remove_first(line);
	if (strlen(line)) strncpy(hostsite,line,sizeof(hostsite));
	else strncpy(hostsite,SYS_LOOK_FAILED,sizeof(hostsite));

      if (!strcmp(ipsite,ustr[user].site)) {
	if (!timestamp || ((time(0)-timestamp) <= MAX_DNS_TTL)) {
		/* if no timestamp, or cache entry is not too old */
	        strcpy(ustr[user].net_name,hostsite);
		found=1;
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"TALKER: cache_lookup: Found ip %s in cache, returning success with %s\n",ipsite,hostsite);
#endif
	}
	else {
	/* timestamp is past TTL, redo resolution */
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"TALKER: cache_lookup: Found ip %s in cache, but TTL of is up (%ld/%d). Re-doing resolution.\n",ipsite,(unsigned long)(time(0)-timestamp),MAX_DNS_TTL);
#endif
		if (resolve_names==1) found=2;
		else if (resolve_names==2) found=3;
	} /* else */
        system_stats.cache_hits++;
        break;
        } /* strcmp if matched */
   } /* end of while */
 fclose(fp);
 line[0]=0;
 if (found==1) {
		/* found match in cache, returning success */
		return 1;
	}
 else if (found==2) {
                /* could not do this above, because we were in  */
                /* the middle of reading the cache file         */
		/* delete the entry from cache because we are	*/
		/* going to re-resolve it when we return error	*/
                del_from_resolver_cache(user);
		/* return -1 to get it to redo resolution */
		return -1;
	}
 else if (found==3) {
		/* return -1 to get it to redo resolution */
		return -1;
	}
/* found must be 0 here or we would have returned in the previous */
/* ifs */
return -2;
} /* end of resolve_names if */

return 1;
}


/* add ip and resolved hostname to our local cache file */
void add_to_resolver_cache(int user) {
int lines=0;
int full=0;
char line[257];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;
FILE *fp2;

    sprintf(filename,"%s",SITECACHE_FILE);
    /* Check for empty slot in cache */
    lines=file_count_lines(filename);
    if (lines >= SITECACHE_SIZE) full=1;
    if (!full) {
    if (!(fp=fopen(filename,"a"))) return;
    sprintf(line,"%s %ld %s",ustr[user].site,(unsigned long)time(0),ustr[user].net_name);
    fputs(line,fp);
    fputs("\n",fp);
    fclose(fp);
    system_stats.cache_misses++;
    return;
    }
  else {
   /* Cache full, bump site at top and move all others up to make */
   /* room for new site at end */
   strcpy(filename2,get_temp_file());
   if (!(fp2=fopen(filename2,"w"))) return;
   if (!(fp=fopen(filename,"r"))) { fclose(fp2); return; }
   /* get rid of first entry because we write from second one on */
   fgets(line,256,fp);
   line[0]=0;
   while (fgets(line,256,fp) != NULL) {
        fputs(line,fp2);
     } /* end of while */
   fclose(fp);
   sprintf(line,"%s %ld %s",ustr[user].site,(unsigned long)time(0),ustr[user].net_name);
   fputs(line,fp2);
   fputs("\n",fp2);
   fclose(fp2);
   remove(filename);
   rename(filename2,filename);
   system_stats.cache_misses++;  
   } /* end of full else */  

}


/* delete ip and hostnamt from our local cache file */
void del_from_resolver_cache(int user) {
char ipsite[30];
char line[257];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;
FILE *fp2;

   sprintf(filename,"%s",SITECACHE_FILE);
   strcpy(filename2,get_temp_file());
   if (!(fp=fopen(filename,"r"))) {
	return;
   }
   if (!(fp2=fopen(filename2,"w"))) {
	fclose(fp);
	return;
   }
   while (fgets(line,256,fp) != NULL) {
	sscanf(line,"%s",ipsite);
	if (!strcmp(ustr[user].site,ipsite)) continue;
        fputs(line,fp2);
     } /* end of while */
   fclose(fp);
   fclose(fp2);
   remove(filename);
   rename(filename2,filename);
}
