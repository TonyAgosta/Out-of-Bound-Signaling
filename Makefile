CC = gcc
CCFLAGS += -Wall -Werror -pedantic -std=gnu99

INCLUDES= -I.
LIBS= -lpthread
OPTFLAGS= -g

TARGETS= supervisor		\
		 server			\
		 client

.PHONY: all clean cleanall test
.SUFFIXES: .c .h .sh

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
	\rm -f  OOB* *.o *.log

test:
	bash ./test.sh
