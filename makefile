CC=gcc

CFLAGS=-W -Wall

all: decod

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) decod
