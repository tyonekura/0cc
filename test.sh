#!/bin/bash
try(){
        expected="$1"
        input="$2"

        ./0cc "$input" > tmp.s
        more tmp.s
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

try 0 0
try 42 42
try 8 '5+10-4+3'
try 41 ' 12 + 34 - 5 '
try 47 "5+6*7"
try 77 "(5+6)*7"
try 4 "2*7/3"
try 4 "(3+5)/  2"

echo OK
