CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

wottocc: $(OBJS)
	$(CC) -o wottocc $(OBJS) $(LDFLAGS)

$(OBJS): wottocc.h

test: wottocc
	./wottocc -test
	./test.sh

clean:
	rm -f wottocc *.o *~ tmp*
