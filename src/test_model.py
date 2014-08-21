#! /usr/bin/env python
import sys
import argparse
from sklearn import metrics
from sklearn.metrics import precision_recall_curve, average_precision_score
from sklearn.externals import joblib
import numpy as np

from tools import ScoreLine, FileType, Model

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Tests model""")
    parser.add_argument('-i', dest = 'input_test', 
                        type = FileType('r'), required = True,
                        help = "file of scores and annotated test data")
    parser.add_argument('-m', type = Model(), 
                        required = True, dest = 'input_model',
                        help = "file containing model configuration")
    parser.add_argument('-o', dest = 'output_eval', 
                        type = FileType('w'), required = True,
                        help = "file that will contain evaluation candidates")
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    model = args.input_model
    print "Testing model" , model.name
    print "applied on" , args.input_test.name
    print "Outputting results in" , args.output_eval.name

    lines, ranking = model.apply_model(args.input_test, is_annotated = True)
    lines, ranking = zip(*sorted(zip(lines, ranking), key=lambda i: i[1], reverse = True)) 
    y_ann = [l.annotation for l in lines]
    str_ann = ["1" if a else "0" for a in y_ann]
    precision = np.ones(len(lines))
    recall = np.zeros(len(lines))
    for i in range(0, len(lines)):
        TP = float(np.count_nonzero(y_ann[:i+1]))
        FN = float(np.count_nonzero(y_ann[i+1:]))
        FP = float(i+1) - TP
        TN = float(len(y_ann)) - float(i) - FN
        precision[i] = TP / (TP + FP)
        recall[i] = TP / (TP + FN)
        args.output_eval.write(lines[i].output(ranking[i]) + '\t' + str_ann[i]
                                + ' ' + str(precision[i]) + ' ' + str(recall[i]) + 
                                '\n')

    print "Average precision :" , str(average_precision_score(y_ann, ranking))

    #TODO: Show plot... if plt is installed
    import matplotlib.pyplot as plt
    print recall
    print precision
    plt.plot(recall, precision, linewidth=2)
    plt.xlabel('recall')
    plt.ylabel('precision')
    plt.show()
