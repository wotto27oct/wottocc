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

try 0 "0;"
try 42 "42;"

try 21 "5+20-4;"

try 41 " 12 + 34 - 5 ;"

try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"

try 2 "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((1+1))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));"

try 2 "a=2;"
try 5 "a=5; b=a; b;"
try 3 "a=b=3; a;"

try 4 "abc = 4;"
try 45 "Abakan = 3; Minako = Abakan * 5; Million_t = Abakan * Minako;"

try 1 "return 1;"

try 1 "a = 4; a == 4;"
try 0 "a = 3; b = 4; a == b;"
echo OK
