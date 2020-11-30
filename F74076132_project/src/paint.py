#!/usr/bin/env python3 
import matplotlib.pyplot as plt
import csv
import sys

x = []
y = []
cnt = 0
file_name = sys.argv[1]
with open('./drive/MyDrive/ColabData/handoff/' + file_name, 'r') as f: 
  y = f.read().split(',')
x = list(range(len(y)))
# print(x)
# print(y)

plt.bar(x, y, .5)
plt.savefig('./img' + file_name + '.png')