#!/usr/bin/env python
from time import sleep

def generate(N, rev=False):
    if rev:
        for i in range(N, 0, -1):
            print(i)
    else:
        for i in range(1, N+1, 1):
            print(i)

def wave():
    generate(250000);
    generate(250000, True);

    generate(90000);
    generate(90000, True);

    generate(200000);
    generate(200000, True);

    generate(90000);
    generate(90000, True);

    generate(250000);
    generate(250000, True);

def large_steps():
    N = 20000000
    a = 2000000
    for i in range(N):
        print(int(i/a))

    while N > 0:
        print(int(N/a))
        N -= 1

if __name__ == '__main__':
    sleep(1)
    wave()
