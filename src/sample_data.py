#! /usr/bin/env python
import sys
import argparse
import random
import time

from tools import ScoreLine, FileType, count_lines, percentage

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Random sampling of annotated candidates""")
    parser.add_argument('-i', dest = 'annotated_file',
                        type = FileType('r'), required = True,
                        help = ("file containing the annotated candidates to "
                                "sample"))
    parser.add_argument('-t', dest = 'train_file',
                        type = FileType('w'), required = True,
                        help = "file that will contain candidates "
                               " for training purposes")
    parser.add_argument('-s', dest = 'test_file',
                        type = FileType('w'), required = True,
                        help = "file that will contain candidates "
                               " for testing purposes")
    parser.add_argument('-p', dest = 'sample_ratio', required = True, 
                        help = ("percentage of the annotated_file that will be"
                                " output as training data"))
    parser.add_argument('-r', dest = 'seed', type = int, 
                        default = int(time.time()),
                        help = "seed to use to randomize")
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    nlines = count_lines(args.annotated_file)
    sample_ratio = percentage(args.sample_ratio)

    print "Sampling data from file" , args.annotated_file.name
    print "Seed used (useful to replicate sampling) :" , args.seed

    lines = []
    for line in args.annotated_file: # while there is a file not entirely read
        lines.append(ScoreLine(line, True))

    true_mwe, false_mwe = [], []
    for l in lines:
        (true_mwe if l.annotation else false_mwe).append(l)

    true_ratio = float(len(true_mwe)) / float(len(false_mwe))
    print "true : " , len(true_mwe) , " false : " , len(false_mwe)
    print "Ratio marked as true / marked as false candidates : " , true_ratio

    ScoreLine.print_scores = True
  
    random.seed(args.seed)
    random.shuffle(true_mwe);
    random.shuffle(false_mwe);

    true_in_training = int(round(sample_ratio * len(true_mwe)))
    false_in_training = int(round(sample_ratio * len(false_mwe)))
    true_in_test = len(true_mwe) - true_in_training
    false_in_test = len(false_mwe) - false_in_training
    print "Outputting" , true_in_training , "candidates marked as true and" , \
          false_in_training , "marked as false as training data in file", \
          args.train_file.name
    print "Ratio marked as true / marked as false : " , \
          float(true_in_training) / float(false_in_training)
    print "Outputting" , true_in_test , "candidates marked as true and" , \
          false_in_test , "marked as false as test data in file", \
          args.test_file.name
    print "Ratio marked as true / marked as false : " , \
          float(true_in_test) / float(false_in_test)
    
    write_to_output = lambda output,line: output.write(str(line) + '\n')

    for line in true_mwe[:true_in_training]:
        write_to_output(args.train_file, line)
    for line in false_mwe[:false_in_training]:
        write_to_output(args.train_file, line)

    for line in true_mwe[true_in_training+1:]:
        write_to_output(args.test_file, line)
    for line in false_mwe[false_in_training+1:]:
        write_to_output(args.test_file, line)
