CC = gcc
EXTRA_CFLAGS =
EXTRA_LDFLAGS =
CFLAGS = $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)
INCS =
LIBS =
OBJS = comm.o

.PHONY : all clean

all: comm

clean:
	rm -f $(OBJS) comm

%.o: %.c
	$(CC) -o $@ $< -c $(CFLAGS) $(INCS)

comm: $(OBJS)
	$(CC) -o $@ $< $(LDFLAGS) $(LIBS)
