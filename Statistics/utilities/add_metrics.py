#!/usr/bin/python

from sys import argv,exit
from os import system
from os.path import abspath

if len(argv) < 3:
    print "Usage: %s <add_derived_metrc> <input.family> <input.seg> <field.raw> <field_name>" % argv[0]
    exit(0)


exe = argv[1]

cmd = "%s %s %s %s " % (abspath(argv[1]),argv[2],argv[3],argv[4]) + " %s %s_" + argv[5]


for method in ["volume","mean","variance"]:  
    print cmd % (method,method)
    #system(cmd % (method,method))
