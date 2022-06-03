from sys import argv,exit
from os import system
from os.path import *
import xlsxwriter 
from subprocess import *
import numpy as np

def make_title(options):
    
    if options['Type'] == 'time':
        title = "%s of %s %s over time" % (options['Agg'],options['Stat'],options['Attribute'])
    else:
        title = "%s of %s %s" % (options['Type'],options['Stat'],options['Attribute'])

    return title

def read_options(line):
    opt = dict()
    
    for l in line:
        l = l.split('=')
        opt[l[0]] = l[1]
    
    print opt
    return opt

def process_options(process_statistics,options,families):
    
    cmd = [process_statistics]
    cmd += ["--i"]
    
    for f in families:
        cmd += [f]
    
    cmd += ["--type"]
    types = options['Type'].split(':')
    cmd += [types[0]]
    cmd += [options['Parameter']]
    
    for t in types[1:]:
        cmd += [t]
    
    if options['Type'] == 'time':
        cmd += ["--family-agg"] + [options['Agg']]
    
    cmd += ["--attribute"] + [options['Attribute']]
    cmd += ["--stat"] + [options['Stat']]
    
    print cmd
    proc = Popen(cmd,stdout=PIPE)
    output = proc.communicate()[0]
    
    data = []
    for l in output.split('\n'):
        l = l.split()
        print l
        if len(l) == 1:
            data += float(l[0])
        elif len(l) > 1:
            data += [[float(ll) for ll in l]]
    
    
    data = np.array(data)
    if options['Type'][0:4] == 'hist':
        data = data[::2,:]
        
        
    return data
    
def write_data(title,data,sheet,col):
    width = len(data.shape)
    sheet.merge_range(0,col,0,col+width-1,title,merge_format)
    if width == 1:
        for i in xrange(0,data.shape[0]):
            sheet.write(i+1,col,data[i])
        
    else:
        for i in xrange(0,data.shape[0]):
            for j in xrange(0,data.shape[1]):
                sheet.write(i+1,col+j,data[i,j])
    
    return width


    
# How much free space between columns
column_sep = 1

if len(argv) < 5:
    print "Usage: %s <proc_stat> <config> <family1> ... <familyn> <output.xlsx>" % argv[0]
    exit(0)
    
process_statistics = abspath(argv[1])
config = open(argv[2])
workbook = xlsxwriter.Workbook(argv[-1])
merge_format = workbook.add_format({'align': 'center'})

families = argv[3:-1]

lines = config.readlines()
i = 0;
while i < len(lines):
    line = lines[i].split()
    i +=  1
   
    if len(line) > 0 and line[0] == 'Start':
        if line[1] == "Sheet":
            sheet = workbook.add_worksheet(line[2])
            col = 0
            line = lines[i].split()
            print line
            while len(line) == 0 or line[0] != "Stop":
                
                if len(line) == 0 or line[0] != 'Data:':
                    i += 1
                    line = lines[i].split()
                    continue
                
                options = read_options(line[1:])
                title = make_title(options)
                data = process_options(process_statistics,options,families)
                col += write_data(title,data,sheet,col)
                col += column_sep
                
                i += 1
                line = lines[i].split()
            i += 1
                
workbook.close()