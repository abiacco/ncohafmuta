#ifndef _PROTOS_H
#define _PROTOS_H

/* Precious declaration reads                   */
/* Yours is yours and mine you leave alone now  */
/* Precious declaration says                    */
/* I believe all hope is dead no longer         */

/* probably don't need to include this anymore, since more	*/
/* files that include us, include includes.h first, which takes	*/
/* care of this							*/
/* #include <time.h> */

#if defined(__sun__)
 /* SunOS of some sort */
  #if defined(__svr4__) || defined(__SVR4)
  /* Solaris, i.e. SunOS 5.x */
   #define SOL_SYS
  #endif
#endif

/* Can't crash now I've been waiting for this   */
void realloc_str(char** strP, int size);

void write_cygnus(int user);
RETSIGTYPE sigcall(int sig);
void init_signals(void);
RETSIGTYPE zombie_killer(int sig);
void shutdown_error(char *message);
void web_error(int user, int header, int mode);
RETSIGTYPE handle_sig(int sig);
void abbrcount(void);
void read_init_data(int mode);
void read_exem_data(void);
void read_nban_data(void);
void make_sockets(void);
void init_user_struct(void);
void init_area_struct(void);
void init_misc_struct(void);
void check_mess(int startup);
void say(int user, char *inpstr, int mode);
void check_total_users(int mode);
void messcount(void);
void sysud(int ud, int user);
void reset_chal(int user, char *inpstr);
void reset_alarm(void);
void cbtbuff(void);
void cshbuff(void);
void free_sock(int user, char port);
void external_who(int as);
void write_it(int sock, char *str);
void write_str(int user, char *str);
void write_str_www(int user, char *str, int size);
void writeall_str(char *str, int area, int user, int send_to_user, int who_did, int mode, int type, int sw);
void user_quit(int user, int mode);
void strtolower(char *str);
void telnet_echo_on(int user);
void telnet_echo_off(int user);
void check_alert(int user, int mode);
void my_login(int user, char *inpstr);
void set_profile(int user, char *inpstr);
void talker(int user, char *inpstr);
void enter_votedesc(int user, char *inpstr);
void descroom(int user, char *inpstr);
void exec_bot_com(int com_num, int user, char *inpstr);
void exec_com(int com_num, int user, char *inpstr);
void clear_mail(int user, char *inpstr);
void shutdown_d(int user, char *inpstr);
void parse_input(int user, char *inpstr);
void do_telnet_commands(int user);
void terminate(int user, char *str);
void review(int user);
void resolve_add(int user_wait, unsigned long addr, int mode);
void reset_userfors(int startup);
void remove_junk(int startup);
void auto_restrict(int user);
void quotes(int user);
void delete_verify(int user);
void listen_all(int user);
void write_str_nr(int user, char *str);
void midcpy(char *strf, char *strt, int fr, int to);
void get_rwho(int user, char *host, int port, char *info, int type);
void add_user(int user);
void add_user2(int user, int mode);
void t_who(int user, char *inpstr, int mode);
void attempts(int user);
void init_user(int user);
void write_hilite(int user, char *str);
void add_hilight(char *str);
void write_raw(int user, unsigned char *str, int len);
void look(int user, char *inpstr);
void check_mail(int user);
void syssign(int user, int onoff);
void remove_first(char *inpstr);
void st_crypt(char *str);
void clear_sent(int user, char *inpstr);
void write_user(char *name);
void newwho(int user);
void copy_from_user(int user);
void btell(int user, char *inpstr);
void cls(int user);
void shout(int user, char *inpstr);
void tell_usr(int user, char *inpstr, int mode);
void user_listen(int user, char *inpstr);
void user_ignore(int user, char *inpstr);
void go(int user, char *inpstr, int user_knock);
void room_access(int user, int priv);
void invite_user(int user, char *inpstr);
void emote(int user, char *inpstr);
void rooms(int user, char *inpstr);
void write_board(int user, char *inpstr, int mode);
void read_board(int user, int mode, char *inpstr);
void wipe_board(int user, char *inpstr, int wizard);
void set_topic(int user, char *inpstr);
void kill_user(int user, char *inpstr);
void search_boards(int user, char *inpstr);
void help(int user, char *inpstr);
void broadcast(int user, char *inpstr);
void system_status(int user);
void move(int user, char *inpstr);
void system_access(int user, char *inpstr, int co);
void toggle_atmos(int user, char *inpstr);
void my_echo(int user, char *inpstr);
void set_desc(int user, char *inpstr);
void toggle_allow(int user, char *inpstr);
void greet(int user, char *inpstr);
void arrest(int user, char *inpstr, int mode);
void cbuff2(int user, char *inpstr);
void socials(int user, char *inpstr, int type);
void macros(int user, char *inpstr);
void read_mail(int user, char *inpstr);
void send_mail(int user, char *inpstr, int mode);
void promote(int user, char *inpstr);
void demote(int user, char *inpstr);
void muzzle(int user, char *inpstr, int type);
void unmuzzle(int user, char *inpstr);
void bring(int user, char *inpstr);
void hide(int user, char *inpstr);
void display_ranks(int user);
void restrict(int user, char *inpstr, int type);
void unrestrict(int user, char *inpstr, int type);
void picture(int user, char *inpstr);
void preview(int user, char *inpstr);
void password(int user, char *pword);
void permission_u(int user, char *inpstr);
void semote(int user, char *inpstr);
void tog_monitor(int user);
void ptell(int user, char *inpstr);
void follow(int user, char *inpstr);
void beep_u(int user, char *inpstr);
void set_afk(int user, char *inpstr);
void systime(int user, char *inpstr);
void print_users(int user, char *inpstr);
void usr_stat(int user, char *inpstr, int mode);
void set(int user, char *inpstr);
void swho(int user, char *inpstr);
void nuke(int user, char *inpstr, int mode);
void set_bafk(int user, char *inpstr);
void clist(int user, char *inpstr);
void vote(int user, char *inpstr);
void force_user(int user, char *inpstr);
void readlog(int user, char *inpstr);
void home_user(int user);
void nerf(int user, char *inpstr);
void fight_another(int user, char *inpstr);
void meter(int user, char *inpstr);
void set_quota(int user, char *inpstr);
void xcomm(int user, char *inpstr);
void think(int user, char *inpstr);
void sos(int user, char *inpstr);
void show(int user, char *inpstr);
void cline(int user, char *inpstr);
void bbcast(int user, char *inpstr);
void version(int user);
void shemote(int user, char *inpstr);
void suname(int user, char *inpstr);
void supass(int user, char *inpstr);
void set_entermsg(int user, char *inpstr);
void abbrev(int user, char *inpstr);
void last_u(int user, char *inpstr);
void bubble(int user);
void sthink(int user, char *inpstr);
void where(int user, char *inpstr);
void call(int user, char *inpstr);
void creply(int user, char *inpstr);
void set_fail(int user, char *inpstr);
void set_succ(int user, char *inpstr);
void mutter(int user, char *inpstr);
void fmail(int user, char *inpstr);
void swipe(int user, char *inpstr);
void anchor_user(int user, char *inpstr);
void quote_op(int user, char *inpstr);
void list_last(int user, char *inpstr);
void real_user(int user, char *inpstr);
void pukoolsn(int user, char *inpstr);
void regnif(int user, char *inpstr);
void siohw(int user, char *inpstr);
void read_sent(int user, char *inpstr);
void same_site(int user, char *inpstr);
void gag(int user, char *inpstr);
void alert(int user, char *inpstr);
void schedule(int user);
void sing(int user, char *inpstr);
void show_expire(int user, char *inpstr);
void set_exitmsg(int user, char *inpstr);
void reload(int user);
void list_socs(int user);
void check_mem(int user);
void add_atmos(int user, char *inpstr);
void del_atmos(int user, char *inpstr);
void list_atmos(int user);
void shout_think(int user, char *inpstr);
void suicide_user(int user, char *inpstr);
void say_to_user(int user, char *inpstr);
void gag_comm(int user, char *inpstr, int type);
void frog_user(int user, char *inpstr);
void auto_com(int user, char *inpstr);
void eight_ball(int user, char *inpstr);
void warning(int user, char *inpstr, int mode);
void banname(int user, char *inpstr);
void player_create(int user, char *inpstr);
void revoke_com(int user, char *inpstr);
void bot_whoinfo(int user, char *inpstr);
void remove_user(char *name);
void cbuff(int user);
void not_signed_on(int user, char *name);
void knock(int user, int new_area);
void inedit_file(int user, char *inpstr, int line_num, int mode);
void check_promote(int user, int mode);
void inedit_file2(int user, char *inpstr, int line_num, int mode);
void auto_nuke(int user);
void get_bounds_to_delete(char *str, int *lower, int *upper, int *mode);
void auto_expr(int user);
void auto_prom(int user);
void remove_lines_from_file(int user, char *file, int lower, int upper);
void shutdown_auto(void);
void ctellbuff(int user);
void command_disabled(int user);
void print_dir(int user, char *inpstr, char *s_search);
void check_shut(void);
void check_idle(void);
void atmospherics(void);
void write_area(int area, char *inpstr);
void write_meter(int mode);
void copy_to_user(int user);
void get_buf(FILE *f, char *buf2, int buflen);
void putbuf(FILE *f, char *buf2);
void catall(int user, char *filename);
void write_log(int type, int wanttime, char *str, ...);
void print_ban_dir(int user, char *inpstr, char *s_search);
void set_email(int user, char *inpstr);
void set_homepage(int user, char *inpstr);
void set_webpic(int user, char *inpstr);
void set_sex(int user, char *inpstr);
void set_rows(int user, char *inpstr);
void set_cols(int user, char *inpstr);
void set_car_ret(int user, char *inpstr);
void set_atmos(int user, char *inpstr);
void set_abbrs(int user, char *inpstr);
void set_white_space(int user, char *inpstr);
void set_hilite(int user, char *inpstr);
void set_hidden(int user, char *inpstr);
void set_pause(int user);
void set_pbreak(int user, char *inpstr);
void set_beep(int user, char *inpstr);
void set_recap(int user, char *inpstr);
void set_home(int user, char *inpstr);
void set_color(int user, char *inpstr);
void set_visemail(int user);
void set_help(int user);
void set_who(int user);
void set_icq(int user, char *inpstr);
void set_autoread(int user);
void set_autofwd(int user);
void will_time_mark(int user);
void issue_chal(int user, int user2);
void accept_chal(int user);
void file_copy_lines(FILE *in_file, FILE *out_file, int lines);
void file_skip_lines(FILE *in_file, int lines);
void file_copy(FILE *in_file, FILE *out_file);
void fill_bar(int val1, int val2, char *str);
void ignore_all(int user);
void external_www(int user);
void external_users(int user, int mode, char *query, int photofilter);
void telnet_ask_tuid(int user);
void telnet_neg_ttype(int user, int mode);
void proc_dont(int user);
void proc_do(int user);
void proc_wont(int user);
void proc_will(int user);
void proc_sb(int user);
void junk1(int user, char *inpstr);
void resolve_names_set(int user, char *inpstr);
void grant_com(int user, char *inpstr);
void listall_gravokes(int user, int mode);
void write_hilite(int user, char *str);
void write_raw(int user, unsigned char *str, int len);
void write_it(int sock, char *str);
void write_hilite_nr(int user, char *str);
void write_bot(char *fmt);
void write_bot_nr(char *fmt);
void write_str(int user, char *str);
void write_str_nr(int user, char *str);
void reset_user_struct(int user, int mode);
void telnet_ask_eor(int user);
void telnet_write_eor(int user);
void initabbrs(int user);
void copy_abbrs(int user, int ref);
void add_abbrs(int user, int ref, int num);
void auto_expire(void);
void ttt_print_board(int user);
void ttt_end_game(int user, int winner);
void ttt_new_game(int user, char *str);
void ttt_make_move(int p, char *str);
void ttt_print(int p);
void ttt_abort(int p);
void ttt_cmd(int p, char *str);
void play_hangman(int user, char *inpstr);
void guess_hangman(int user, char *inpstr);
void frtell(int user, char *inpstr);
void femote(int user, char *inpstr);
void queue_write(int user, char *queue_str, int length);
void queue_write_www(int user, char *new_str, int length);
void external_menu(int user, char *wusername, char *wpassword, char *woption);
void unescape_url(char *url);
void plustospace(char *str);
void read_rebootdb(void);
void write_rebootdb(int user);
void user_hot_quit(int user);
void backup_stuff(int user, char *inpstr);
void trim_backups(int user, int days);
void do_tracking(int mode, char *downmess);
void do_timeset(char *zonetime);

