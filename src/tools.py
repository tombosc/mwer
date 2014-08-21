from collections import Counter
from collections import OrderedDict
import numpy as np
from sklearn.externals import joblib
from argparse import FileType as ft
import gzip
import sys


class Model(ft):
    """Model file type"""
    def __call__(self, string):
        try:
            self.name = string
            self.standardizer, self.model = joblib.load(self.name)
            return self
        except IOError as e:
            print "Error: Model", self.name, ":" ,  e[1]
            sys.exit(1)
        except IndexError:
            print "Error: Model", self.name, 
            print ": not a valid model file"
            sys.exit(1)

    def apply_model(self, input_file, is_annotated):
        """Apply model on score file

        Args:
            input_file: Text file to read
            is_annotated: True if the text is annotated and annotations
            need to be store

        Returns:
           Tuple (ScoreLine, ranking) with ranking a score
        """
        lines = []
        X = None
        nlines = count_lines(input_file)
        for i, line in enumerate(input_file):
            l = ScoreLine(line, annotated = is_annotated)
            if i == 0:
                X = np.zeros(shape = (nlines, len(l.scores)))
            lines.append(l)
            X[i] = l.scores

        #TODO document and supprimer fonction intermediarie?
        X = remove_nan_inf(X)
        X = self.standardizer.transform(X)
        ranking = self.model.decision_function(X).tolist()
        return (lines, ranking)

    def save(self, filename=None):
        if filename == None:
            filename = self.name
        joblib.dump((self.standardizer, self.model), filename,
                    compress=1) #in one file

class FileType(ft):
    """Raw text file or .gz'ed text file"""
    def __call__(self, string):
        if string.endswith('.gz'):
            return gzip.open(string, self._mode)
        return super(FileType, self).__call__(string)

# recipe found in python official documentation
class OrderedCounter(Counter, OrderedDict):
    """
    Counter that remembers the order elements are first encountered
    """

    def __repr__(self):
     return '%s(%r)' % (self.__class__.__name__, OrderedDict(self))

    def __reduce__(self):
     return self.__class__, (OrderedDict(self),)

class Line:
    def __hash__(self):
        return sum([hash(t) for t in self.types])
 
    def __eq__(self, stat):    
        """Return true if 2 lines refer to to the same candidate"""
        return self.types == stat.types

    def __lt__(self, stat):
        """Return true if stat is lexicographically inferior to self"""
        # in the case of a file containing broad context statistics, 
        # we have unigrams and n-grams
        # unigrams are stored first (because they are needed to
        # compute scores) so we need to compare the length of types also
        return (len(self.types_split), self.types_split) < (
                (len(stat.types_split), stat.types_split))

class StatisticLine(Line):
    """
    Statistical informations of a MWE

    A statistic line is composed of :
    - One or several word types 
    - One or several counters
    - Zero, one or several countexts

    To make documentation more readable, examples provided assume that : 
    CONST_SEP_WORDS = ' '
    CONST_SEP_TYPE_FREQ = ':' 
    CONST_SEP_SECTIONS = '\t'
    CONST_SEP_FACTORS = '|'
    """

    CONST_SEP_SECTIONS = '\t'
    CONST_SEP_WORDS = ' '
    CONST_SEP_TYPE_FREQ = ':'
    CONST_SEP_FACTORS = '|'

    def build_context_dict(self,freq_list):
        """
        Build an OrderedCounter out of a string freq_list
    
        Args:
            freq_list: a list of strings representing pairs 
            (types, frequencies). Pairs' elements are separated by
            CONST_SEP_TYPE_FREQ.

        Returns: an OrderedCounter with types as keys and frequencies as values

        Example : freq_list = ["dog:3", "cat:1", "Sheep:10"] will return
            {"dog":3, "cat":1, "Sheep":10} in this order
        """
        if freq_list == ['']:
            return OrderedCounter()

        split_type_freq = lambda s: s.rsplit(self.CONST_SEP_TYPE_FREQ,1)
        od = OrderedCounter()
        for e in freq_list:
            w,f = split_type_freq(e)
            od[w] = int(f) 
        return od
 

    def __init__(self, line):
        """
        Args: 
            line: a string in the format : type_1 .. type_n \t 
            contingency_table \t left_context \t right_context \t broad_context
            OR type_1 \t freq \t broad_context
            With contextes being a list of type_i:frequency
        """
        line = line.rstrip('\n')
        lines_sec_separated = line.split(self.CONST_SEP_SECTIONS)
        split_in_words = lambda s: (s.split(self.CONST_SEP_WORDS))

        self.types = [s for s in split_in_words(lines_sec_separated[0])]
        # need to split the types to compare them with __lt__
        self.types_split = [t.split(
                        self.CONST_SEP_FACTORS) for t in self.types]

        self.contingency_table = [int(x) for x in split_in_words(
                                        lines_sec_separated[1])]

        # we build a list of contexts
        self.contexts = []
        for str_context in lines_sec_separated[2:]:
            split_context = split_in_words(str_context)
            self.contexts.append(self.build_context_dict(split_context))

    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, str(self))

    def __str__(self):
        # convert a split context to a string
        context_to_str = lambda c: (
            [w + ":" + str(f) for (w,f) in c.items()])
        append_joined_list = lambda list, sublist: (
            list.append(self.CONST_SEP_WORDS.join(sublist)))

        r = []
        append_joined_list(r, self.types)
        append_joined_list(r, [str(i) for i in self.contingency_table])
        for c in self.contexts:
            append_joined_list(r, context_to_str(c))
        return self.CONST_SEP_SECTIONS.join(r)
      
    def add(self, line):
        """Add the statistics of all objects in the list to the object"""
        # if 2 candidates are the same, we add their frequencies
        # and all their context informations
        self.contingency_table = (x+y for x,y in (
                        zip(self.contingency_table, line.contingency_table)))
        if len(self.contexts) != len(line.contexts):
            raise ParamError("""Can't add %s to %s : statistic sizes are 
                                differents""" % (str(self), str(stat)))

        for i in range(len(self.contexts)):
            self.contexts[i] += line.contexts[i]

