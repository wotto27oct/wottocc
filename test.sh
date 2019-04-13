#!/bin/bash
try() {
	expected="$1"
	input="$2"

	./wottocc "$input" > tmp.s
	gcc -o tmp tmp.s foo.o
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$expected expected, but got $actual"
		exit 1
	fi
}

tryfunc() {
	expected="$1"
	input="$2"

	./wottocc "$input" > tmp.s
	gcc -o tmp tmp.s foo.o
	actual=$(./tmp)

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$expected expected, but got $actual"
		exit 1
	fi
}

try 0 "main(){0;}"
try 42 "main(){42;}"

try 21 "main(){5+20-4;}"

try 41 "main(){ 12 + 34 - 5 ;}"

try 47 "main(){5+6*7;}"
try 15 "main(){5*(9-6);}"
try 4 "main(){(3+5)/2;}"

try 2 "main(){((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((1+1))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));}"

try 2 "main(){a=2;}"
try 5 "main(){a=5; b=a; b;}"
try 3 "main(){a=b=3; a;}"

try 4 "main(){abc = 4;}"
try 45 "main(){Abakan = 3; Minako = Abakan * 5; Million_t = Abakan * Minako;}"

try 1 "main(){return 1;}"

try 1 "main(){a = 4; a == 4;}"
try 0 "main(){a = 3; b = 4; a == b;}"

try 1 "main(){3+4 != 5+6;}"
try 0 "main(){a = 4; a + 2 != 6;}"

tryfunc "foo!" "main(){foo();}"
tryfunc "2" "main(){footwo(2);}"
tryfunc "11" "main(){a = 3; b = 2; c = 4; footwo(a+b*c);}"
tryfunc "9" "main(){foothree(1,4);}"
tryfunc "9" "main(){b=2;foothree(1,b+b);}"



echo OK
