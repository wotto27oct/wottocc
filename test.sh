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

try 3 "int main(){int x; x=3; int *y; y = &x; return *y;}"

try 20 "int main(){int i; int a; a = 10; for(i = 0; i < 10; ++i) ++a; return a;}"

try 2 "int main(int x){x = 2; return x;}"
try 4 "int foo(int x){return x+1;} int main(){return foo(3);}"

try 5 "int foo(int x, int y){return x + y * 2;} int main(){foo(1,2);}"
try 5 "int foo(int x, int y){return x + y * 2;} int main(){int x;int y;x=2;y=1;foo(y,x);}"

try 1 "int main(){int **a; return 1;}"
try 3 "int main(){int x; x=3; int *y; y=&x; return *y;}"
try 2 "int main(){int x; x=3; int *y; y=&x; *y=2; return x;}"

try 0 "int main(){int *p; int *q; q = p + 2; return 0;}"
try 0 "int main(){int **p; int **q; q = p - 4; return 0;}"

try 0 "int main(){int a[10]; return 0;}"

try 1 "int main(){int a[2]; *a=1; return *a;}"

try 3 "int main(){int a[2];*a=1;*(a+1)=2; int *p; p = a;return *p + *(p+1);}"

try 3 "int main(){int a[3]; a[2] = 3; return a[2];}"
try 5 "int main(){int a[3]; *(2+a)=5;return a[2];}"

try 2 "int main(){int a;int b;int c;a=1;b=2;c=0;if(a==1)if(b==1)c=1;else c=2; else c=3; return c;}"
try 10 "int main(){int i; int b; b=0; for(i=0;i<10;++i) ++b; return b;}"
try 0 "int main(){int i; i=10; while(i>0) --i; return i;}"
try 1 "int main(){int i; int a[5]; a[3] = 1; i = a[3]; return i;}"
try 4 "int main(){int i;i=2;if(1){i=3;i=4;}return i;}"

try 3 "int main(){int i;i=3;{int i;i=4;}return i;}"

try 10 "int main(){int x,y;x=5;y=10;return y;}"
try 3 "int main(){int *x,y;y=3;return y;}"

try 0 "int main(){while(1){break;}return 0;}"
try 1 "int main(){int i;i=1;while(1){if(i==1)break;}return i;}"
try 11 "int main(){int i;i=0;while(1){if(i>10) break; i=i+1;}return i;}"

try 1 "int main(){int i;i=0;switch(i){}return 1;}"
try 1 "int main(){int i;i=0;switch(i){case 0: break; case 1: break;} return 1;}"
try 2 "int main(){int i;i=0;switch(i){case 0: i=2;break; case 1: break;} return i;}"
try 3 "int main(){int i;i=2;switch(i){case 0: i=0; break; case 2: i=3; break;} return i;}"

try 3 "int main(){int i;i=0;while(i<3){while(i<2){++i;} ++i;}return i;}"
try 5 "int main(){int i;i=0;while(1){while(1){++i; if(i==5){break;}} break;} return i;}"

try 5 "int main(){int i;int a;i=0;a=0;for(i=0;i<10;++i){if(i<5) continue; ++a;} return a;}"

try 10 "int main(){int i=10;return i;}"
try 10 "int main(){for(int i=0;i<10;++i){1;}return i;}"

try 10 "int main(){int i=0;for(;i<10;++i){} return i;}"
try 10 "int main(){int i=1;for(;;) { ++i; if(i == 10) break;} return i;}"

try 1 "int main(){int i=0;if(i==0)if(i==1)i=2;else i=1;return i;}"

echo OK