class ScoreLine(Line):
    """
    Score informations of a MWE

    A score line is composed of :
    - One or several word types 
    - One or several scores

    To make documentation more readable, examples provided assume that : 
    CONST_SEP_WORDS = ' '
    CONST_SEP_TYPE_FREQ = ':' 
    CONST_SEP_SECTIONS = '\t'
    CONST_SEP_FACTORS = '|'
    """

    CONST_SEP_SECTIONS = '\t'
    CONST_SEP_WORDS = ' '
    CONST_SEP_TYPE_FREQ = ':'
    CONST_SEP_FACTORS = '|'
    annotated = False

    def __init__(self, line, annotated = False):
        """
        Args: 
            line: a string in the format : type_1 .. type_n \t score1 .. scoren
        """
        line = line.rstrip('\n')
        lines_split = line.split(self.CONST_SEP_SECTIONS)
        split_in_words = lambda s: (s.split(self.CONST_SEP_WORDS))

        self.types = [s for s in split_in_words(lines_split[0])]

        self.scores = [float(x) for x in split_in_words(lines_split[1])]

        self.annotated = annotated

        if annotated:
            if len(lines_split) == 3:
                if lines_split[2] == '0':
                    self.annotation = False
                elif lines_split[2] == '1':
                    self.annotation = True
                else:
                    raise ParamError("""Can't recognize boolean in %s.""",
                            lines_split[2])
            else:
                self.scores = []
                self.annotation = bool(lines_split[1])

    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, str(self))

    def __hash__(self):
        return sum([hash(t) for t in self.types])
 
    def __eq__(self, stat):    
        """Return true if 2 lines refer to to the same candidate"""
        return self.types == stat.types

    def output(self, rank):
        r = self.CONST_SEP_WORDS.join(self.types)
        r = r + self.CONST_SEP_SECTIONS + str(rank) 
        return r


    def __str__(self):
        append_joined_list = lambda list, sublist: (
            list.append(self.CONST_SEP_WORDS.join(sublist)))

        r = []
        append_joined_list(r, self.types)
        append_joined_list(r, [str(i) for i in self.scores])
        if self.annotated:
            append_joined_list(r, [('1' if self.annotation else '0')])
        return self.CONST_SEP_SECTIONS.join(r)

def count_lines(f):
    begin_input = f.tell()
    i = 0
    for l in enumerate(f):
        i = i + 1
    f.seek(begin_input);
    return i

def percentage(s):
    """
    Args:
      s: a percentage in format of a string int%
    Returns:
      a floating value of the percentage s between 0 and 1
    """
    f = float(s.strip('%')) / 100.0
    if f < 0.0:
        f = 0.0
    if f > 1.0:
        f = 1.0
    return f

def remove_nan_inf(X):
    """Verbosely remove NaN & inf value from a numpy array, and return it"""
    try:
        if np.isnan(X).any() :
            lines_nan = [ str(i) for i,x in enumerate(X) if np.isnan(x).any()]
            print "Warning: Array contain NaN (not a number) value(s) "
            X = np.nan_to_num(X)
        if np.isinf(X).any() :
            lines_inf = [ str(i) for i,x in enumerate(X) if np.isinf(x).any()]
            print "Warning: Array contain inf (infinite) value(s)"
            X = np.nan_to_num(X)
    except TypeError as e:
        print X
        print e
    return X


#TODO : delete ?
#def write_model(output_file, standardizer, model):
#    """
#    Write means, variances & model coefficients in a output
#
#    Args:
#        output_file: an opened writable file 
#        standardizer: a sklearn.preprocessing.StandardScaler object
#        model: a sklearn model
#    """
#    means = standardizer.mean_
#    variances = np.sqrt(standardizer.std_)
#    write_to_output = lambda output,line: output.write(str(line) + '\n')
#    str_list = lambda list: [str(x) for x in list]
#    write_to_output(args.output_model, ' '.join(str_list(means)))
#    write_to_output(args.output_model, ' '.join(str_list(variances)))
#    write_to_output(args.output_model, ' '.join(str_list(model.raw_coef_)))
#    write_to_output(args.output_model, model.get_params())
#
#def read_model(input_file):
#    """
#    Read a model file
#    Args:
#        input_file: an opened readable file
#    Returns:
#        a tuple of (sklearn.preprocessing.StandardScaler, sklearn model)
#    """
#    
#
#
#    return (standardizer, model)
