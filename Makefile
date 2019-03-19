wottocc: wottocc.c

test: wottocc
	./test.sh

clean:
	rm -f wottocc *.o *~ tmp*
