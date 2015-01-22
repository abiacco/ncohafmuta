##########################################################################
##  Makefile for Ncohafmuta 1.4.x utilities  by: Cygnus                 ## 
##                                         Last modified: Apr 12,1997   ##
##########################################################################

# Name of the executable to be made
ADDBIN           = addstruct
TRIMBIN          = trimbackups

##########################################################################
# Your compiler program. Must be 
# able to grok ANSI C
##########################################################################
#
CC               = gcc
#CC               = cc

# Files used by the program
ADDCFILES         = addstruct.c
TRIMCFILES        = trimbackups.c

# Makefile arguments
#
all:		clean \
		$(ADDBIN) \
		$(TRIMBIN)
				@echo 'Made all'

$(ADDBIN): $P $(OFILES) Makefile
	  $(CC) -o $(ADDBIN) $(ADDCFILES)

$(TRIMBIN): $P $(OFILES) Makefile
	  $(CC) -o $(TRIMBIN) $(TRIMCFILES)

clean:
	rm -f $(ADDBIN) $(TRIMBIN)

# DO NOT REMOVE THIS LINE OR CHANGE ANYTHING AFTER IT #
addstruct.o: addstruct.c 
trimbackups.o: trimbackups.c 


