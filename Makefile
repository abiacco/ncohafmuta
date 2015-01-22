# Makefile generated on Thu Apr  8 14:40:45 PDT 2004 for Ncohafmuta 1.4.2
#
CC = gcc
CFLAGS = -DHAVE_CONFIG_H -O3 
LIBS =  -lresolv
EXE_EXT = 
VERSION = Ncohafmuta 1.4.2
#####          YOU SHOULD NOT NEED TO EDIT ANYTHING AFTER THIS LINE  #####
##########################################################################
# Temp sub-directory that will be made for "make dist" processing
DIST-DIR	= ncohafmuta-1.4.2

# The directory where the exectuable,
# and all other main files/dirs lie
MAIN-DIR	= .

# Directory where temp compiled files will be put
OBJDIR		= objfiles
# Directory where the source files are located
SRCDIR		= srcfiles
# Directory where the header or library files are located
HDRDIR		= hdrfiles

# Name of the executable to be made
SERVBIN		= server$(EXE_EXT)

# Name of executable to be made for testing..in a 'make test'
TESTBIN		= a.out$(EXE_EXT)

# Name of the resolver executable to be made
RESBIN		= resolver_clipon$(EXE_EXT)

# Header files
HDRS         =

# Files used by the program
# .o files..specify one for every .c files we have
OFILES   = $(OBJDIR)/main.o $(OBJDIR)/socket_funcs.o \
		$(OBJDIR)/comm_cmds.o $(OBJDIR)/room_cmds.o \
		$(OBJDIR)/server.o $(OBJDIR)/smtp_funcs.o \
		$(OBJDIR)/write_funcs.o $(OBJDIR)/check_funcs.o \
		$(OBJDIR)/get_funcs.o $(OBJDIR)/rwho_funcs.o \
		$(OBJDIR)/authuser_funcs.o $(OBJDIR)/strutil_funcs.o \
		$(OBJDIR)/everify_funcs.o $(OBJDIR)/fileutil_funcs.o \
		$(OBJDIR)/game_funcs.o $(OBJDIR)/misc_funcs.o \
		$(OBJDIR)/restrict_funcs.o $(OBJDIR)/userutil_funcs.o \
		$(OBJDIR)/login_funcs.o $(OBJDIR)/command_funcs.o \
		$(OBJDIR)/social_funcs.o $(OBJDIR)/snprintf_funcs.o\
		$(OBJDIR)/signal_funcs.o $(OBJDIR)/datautil_funcs.o \
		$(OBJDIR)/telopt_funcs.o $(OBJDIR)/resolve_funcs.o \
		$(OBJDIR)/whowww_funcs.o $(OBJDIR)/set_funcs.o \
		$(OBJDIR)/resolver_client.o
RES_OFILES    = $(OBJDIR)/resolver_clipon.o

# Makefile arguments
#
#.SUFFIXES: .c .o

all:   		make_line make_server make_res
		@echo '-----------------------------------------------------------------------------'
		@echo ''
		@echo 'Made all'

test:		make_line make_testserver make_res
		@echo '-----------------------------------------------------------------------------'
		@echo ''
		@echo 'Made test binary'

$(SERVBIN):	make_line make_server make_res
		@echo '-----------------------------------------------------------------------------'
		@echo ''
		@echo 'Made $(SERVBIN)'

$(TESTBIN):	make_line make_testserver make_res
		@echo '-----------------------------------------------------------------------------'
		@echo ''
		@echo 'Made $(TESTBIN)'

$(RESBIN):	make_line make_res
		@echo '-----------------------------------------------------------------------------'
		@echo ''
		@echo 'Made $(RESBIN)'

make_server: $(OFILES) Makefile
	  $(CC) $(CFLAGS) $(HDRS) -o $(SERVBIN) $(OFILES) $(LIBS)
	  chmod 700 $(SERVBIN) restart shutdown

make_testserver: $(OFILES) Makefile
	  $(CC) $(CFLAGS) $(HDRS) -o $(TESTBIN) $(OFILES) $(LIBS)
	  chmod 700 $(TESTBIN) restart shutdown

make_res: $(RES_OFILES) Makefile
	  $(CC) $(CFLAGS) $(HDRS) -o $(RESBIN) $(RES_OFILES) $(LIBS)
	  chmod 700 $(RESBIN)

make_line:
	@echo '-----------------------------------------------------------------------------'

