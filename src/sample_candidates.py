#! /usr/bin/env python
import sys
import argparse
import random
import time
from tools import ScoreLine, FileType, count_lines, percentage
import numpy as np

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Samples randomly candidates to label""")
    parser.add_argument('-i', dest = 'scores_file',
                        type = FileType('r'), required = True,
                        help = ("file containing the candidates & scores to "
                                "sample"))
    parser.add_argument('-o', dest = 'annotated_file',
                        type = FileType('w'), required = True,
                        help = "file that will contain sampled candidates")
    parser.add_argument('-s', dest = 'sample_size', 
                        required = True, 
                        help = ("size of the sample (number of candidates) in" 
                                " absolute value or percentage (N%)"))
    parser.add_argument('-r', dest = 'seed', type = int, 
                        default = int(time.time()),
                        help = "seed to use to randomize")
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    # computing sample size (percent or absolute value)

    nlines = count_lines(args.scores_file) 
    if args.sample_size[-1] == '%':
        percent = percentage(args.sample_size)
        args.sample_size = int(percent * nlines)
    else:
        args.sample_size = int(args.sample_size)
    if args.sample_size == 0:
        args.sample_size = 1 #min value to sample

    print "Sampling candidates from file" , args.scores_file.name
    print "Outputting" , args.sample_size , "sampled candidates into file",(
                      args.annotated_file.name)
    print "Seed used (useful to replicate sampling) :" , args.seed

    lines = []

    for line in args.scores_file: # while there is a file not entirely read
        lines.append(ScoreLine(line))

    random.seed(args.seed)
    sample = random.sample(lines, args.sample_size);
    write_to_output = lambda line: args.annotated_file.write(str(line) + '\n')
    for line in sample:
        write_to_output(line)
