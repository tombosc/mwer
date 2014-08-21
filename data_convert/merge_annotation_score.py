#! /usr/bin/env python

# Merge a score file to append to each line the annotation (last field of
# an annotated file)

import argparse
from itertools import izip

parser = argparse.ArgumentParser()
parser.add_argument('-i', dest = 'input_scores',
                    type = argparse.FileType('r'), required = True,
                    help = "file containing scores")
parser.add_argument('-a', dest = 'input_annot',
                    type = argparse.FileType('r'), required = True,
                    help = "file containing annotations")
parser.add_argument('-o', dest = 'output_scores_annot', 
                    type = argparse.FileType('w'), required = True,
                    help = "file that will contain scores & annotations")

try: 
    args = parser.parse_args()
except IOError as e:
    print "Error: " + e.filename + " : " + e.strerror
    sys.exit(1)

for line_score, line_annot in izip(args.input_scores, args.input_annot):
    fields_annot = line_annot.split('\t')
    annotation = fields_annot[-1]
    line_score = line_score.rstrip('\n')
    args.output_scores_annot.write(line_score + '\t' + annotation)