clean:	objclean
	rm -f $(SERVBIN) $(TESTBIN) $(RESBIN) core config.log

distclean:	clean smtpclean logclean
	rm -f $(HDRDIR)/config.h	
	rm -f junk/*
	rm -f Makefile

objclean:
	rm -f $(OBJDIR)/*.o

logclean:
	rm -f logfiles/*.log logfiles/lastcommand logfiles/lastcommand.*

smtpclean:
	rm -f maildir/smtp_queue/* maildir/smtp_active/*

mkdist:
	mkdir -m 700 $(DIST-DIR)
	mkdir -m 700 $(DIST-DIR)/users
	mkdir -m 700 $(DIST-DIR)/maildir
	mkdir -m 700 $(DIST-DIR)/maildir/smtp_queue
	mkdir -m 700 $(DIST-DIR)/maildir/smtp_active
	mkdir -m 700 $(DIST-DIR)/prodir
	mkdir -m 700 $(DIST-DIR)/wizinfo
	mkdir -m 700 $(DIST-DIR)/messboards
	mkdir -m 700 $(DIST-DIR)/picture
	mkdir -m 700 $(DIST-DIR)/restrict
	mkdir -m 700 $(DIST-DIR)/newrestrict
	mkdir -m 700 $(DIST-DIR)/lib
	mkdir -m 700 $(DIST-DIR)/config
	mkdir -m 700 $(DIST-DIR)/helpfiles
	mkdir -m 700 $(DIST-DIR)/utils
	mkdir -m 700 $(DIST-DIR)/warnings
	mkdir -m 700 $(DIST-DIR)/bot
	mkdir -m 700 $(DIST-DIR)/webfiles
	mkdir -m 700 $(DIST-DIR)/webfiles/userpics
	mkdir -m 700 $(DIST-DIR)/tzinfo
	mkdir -m 700 $(DIST-DIR)/docs
	mkdir -m 700 $(DIST-DIR)/logfiles
	mkdir -m 700 $(DIST-DIR)/$(SRCDIR)
	mkdir -m 700 $(DIST-DIR)/$(HDRDIR)
	mkdir -m 700 $(DIST-DIR)/$(OBJDIR)
	mkdir -m 700 $(DIST-DIR)/junk ;\
	cp -r $(MAIN-DIR)/$(SRCDIR)/* $(DIST-DIR)/$(SRCDIR)
	cp -r $(MAIN-DIR)/$(HDRDIR)/* $(DIST-DIR)/$(HDRDIR)
	rm -fr $(DIST-DIR)/$(SRCDIR)/old
	rm -fr $(DIST-DIR)/$(SRCDIR)/newold
	rm -f $(DIST-DIR)/$(SRCDIR)/*.old
	rm -f $(DIST-DIR)/$(HDRDIR)/config.h
	cp $(MAIN-DIR)/restart $(DIST-DIR)/
	cp $(MAIN-DIR)/shutdown $(DIST-DIR)/
	cp $(MAIN-DIR)/Makefile.in $(DIST-DIR)/
	cp $(MAIN-DIR)/configure $(DIST-DIR)/
	cp $(MAIN-DIR)/config.sub $(DIST-DIR)/
	cp $(MAIN-DIR)/config.guess $(DIST-DIR)/
	cp $(MAIN-DIR)/READ_docs_DIR $(DIST-DIR)/
	cp -r $(MAIN-DIR)/docs/* $(DIST-DIR)/docs
	cp $(MAIN-DIR)/picture/* $(DIST-DIR)/picture
	cp -r $(MAIN-DIR)/lib/* $(DIST-DIR)/lib
	cp -r $(MAIN-DIR)/bot/* $(DIST-DIR)/bot
	rm -fr $(DIST-DIR)/bot/Stories/*
	rm -f $(DIST-DIR)/bot/storybot
	rm -f $(DIST-DIR)/bot/botlog*
	rm -fr $(DIST-DIR)/bot/old
	cp -r $(MAIN-DIR)/bot/Stories/'Using spokes' $(DIST-DIR)/bot/Stories/
	cp -r $(MAIN-DIR)/tzinfo/* $(DIST-DIR)/tzinfo
	cp -r $(MAIN-DIR)/config/* $(DIST-DIR)/config
	cp -r $(MAIN-DIR)/utils/* $(DIST-DIR)/utils
	rm -fr $(DIST-DIR)/utils/backupd/*.tar.gz
	rm -fr $(DIST-DIR)/utils/backupd/*.tar
	rm -fr $(DIST-DIR)/utils/backupd/restored/*
	cp -r $(MAIN-DIR)/webfiles/* $(DIST-DIR)/webfiles
	rm -f $(DIST-DIR)/webfiles/userpics/*
	cp $(MAIN-DIR)/helpfiles/* $(DIST-DIR)/helpfiles
	rm -f $(DIST-DIR)/lib/activity

mkdistsmallrm:
	rm -fr $(DIST-DIR)/picture/*
	rm -fr $(DIST-DIR)/bot/*
	rm -fr $(DIST-DIR)/utils/*
	rm -fr $(DIST-DIR)/tzinfo/America $(DIST-DIR)/tzinfo/Asia
	rm -fr $(DIST-DIR)/tzinfo/Africa $(DIST-DIR)/tzinfo/Atlantic
	rm -fr $(DIST-DIR)/tzinfo/Australia $(DIST-DIR)/tzinfo/Brazil
	rm -fr $(DIST-DIR)/tzinfo/Canada $(DIST-DIR)/tzinfo/Chile
	rm -fr $(DIST-DIR)/tzinfo/Europe $(DIST-DIR)/tzinfo/Indian
	rm -fr $(DIST-DIR)/tzinfo/Mexico $(DIST-DIR)/tzinfo/Mideast
	rm -fr $(DIST-DIR)/tzinfo/SystemV $(DIST-DIR)/tzinfo/US
	rm -fr $(DIST-DIR)/tzinfo/Pacific

dist:	mkdist
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-full.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-dist-full.tar $(DIST-DIR) ;\
		echo Compressing.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-full.tar.gz ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-full.tar.gz ;\
	)

distZ:	mkdist
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-full.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-dist-full.tar $(DIST-DIR) ;\
		echo Compressing.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-full.tar.Z ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-full.tar.Z ;\
	)

distzip:	mkdist
		mkdir -p /tmp/ncohafmuta
	(	echo Zipping.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-full.zip ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-dist-full.zip $(DIST-DIR) ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-full.zip ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-full.zip ;\
	)

distuu:	mkdist
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-full.uu ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-full.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-dist-full.tar $(DIST-DIR) ;\
		echo Compressing.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-dist-full.tar ;\
		uuencode /tmp/ncohafmuta/ncohafmuta-dist-full.tar.gz ncohafmuta-dist-full.tar.gz > /tmp/ncohafmuta/ncohafmuta-dist-full.uu ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-full.uu ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-full.uu ;\
	)

distpic:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring pictures.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-pictures.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-pictures.tar picture ;\
		echo Compressing pictures.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-pictures.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-pictures.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-pictures.tar.gz ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-pictures.tar.gz ;\
	)

distpiczip:
	mkdir -p /tmp/ncohafmuta
	(	rm -fr /tmp/ncohafmuta/ncohafmuta-pictures.zip ;\
		echo Compressing pictures.. ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-pictures.zip picture ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-pictures.zip ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-pictures.zip ;\
	)

distpicZ:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring pictures.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-pictures.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-pictures.tar picture ;\
		echo Compressing pictures.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-pictures.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-pictures.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-pictures.tar.Z ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-pictures.tar.Z ;\
	)

distbot:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring bot.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-bot.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-bot.tar bot ;\
		echo Compressing bot.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-bot.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-bot.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-bot.tar.gz ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-bot.tar.gz ;\
	)

distbotzip:
	mkdir -p /tmp/ncohafmuta
	(	rm -fr /tmp/ncohafmuta/ncohafmuta-bot.zip ;\
		echo Compressing bot.. ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-bot.zip bot ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-bot.zip ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-bot.zip ;\
	)

distbotZ:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring bot.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-bot.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-bot.tar bot ;\
		echo Compressing bot.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-bot.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-bot.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-bot.tar.Z ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-bot.tar.Z ;\
	)

disttzinfo:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring tzinfo.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-tzinfo.tar tzinfo ;\
		echo Compressing tzinfo.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-tzinfo.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-tzinfo.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.gz ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.gz ;\
	)

disttzinfozip:
	mkdir -p /tmp/ncohafmuta
	(	rm -fr /tmp/ncohafmuta/ncohafmuta-tzinfo.zip ;\
		echo Compressing tzinfo.. ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-tzinfo.zip tzinfo ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-tzinfo.zip ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-tzinfo.zip ;\
	)

disttzinfoZ:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring tzinfo.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-tzinfo.tar tzinfo ;\
		echo Compressing tzinfo.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-tzinfo.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-tzinfo.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.Z ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-tzinfo.tar.Z ;\
	)

distutils:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring utils.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-utils.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-utils.tar utils ;\
		echo Compressing utils.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-utils.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-utils.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-utils.tar.gz ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-utils.tar.gz ;\
	)

distutilszip:
	mkdir -p /tmp/ncohafmuta
	(	rm -fr /tmp/ncohafmuta/ncohafmuta-utils.zip ;\
		echo Compressing utils.. ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-utils.zip utils ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-utils.zip ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-utils.zip ;\
	)

distutilsZ:
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring utils.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-utils.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-utils.tar utils ;\
		echo Compressing utils.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-utils.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-utils.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-utils.tar.Z ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-utils.tar.Z ;\
	)

distsmall:	mkdist mkdistsmallrm
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar.gz ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar $(DIST-DIR) ;\
		echo Compressing.. ;\
		gzip -9 /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar.gz ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar.gz ;\
	)

distsmallzip:	mkdist mkdistsmallrm
	mkdir -p /tmp/ncohafmuta
	(	echo Zipping.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-minimal.zip ;\
		zip -9 -v -r /tmp/ncohafmuta/ncohafmuta-dist-minimal.zip $(DIST-DIR) ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-minimal.zip ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-minimal.zip ;\
	)

distsmallZ:	mkdist mkdistsmallrm
	mkdir -p /tmp/ncohafmuta
	(	echo Tarring.. ;\
		rm -fr /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar.Z ;\
		tar cpf /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar $(DIST-DIR) ;\
		echo Compressing.. ;\
		compress -v /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar ;\
		rm -f /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar ;\
		chmod 600 /tmp/ncohafmuta/ncohafmuta-dist-minimal.tar.Z ;\
		echo Removing buffer directory.. ;\
		rm -fr $(DIST-DIR) ;\
		echo Archive is in /tmp/ncohafmuta/ncohafmuta-dist-minimal.Z ;\
	)

love:
	@echo "Not war?" ; sleep 2
	@echo "Look, I'm not equipped for that, okay?" ; sleep 2
	@echo "Contact your hardware vendor for appropriate mods."

# DO NOT REMOVE THIS LINE OR CHANGE ANYTHING AFTER IT	#
# UNLESS YOU ADD MORE C FILES TO COMPILE IN		#
$(OBJDIR)/authuser_funcs.o: $(SRCDIR)/authuser_funcs.c \
 $(HDRDIR)/authuser.h $(HDRDIR)/constants.h $(HDRDIR)/includes.h \
 $(HDRDIR)/netdb.h $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h \
 $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/authuser_funcs.o -c $(SRCDIR)/authuser_funcs.c
$(OBJDIR)/check_funcs.o: $(SRCDIR)/check_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/check_funcs.o -c $(SRCDIR)/check_funcs.c
$(OBJDIR)/comm_cmds.o: $(SRCDIR)/comm_cmds.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/comm_cmds.o -c $(SRCDIR)/comm_cmds.c
$(OBJDIR)/command_funcs.o: $(SRCDIR)/command_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/command_funcs.o -c $(SRCDIR)/command_funcs.c
$(OBJDIR)/datautil_funcs.o: $(SRCDIR)/datautil_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/datautil_funcs.o -c $(SRCDIR)/datautil_funcs.c
$(OBJDIR)/everify_funcs.o: $(SRCDIR)/everify_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/everify_funcs.o -c $(SRCDIR)/everify_funcs.c
$(OBJDIR)/fileutil_funcs.o: $(SRCDIR)/fileutil_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/fileutil_funcs.o -c $(SRCDIR)/fileutil_funcs.c
$(OBJDIR)/game_funcs.o: $(SRCDIR)/game_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/text.h \
 $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/game_funcs.o -c $(SRCDIR)/game_funcs.c
$(OBJDIR)/get_funcs.o: $(SRCDIR)/get_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/get_funcs.o -c $(SRCDIR)/get_funcs.c
$(OBJDIR)/login_funcs.o: $(SRCDIR)/login_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/login_funcs.o -c $(SRCDIR)/login_funcs.c
$(OBJDIR)/main.o: $(SRCDIR)/main.c $(HDRDIR)/constants.h \
 $(HDRDIR)/includes.h $(HDRDIR)/netdb.h $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h \
 $(HDRDIR)/resolver_clipon.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/main.o -c $(SRCDIR)/main.c
$(OBJDIR)/misc_funcs.o: $(SRCDIR)/misc_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/misc_funcs.o -c $(SRCDIR)/misc_funcs.c
$(OBJDIR)/resolve_funcs.o: $(SRCDIR)/resolve_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/resolve_funcs.o -c $(SRCDIR)/resolve_funcs.c
$(OBJDIR)/resolver_client.o: $(SRCDIR)/resolver_client.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/resolver_clipon.h \
 $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/resolver_client.o -c $(SRCDIR)/resolver_client.c
$(OBJDIR)/resolver_clipon.o: $(SRCDIR)/resolver_clipon.c \
 $(HDRDIR)/includes.h $(HDRDIR)/nameser.h $(HDRDIR)/netdb.h $(HDRDIR)/osdefs.h \
 $(HDRDIR)/resolv.h $(HDRDIR)/resolver_clipon.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/resolver_clipon.o -c $(SRCDIR)/resolver_clipon.c
$(OBJDIR)/restrict_funcs.o: $(SRCDIR)/restrict_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/restrict_funcs.o -c $(SRCDIR)/restrict_funcs.c
$(OBJDIR)/room_cmds.o: $(SRCDIR)/room_cmds.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/room_cmds.o -c $(SRCDIR)/room_cmds.c
$(OBJDIR)/rwho_funcs.o: $(SRCDIR)/rwho_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/rwho_funcs.o -c $(SRCDIR)/rwho_funcs.c
$(OBJDIR)/server.o: $(SRCDIR)/server.c \
 $(HDRDIR)/authuser.h \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/resolver_clipon.h \
 $(HDRDIR)/text.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/server.o -c $(SRCDIR)/server.c
$(OBJDIR)/set_funcs.o: $(SRCDIR)/set_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/set_funcs.o -c $(SRCDIR)/set_funcs.c
$(OBJDIR)/signal_funcs.o: $(SRCDIR)/signal_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/signal_funcs.o -c $(SRCDIR)/signal_funcs.c
$(OBJDIR)/smtp_funcs.o: $(SRCDIR)/smtp_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/smtp_funcs.o -c $(SRCDIR)/smtp_funcs.c
$(OBJDIR)/snprintf_funcs.o: $(SRCDIR)/snprintf_funcs.c \
 $(HDRDIR)/includes.h $(HDRDIR)/netdb.h $(HDRDIR)/snprintf.h \
 $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/snprintf_funcs.o -c $(SRCDIR)/snprintf_funcs.c
$(OBJDIR)/social_funcs.o: $(SRCDIR)/social_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/social_funcs.o -c $(SRCDIR)/social_funcs.c
$(OBJDIR)/socket_funcs.o: $(SRCDIR)/socket_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/resolver_clipon.h \
 $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/socket_funcs.o -c $(SRCDIR)/socket_funcs.c
$(OBJDIR)/strutil_funcs.o: $(SRCDIR)/strutil_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/strutil_funcs.o -c $(SRCDIR)/strutil_funcs.c
$(OBJDIR)/telopt_funcs.o: $(SRCDIR)/telopt_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/telopt_funcs.o -c $(SRCDIR)/telopt_funcs.c
$(OBJDIR)/userutil_funcs.o: $(SRCDIR)/userutil_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/userutil_funcs.o -c $(SRCDIR)/userutil_funcs.c
$(OBJDIR)/whowww_funcs.o: $(SRCDIR)/whowww_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/whowww_funcs.o -c $(SRCDIR)/whowww_funcs.c
$(OBJDIR)/write_funcs.o: $(SRCDIR)/write_funcs.c \
 $(HDRDIR)/constants.h $(HDRDIR)/includes.h $(HDRDIR)/netdb.h \
 $(HDRDIR)/osdefs.h $(HDRDIR)/protos.h $(HDRDIR)/config.h
	$(CC) $(CFLAGS) $(HDRS) -o $(OBJDIR)/write_funcs.o -c $(SRCDIR)/write_funcs.c
