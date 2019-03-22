wottocc: wottocc.c vector.c
	gcc wottocc.c vector.c -o wottocc

test: wottocc
	./wottocc -test
	./test.sh

clean:
	rm -f wottocc *.o *~ tmp*
