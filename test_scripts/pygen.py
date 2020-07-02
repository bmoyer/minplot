#!/usr/bin/env python3

def large_steps():
    N = 20000000
    a = 200000
    for i in range(N):
        print(int(i/a))

    while N > 0:
        print(int(N/a))
        N -= 1
    
if __name__ == '__main__':
    large_steps()
    large_steps()
    large_steps()
    large_steps()
