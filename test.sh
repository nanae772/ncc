#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./ncc "$input" > tmp.s
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
try 123 123
try 21 '5+20-4'
try 5 '1+2+3+4+5-10'
try 1 '0-1+2'
try 6 '1 + 2 + 3'
try 41 ' 12 + 34  - 5 '
try 123 '   123   '

echo OK