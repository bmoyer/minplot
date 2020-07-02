#!/usr/bin/env python3

def bars():
    width = 100000
    skip = 1000

    i = 0
    while i < width:
        for j in range(skip):
            print(0)
            i += 1

        for j in range(skip):
            print(100)
            i += 1

if __name__ == '__main__':
    bars()