/* Won't crash now I found some encouragement   */
int quit_multiples(int user);
int find_free_slot(char port);
int cat_to_sock(char *filename, int accept_sock);
int cat_to_www(char *filename, int user);
int cat(char *filename, int user, int line_num);
int nospeech(char *str);
int get_com_num(int user, char *inpstr);
int get_com_num_plain(char *inpstr);
int get_rank(char *inpstr);
int user_wants_message(int user, int type);
int check_gag(int user, int user2, int mode);
int check_gag2(int user, char *name);
int count_color(char *str, int mode);
int file_count_lines(char *file);
int file_count_bytes(char *file, int mode, int mode2);
int check_verify(int user, int mode);
int change_exem_data(char *user1, char *user2);
int write_exem_data(void);
int remove_exem_data(char *user1);
int write_nban_data(void);
int check_for_user(char *name);
int read_to_user(char *name, int user);
int get_user_num(char *i_name, int user);
int instr2(int pos, char *ss, char *sf, int mode);
int how_many_users(char *name);
int get_user_num_exact(char *i_name, int user);
int check_macro(int user, char *inpstr);
int log_misc_connect(int user, unsigned long addr, int type);
int find_num_in_area(int area);
int get_length(char *filen);
int read_user(char *name);
int check_restriction(int user, int type, int type2);
int check_nban(char *str, char *sitename);
int check_misc_restrict(int sock2, char *site, char *namesite);
int check_fname(char *inpstr, int user);
int write_pro(int user);
int write_room(int user);
int write_vote(int user);
int check_for_file(char *name);
int check_site(char *str1, char *str2, int mode);
int cache_lookup(int user, unsigned long addr2);
int convert_file(FILE *f, char *filename, int mode);
int strip_com(char *str);
int strip_level(char *str);
int is_revoke(char *str);
int is_grant(char *str);
int get_emote(int user);
int write_verifile(int user);
int mail_verify(int user, char *emailadd);
int check_rwho(int user, char *inpstr);
int get_odds_value(int user);
int determ_rand(int u1, int u2);
int get_flag_num(char *inpstr);
int ttt_is_end(int user);
int get_int(FILE *f);
int queue_flush(int user);
int queue_flush_www(int user);
int external_login(int user, char *wusername, char *wpassword, int wlogin);
int get_input(int user, char port, int mode);
int is_quit(int user, char *str);
int check_for_creation(char *name);
int backup_logs(int user);
long get_long(FILE *f);
long temp_mem_get(void);
long user_mem_get(int mode);
long area_mem_get(void);
long conv_mem_get(int mode);
long wiz_mem_get(void);
long shout_mem_get(void);

