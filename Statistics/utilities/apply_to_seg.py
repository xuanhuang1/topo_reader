#! /usr/bin/python

from sys import argv,exit
from os import system
from os.path import splitext,join
import glob

if len(argv) < 4:
    print "Usage: %s \"<file-template.family>\" command --input <par0> .. <parN>" % argv[0]
    print "Will call command file.family file.seg file.map <par0> ... <parN>"
    exit(0)

cmd = ""

files = glob.glob(argv[1])

for arg in argv[2:]:

    if arg == "--input":
        cmd += " %s"
    else:
        cmd += " " + arg

for f in files:

    triple = f + " " + splitext(f)[0] + ".seg " + splitext(f)[0] + ".map"  

    #print cmd % triple
    system(cmd % triple)


    
