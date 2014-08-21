#! /usr/bin/env python
import sys
import argparse
from sklearn import linear_model, lda, svm, preprocessing
from sklearn.externals import joblib
import numpy as np

from tools import ScoreLine, FileType, count_lines, percentage, remove_nan_inf

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Trains model""")
    parser.add_argument('-t', dest = 'input_train',
                        type = FileType('r'), required = True,
                        help = "file containing annotated candidates")
    parser.add_argument('-o', dest = 'output_model',
                        required = True,
                        help = "file that will contain model configuration")
    parser.add_argument('-p', dest = 'model_param', required = True, 
                        choices = ['logistic_regression','LDA',
                        'SVM'],
                        default='logistic_regression', help = "model to fit")
    # TODO : param d
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    print "Training model from file" , args.input_train.name
    print "Outputting model in file" , args.output_model

    lines = []

    nlines = count_lines(args.input_train)
    y_train = np.zeros(shape = nlines)
    X_train = None

    for i, line in enumerate(args.input_train):
        l = ScoreLine(line, annotated = True)
        if i == 0:
            X_train = np.zeros(shape = (nlines, len(l.scores)))
        lines.append(l)
        X_train[i] = l.scores
        y_train[i] = int(l.annotation)
    X_train = remove_nan_inf(X_train)
    standardizer = preprocessing.StandardScaler().fit(X_train)

    model = None 
    if args.model_param == 'logistic_regression':
        model = linear_model.LogisticRegression()
    elif args.model_param == 'LDA':
        model = lda.LDA()
    elif args.model_param == 'SVM':
        model = svm.LinearSVC()
    else:
        print "Model not implemented yet"
        sys.exit(1)

    model.fit(X_train, y_train)
    print "Score of the fit : " , model.score(X_train, y_train)

    joblib.dump((standardizer, model), args.output_model, compress=1) #in one file