/* Can't break now I've been living for this    */
char *get_time(time_t ref, int mode);
char *get_help_file(char *i_name, int user);
char *get_temp_file(void);
char *get_mime_type(char *filen);
char *check_var(char *line, char *MACRO, char *Replacemnt);
char *itoa(int num);
char *strip_color(char *str);
char *convert_color(char *str);
char *converttime(long mins);
char *get_error(void);
char *generate_password(void);
char *start_hang_word(char *aword);
char *get_value(char *keypair, char *ukey);
char *log_error(int error);
char *get_iac_string(unsigned char code);
char x2c(char *what);
char get_onechar(FILE *f);

#if defined(FREEBSD_SYS) || defined(NETBSD_SYS)
/* From FreeBSD 4.3:
    char * ctime(const time_t *clock);
    struct tm *localtime(const time_t *clock);
*/
char *ctime(const time_t *);
struct tm *localtime(const time_t *);
#endif

#if defined(SOL_SYS)
void bcopy(const void *, void *, size_t);
# if defined(HAVE_GETHOSTNAME) && !defined(CYGWIN_SYS)
int gethostname(char *, int);
# endif
# if defined(HAVE_GETDOMAINNAME) && !defined(CYGWIN_SYS)
int getdomainname(char *, int);
# endif
# if defined(HAVE_TZSET)
void tzset(void);
# endif
/* char *strptime(const char *buf, const char *format, struct tm *tm); */
#endif

