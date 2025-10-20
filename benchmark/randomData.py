#!/bin/python3

from random import randint
import sys

file: str = "data.md"
if len(sys.argv) > 1:
    file = sys.argv[1]

with open("gendata.md") as f:
    s = f.read()

data: list[str] = s.splitlines()
n: int = len(data)

with open(file, "w") as f:
    maxN = 20
    for _ in range(32000):
        idx = randint(0, n)
        if idx == n or maxN < 0 or data[idx] == '':
            _ = f.write("\n")
            maxN = 20
        else:
            _ = f.write(data[idx])
            maxN -= 1;
