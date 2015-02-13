CC = gcc
CFLAGS = -g -Wall
LDFLAGS = 

OBJS = main.o util.o state.o cli.o instructions.o debug-state.o interrupts.o define.o inst2text.o
SRCS = main.c util.c state.c cli.c instructions.c debug-state.c interrupts.c define.c inst2text.c

PROGRAM = jonsp430
	
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o jonsp430
