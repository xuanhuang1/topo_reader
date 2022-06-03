from sys import argv,exit
from talassDB import *


stat = StatisticsDB()
stat.initialize(argv[1:])

#stat.loadAttribute(argv[-2],argv[-1])

#d = stat.getValues(2.5e-5)