#if defined(__OSF__) || defined(__osf__) || defined(OSF_SYS) || defined(MACOSX_SYS)
# if defined(HAVE_GETDOMAINNAME)
int getdomainname(char *, int);
# endif
#endif

/* CYGRESOLVE */
int init_resolver_clipon(void);
void kill_resolver_clipon(void);
void send_resolver_request(int user, char *site, char *bad);
void read_resolver_reply(void);
void add_to_resolver_cache(int user);
void cleanup_resolver(void);
void del_from_resolver_cache(int user);

/* 1.2.3draft */
void addto_conv_buffer(ConvPtr buf, char *str);
void init_conv_buffer(ConvPtr buf);
void write_conv_buffer(int user, ConvPtr buf);
void init_macro_buffer(MacroPtr buf);
void check_sockets(void);
char *get_username_from_delims(char *str, int pos);
char *get_reaction(char *inpstr);
char *time_format_1(time_t dt);
char *time_format_2(time_t dt);
char *my_strptime(char *buf, char *fmt, struct tm *tmStruct);
int MY_FCNTL(int a, int b, unsigned long c);
int errno_ok(int myerr);

handler_t setsignal(int sig, handler_t handler);
/* RETSIGTYPE (*setsignal(int, RETSIGTYPE (*)(int)))(int); */

#if defined(OPENBSD_SYS)
/* our own ctime because the one in OpenBSD behaves badly */
char *ctime(const time_t *mytm);
#endif

int SHUTDOWN(int sock, int how);
void check_misc_connects(void);
void do_stafflist(void);

/* SMTP */
void check_smtp(void);
void requeue_smtp(int user);
int do_smtp_connect(void);
int queuetoactive_smtp(int user);   
int write_smtp_data(int user, int type);
int find_queue_slot(char *inpstr);
FILE *get_mailqueue_file(void);
/* SMTP */

/* 1.3.3 */
int send_ext_mail(int user, int target, int mode, char *subject, char *mailmess, int mailtype, char *recipient);
/* 1.3.3 */

/* 1.3.6 */
int find_free_connslot(void);
int in_connlist(int user);
int check_connlist(int user);
void check_connlist_entries(int mode);
void old_func(int user, char *inpstr, int mode);
/* 1.3.6 */

#endif /* _PROTOS_H */
