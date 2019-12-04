CC = gcc
CCFLAGS += -Wall -Werror -pedantic -fsanitize=address -o2 -fno-omit-frame-pointer -std=gnu99

INCLUDES= -I.
LIBS= -lpthread
OPTFLAGS= -g

TARGETS= supervisor		\
		 server			\
		 client

.PHONY: all clean cleanall 
.SUFFIXES: .c .h

all : $(TARGETS)

supervisor: Supervisor.c mylib.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $<

server: Server.c mylib.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

client: Client.c mylib.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $<

clean	:
	rm -f $(TARGETS)
cleanall : clean
	\rm -f  OOB* *.o 
