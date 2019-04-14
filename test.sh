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

try 10 "int main(){int x; x=10; return x;}"

try 4 "int main(){int i; int a; a = 10; i = 4;return i;}"

try 20 "int main(){int i; int a; a = 10; for(i = 0; i < 10; ++i) ++a; return a;}"

try 5 "int foo(int x, int y){return x + y * 2;} int main(){foo(1,2);}"
try 5 "int foo(int x, int y){return x + y * 2;} int main(){int x;int y;x=2;y=1;foo(y,x);}"

try 1 "int main(){int **a; return 1;}"
try 3 "int main(){int x; x=3; int *y; y=&x; return *y;}"
try 2 "int main(){int x; x=3; int *y; y=&x; *y=2; return x;}"

try 0 "int main(){int *p; int *q; q = p + 2; return 0;}"
try 0 "int main(){int **p; int **q; q = p - 4; return 0;}"

try 0 "int main(){int a[10]; return 0;}"

echo OK
