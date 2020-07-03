#!/usr/bin/env python
from time import sleep

def generate(N, rev=False):
    if rev:
        for i in range(N, 0, -1):
            print(i)
    else:
        for i in range(1, N+1, 1):
            print(i)

def wave(scale=1.0):
    generate(int(250000*scale));
    generate(int(250000*scale), True);

    generate(int(90000*scale));
    generate(int(90000*scale), True);

    generate(int(200000*scale));
    generate(int(200000*scale), True);

    generate(int(90000*scale));
    generate(int(90000*scale), True);

    generate(int(250000*scale));
    generate(int(250000*scale), True);

def large_steps():
    N = 20000000
    a = 2000000
    for i in range(N):
        print(int(i/a))

    while N > 0:
        print(int(N/a))
        N -= 1

def bars():
    width = 100000
    skip = 1000

    i = 0
    while i < width:
        for j in range(skip):
            print(50)
            i += 1

        for j in range(skip):
            print(100)
            i += 1

if __name__ == '__main__':
    wave(10)
