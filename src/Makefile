# VAD code makefile

MAKEFILENAME=Makefile

CC = gcc
CFLAGS = -Wall -g -O4
AR = ar
RANLIB = arnlib
LIB = -lpthread -lm
ECHO_PREFIX=@\# 
CMD_PREFIX=

SRCS = $(wildcard *.c)

LIB_TARGET = libvad.a 
OBJECTS = vad.o



all : $(LIB_TARGET)
$(LIB_TARGET) : $(OBJECTS)
	$(ECHO_PREFIX) echo "   [AR] $^ -> $@"
	$(CMD_PREFIX) $(AR) cr $@ $^

#
# make / clean dependency list
#
depend:
	$(MAKE) -f $(MAKEFILENAME) $(MFLAGS) $(MAKEDEFS) clean_depend
	$(CC) $(CFLAGS) $(SRCS) >> $(MAKEFILENAME)

clean_depend:
	chmod u+w $(MAKEFILENAME)
	(awk 'BEGIN{f=1}{if (f) print $0}/^\# DO NOT DELETE THIS LINE -- make depend depends on it./{f=0}'\
	    < $(MAKEFILENAME) > .depend && \
	mv .depend $(MAKEFILENAME)) || exit 1;

	
clean : 
	rm -f $(LIB_TARGET) $(OBJECTS)
	
vad.o : vad.c
