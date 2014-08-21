#! /usr/bin/env python

# Convert old stat file with contingency table only (no contexts) to new
# file format

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-i', dest = 'input_stats',
                    type = argparse.FileType('r'), required = True,
                    help = "file containing stats (old format)")
parser.add_argument('-o', dest = 'output_stats', 
                    type = argparse.FileType('w'), required = True,
                    help = "file that will contain stats (new format)")

try: 
    args = parser.parse_args()
except IOError as e:
    print "Error: " + e.filename + " : " + e.strerror
    sys.exit(1)

for line in args.input_stats:
    fields = line.split('\t')
    type1 = '|'.join([fields[0], fields[1], fields[2]])
    type2 = '|'.join([fields[3], fields[4], fields[5]])
    args.output_stats.write(' '.join([type1, type2]))
    args.output_stats.write('\t')
    args.output_stats.write(' '.join(fields[6:]))
