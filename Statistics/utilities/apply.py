#! /usr/bin/python

from sys import argv,exit
from os import system
import glob

if len(argv) < 4:
    print "Usage: %s \"<file-template>\" command --input <par0> .. <parN>" % argv[0]
    exit(0)

cmd = ""

files = glob.glob(argv[1])

for arg in argv[2:]:

    if arg == "--input":
        cmd += " %s"
    else:
        cmd += " " + arg

for f in files:
    print cmd % f
    system(cmd % f)


    
