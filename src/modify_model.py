#! /usr/bin/env python
import sys
import argparse
import code
from functools import partial
from tools import Model

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
                        View, modify a model and its parameter in a python
                        interpreter""")
    parser.add_argument('-m', dest = 'model', type=Model(), 
                        required = True,
                        help = "file that will contain model configuration")

    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)

    model = args.model.model    
    standardizer = args.model.standardizer
    save = args.model.save
    print "model =" , model
    print "standardizer =", standardizer
    print 
    print "save() will save the model in its current location."
    print "save(filename) will save the model in filename."
    print "exit() will exit."
    code.interact(local=locals())
