#! /usr/bin/env python
import sys
import argparse
from sklearn import preprocessing
from sklearn.externals import joblib
import numpy as np

from tools import ScoreLine, FileType, Model

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Applies model and output ranking""")
    parser.add_argument('-i', dest = 'input_scores',
                        type = FileType('r'), required = True,
                        help = "file containing candidates with scores")
    parser.add_argument('-m', dest = 'input_model',
                        type = Model(), required = True,
                        help = "file containing model configuration")
    parser.add_argument('-o', dest = 'output_rank', 
                        type = FileType('w'), required = True,
                        help = "file that will contain ranked candidates")
    parser.add_argument('-n', dest = 'max_candidates',
                        type = int, required = False, default = None,
                        help = "maximum size of the n-best list to be output")
    parser.add_argument('-t', dest = 'threshold',
                        type = float, required = False, default = None,
                        help = "file containing model configuration")
    #TODO : add an option to output image

    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    # loading model
    model = args.input_model

    print "Applying model" , model.name
    print "on data file" , args.input_scores.name
    print "Outputting ranking in file" , args.output_rank.name
    if args.threshold != None:
        print "Only ranks greater than", str(args.threshold) , "will be output"
    if args.max_candidates != None:
        print "Only the first", args.max_candidates, "best ranked candidates ",
        print "will be output"

    # whether it's annotated or not, we don't read the annotations
    lines, ranking = model.apply_model(args.input_scores, is_annotated = False)
    sorted_indexes = sorted(range(len(ranking)), 
                            key = lambda k: ranking[k], reverse = True)

    write_to_output = lambda output,line: output.write(str(line) + '\n')
    for i in sorted_indexes[:args.max_candidates]:
        if ranking[i] < args.threshold:
            break
        write_to_output(args.output_rank, lines[i].output(ranking[i]))
