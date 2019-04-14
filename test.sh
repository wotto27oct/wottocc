#!/bin/bash
try() {
	expected="$1"
	input="$2"

	./wottocc "$input" > tmp.s
	gcc -o tmp tmp.s
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

try 1 "main(){1;} foo(){2;}"
try 2 "main(){foo();} foo(){return 2;}"
try 3 "foo(){return 3;} footwo(){return 4;} main(){foo();}"

try 0 "foo(){x=3;} main(){x=0;foo();x;}"

try 1 "foo(x){0;} main(){foo(3);1;}"
try 3 "foo(x){x=x+1;return x;} main(){foo(2);}"
try 5 "foo(x,y){return x + 2 * y;} main(){x=2; foo(1,2);}"

try 2 "main(){a=3;if(1)a=2;a;}"
try 3 "main(){a=2;if(3)a=3;a;}"

try 1 "main(){a=0;if(a==1)a=2;else a=1; return a;}"
try 5 "main(){a=0;if(a==1) if(a==2) a=3; else a=4; else a=5; return a;}"
try 11 "main(){a=3; b=2; if(a==3) if(b==1) a=10; else a=11; else a=12; return a;}"

try 1 "main(){a=0;while(a==0)a=1;return a;}"

try 1 "main(){1<2;}"
try 11 "main(){a=0;while(a<11)a=a+1;return a;}"
try 1 "main(){a=10;while(a>1)a=a-1;return a;}"
try 12 "main(){a=0;while(a<=11)a=a+1;return a;}"
try 0 "main(){a=10;while(a>=1)a=a-1;return a;}"
try 1 "main(){1<2<=1;}"

try 10 "main(){b=0;for(i=0;i<10;i=i+1) b=b+1; return b;}"
echo OK
