wottocc: wottocc.c

test: wottocc
	./wottocc -test
	./test.sh

clean:
	rm -f wottocc *.o *~ tmp*
