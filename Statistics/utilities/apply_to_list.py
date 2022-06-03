#! /usr/bin/python

from sys import argv,exit
from os import system
import glob

if len(argv) < 4:
    print "Usage: %s <file-list.txt> command --input <par0> .. <parN>" % argv[0]
    exit(0)

cmd = ""

files = open(argv[1])
count = int(files.readline())

for arg in argv[2:]:

    if arg == "--input":
        cmd += " %s"
    else:
        cmd += " " + arg

for i in xrange(0,count):

    print cmd % files.readline()
    #system(cmd % f)


    
