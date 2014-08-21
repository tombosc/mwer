#! /usr/bin/env python
import sys
import argparse
from sklearn import decomposition, preprocessing
from tools import ScoreLine, count_lines, percentage, remove_nan_inf
import numpy as np


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        Random sampling of candidates to label""")
    parser.add_argument('-i', dest = 'annotated_scores',
                        type = argparse.FileType('r'), required = True,
                        help = "file containing annotated scores")
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    nlines = count_lines(args.annotated_scores)
    y = np.zeros(shape = nlines)
    X = None

    for i, line in enumerate(args.annotated_scores):
        l = ScoreLine(line, annotated = False)
        if i == 0:
            X = np.zeros(shape = (nlines, len(l.scores)))
        X[i] = l.scores
        #y[i] = int(l.annotation)

    X = remove_nan_inf(X)
    standardizer = preprocessing.StandardScaler().fit(X)
    X = standardizer.transform(X)

    pca = decomposition.PCA()
    pca.fit(X)
    var_2 = np.sum(pca.explained_variance_ratio_[:1])
    print "The 2 first components explain" , pca.explained_variance_ratio_[0], 
    print "+" , pca.explained_variance_ratio_[1] , "% of total variance"
    eigen_1 = pca.components_[:,0]
    eigen_2 = pca.components_[:,1]
    print eigen_1
    print eigen_2
    x_values = X * eigen_1
    y_values = X * eigen_2
    color = ['r' if i == 0 else 'b' for i in y]
    scale = [10 if i == 0 else 50 for i in y] # ratio 20%

   #import matplotlib.pyplot as plt
   #plt.scatter(x_values, y_values, c=color, s=scale, alpha = 0.5)
   #plt.show()
