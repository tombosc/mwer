MWER is a set of tools for "multiword expression (MWE)" or "collocation" extraction. It was written during a 3 months [internship](http://ufal.mff.cuni.cz/internship/) at [Institute of Formal and Applied Linguistics, Charles University in Prague](http://ufal.mff.cuni.cz/) under the supervision of [Pavel Pecina](http://ufal.mff.cuni.cz/~pecina/). His work on this subject is summarized in [these slides](http://ufal.mff.cuni.cz/~pecina/files/guf-2012-slides.pdf).

You can use [CzEng corpus](http://ufal.mff.cuni.cz/czeng) as a data source. Please see the [references data for collocation extraction](http://ufal.mff.cuni.cz/~pecina/resources.html) for the annotation (supervised learning part).

Features
--------
* Surface or dependency extraction
* Filtering in or out certain candidates according to their tags, lemmas, or frequency
* Filtering in certain context types
* Available models : Logistic regression, LDA or SVM (and possibly everything in scikit-learn)
* Gzip files support (I/O)

Limitations
-----------
* A multiword expression candidate contains between 2 and 4 tokens.
* A sentence is limited to 1024 words.
* Extraction of broad context is limited to a sentence.
* You cannot extract both surface and dependency candidates at once.

Requirements
------------
* C++11 compatible compiler (ex. : gcc >= 4.8)
* C++ libboost (regex, filesystem, system, iostreams) >= 1.40
* Python >= 2.7.3
* scikit-learn >= 0.14
* numpy >= 1.6.1

Install
--------
1. Run the makefile

		$ make
2. modify your shell path in order to call the executable anywhere

		$ export PATH=$PWD:$PATH
3. if you want the documentation, type

		$ make doc

Directories
------------
- src/ contains the source code
- test/ contains test source code
- data_convert/ contains other utility scripts to convert annotated data.
- html/ contains documentation if it's been generated with "$ make doc"

Quick start
---------------
Tools are meant to be ran one after the other, and output necessary file(s) for the next tool.
We can separate the whole process of MWE extraction in 2 phases. See *example.make* which demonstrates the whole toolset.

The turning point is the annotation of sampled candidates by a human expert. The steps before and after this steps are automatic.

	$ make -f example.make phase1

Then annotate the file located at $(SAMPLED_C) (see *annotate_sample*).
Then, type :

	$ make -f example.make phase2

You should have the output of the test in $(TEST_RESULTS).


Definitions
-----------
factors : we call factors pieces of information that are presents in types & tokens. We separate the factors by the character '|'

type : form or lemma, possibly with a tag  
example : dogs, dog or dog|NN

token : instance of a type, possibly with other factors such as id, parent id if syntactically annotated...

MWE candidate : n types forming a possible expression

distance of 2 tokens in a sentence : two adjacent words are distant of 1, whereas two words separated by a third word are distant of 2, etc...

surfacic distance of a candidate : maximal distance in a sentence between each pair of words

syntactical distance of a candidate : maximal distance in a tree-rep sentence between each pair of words (nodes)

dependency extraction : When a text is syntactically annotated, a tree representation of each sentence is build and candidates can be extracted using this tree. See faq for more info about syntactical distance.

surface extraction : The sentence is here seen as flat, and the extraction simply produce every possible group of tokens in the sentence. The allowed distance range can be tuned with -r.

Regex syntax
--------------
Refer to [boost regex documentation](http://www.boost.org/doc/libs/1_40_0/libs/regex/doc/html/boost_regex/syntax/perl_syntax.html)

extract_candidates
==================
Extracts MWE candidates.

	extract_candidates -n {2,3,4} -c CORPUS_FILE -o OUTPUT_FILE
	 {-d|-s} [-a] [-r dist_min-dist_max] [-f min-max]
	[-l regexp1:...:regexpn] [-t regexp1:...:regexpn]
	Mandatory : 
	  -n : 2,3 or 4
	  -c : input corpus file
	  -o : output candidates file
	  -d, --dependency	: dependency extraction OR
	  -s, --surface : surface extraction
	Optional : 
	  -a, --adjacent : extract candidates that are adjacent only
	  -r min-max : distance filter (accept matchs)
	  -f min-max : frequency filter (accept matchs)
	  -l regexp1:...:regexpn : regex filter for lemmas (accept matchs)
	  -t regexp1:...:regexpn : regex filter for tags (accept matchs)

Notes :
-------
* -a, --adjacent is equivalent to -r n-1:n-1
* You *have* to choose between -d or -s
* You can't choose dependency extraction if your text is not annotated
* -r, -f, -l and -t *accepts* matching candidates. You can't use them to remove candidates that match. Instead, you should use the tool *filter_candidates* 

filter_candidates
=================
Filters MWE candidates.

	filter_candidates -n {2,3,4} -i CANDIDATE_LIST -o FILTERED_LIST
	[-l regexp1:...:regexpn] [-t regexp1:...:regexpn] [-f min-max] [-r]
	Mandatory : 
	  -n : 2,3 or 4
	  -i : input candidate list file
	  -o : output filtered candidate list file
	Optional : 
	  -r : If set, reject matching candidates (applies to all filter)
	       If not set, reject non matching candidates
	  -f min-max : frequency filter
	  -l regexp1:...:regexpn : regex filter for lemmas
	  -t regexp1:...:regexpn : regex filter for tags

Example :
----------
You want to reject all candidates that contains : CC, CD, DT, POS, PRP and punctuation signs, for n=3.  
You could use : -t (CC|CD|DT|POS|PRP|\W+|-.RB-):.*:.* -r  
And then, do it again on the output to match at different positions : -t .*:(CC|CD|DT|POS|PRP|\W+|-.RB-):.* -r  
Finally : -t .*:.*:(CC|CD|DT|POS|PRP|\W+|-.RB-) -r 

extract_statistics
==================
Extracts MWE candidates' statistics.

	extract_statistics -n {2,3,4} -c CORPUS_FILE -o OUTPUT_FILE
	 -i CANDIDATES_FILE {-d|-s} [-a] [-r dist_min-dist_max]
	[--immediate] [--broad] [-t regexp1:...:regexpn]
	Mandatory : 
	  -n : 2,3 or 4
	  -c : input corpus file
	  -o : output candidates file
	  -i : input candidate list file
	  -d, --dependency	: dependency extraction OR
	  -s, --surface : surface extraction
	Optional : 
	  -a, --adjacent : extract candidates that are adjacent only
	  -r min-max : distance filter (accept matchs)
	  -t regexp1:...:regexpn : regex filter for tags of context (accept matchs)
	  --immediate : process immediate context
	  --broad : process broad context

By default, the tool only extract contingency table of the candidate. Here is how the contingency table is output :
a, b, c, d are types, and A, B, C, D are types other than a, b, c, d.
f is the function that counts occurences.
Contingency table order :
* n = 2 : f(ab) f(aB) f(Ab) f(AB)
* n = 3 : f(abc) f(abC) f(aBc) f(Abc) f(aBC) f(AbC) f(ABc) f(ABC)
* n = 4 : f(abcd) f(abcD) f(abCd) f(aBcd) f(Abcd) f(abCD) f(aBcD) f(aBCd) f(AbcD) f(AbCd) f(ABcd) f(aBCD) f(AbCD) f(ABcD) f(ABCd) f(ABCD)

You can also build context. A context is an unordered set of types appearing somewhere near the MWE candidate. *Immediate* context is constituted of left and right context. A type is added to left (right) context if a token occurence of it appears right before (after) an occurence of the MWE candidate. *Broad* context is the set of all types that appear in the same sentence as MWE candidate occurences.  

Notes :
-------
* Parameters -d or -s, -a & -r are meant to be the same as in extract candidates. You don't have to specify them, but if you do, it can speed up the execution, because you will select only candidates already in your list. On the other hand, if you specify them in a more restrictive way (rejecting more candidates than extracted), it will lead to bad counts. You should always put exactly the same
* -t can only be used to filter through, but not to reject. Such a filter, if used, will be applied on contexts. If neither broad or immediate context is extracted, it is useless.
* -t is right now slow (cf. To do section). 

Example :
---------
If you only want nouns in a context, you'd use : -t NN.*  
It would allow only NN, NNS, NNP, NNPS tags in contexts.

merge_statistics.py
===================
	usage: merge_statistics.py [-h] -i STAT_FILES [STAT_FILES ...] -o MERGED_FILE
	
	Merges several statistic files into one file
	
	optional arguments:
	  -h, --help            show this help message and exit
	  -i STAT_FILES [STAT_FILES ...]
	                        2 or more files containing the statistics to merge
	  -o MERGED_FILE        file that will contain merged statistics

It can be used this way :

* extract MWE candidates of a long text
* split a text in n parts
* compute the stats of each part with the *same* candidate list (important, else the contingency table will be all wrong)
* merge the stats together with this tool

compute_scores
==============
	Computes scores
	compute_scores s1 [s2 ... sn] -i input_stat -o output_scores 
	[-s smoothing_parameter]
	Mandatory : 
	  s1 [s2 ... sn] : scores to compute
	  -i : input stat file
	  -o : output score file
	Optional : 
	  -s : smoothing parameter (default=0.5)

Implemented scores :
See [slide number 10/30](http://ufal.mff.cuni.cz/~pecina/files/guf-2012-slides.pdf)
* Contingency table based : 1-9, 17-44
* Immediate context based : 57-60
* Broad context based : 72 68 75 77 81

Smoothing :
The smoothing is done by adding the smoothing parameter to occurences that never happens. If f(aB) = 0 (i.e. aB has never been observed), we use smoothed(f(aB)) = smoothing_parameter. Smoothed values are only used when needed (denominators for instance). Therefore, it is advised to choose the parameter between 0 and 1. If it's greater than 1, the smoothed value will have a stronger weight than a value that actually appeared in the corpus.

Note :
* Scores 56 to 60 require immediate context in stat file
* Scores 61 to 82 requies broad context in stat file
* Supplying an incorrect / not implemented score number will result in a score of 0 for all candidates.
* Scores specific for 3-grams or 4-grams are not implemented, but should not be too different to implement.
* In order to implement a score yourself, please refer to score_calculator.cpp and learn from already implemented scores.

sample_candidates.py
====================
	usage: sample_candidates.py [-h] -i SCORES_FILE -o ANNOTATED_FILE -s
	                            SAMPLE_SIZE [-r SEED]
	
	Random sampling of candidates to label
	
	optional arguments:
	  -h, --help         show this help message and exit
	  -i SCORES_FILE     file containing the candidates & scores to sample
	  -o ANNOTATED_FILE  file that will contain sampled candidates
	  -s SAMPLE_SIZE     size of the sample (number of candidates) in absolute
	                     value or percentage (N%)
	  -r SEED            seed to use to randomize

Annotate your samples
=====================
This step is performed by a (human) expert. It should add a tab in the end of each line, and a 0 or a 1. 0 should indicate that the MWE candidate is NOT a proper MWE, whereas 1 should indicate that it is a MWE.

sample_data.py
==============
	usage: sample_data.py [-h] -i ANNOTATED_FILE -t TRAIN_FILE -s TEST_FILE -p
	                      SAMPLE_RATIO [-r SEED]
	
	Samples randomly candidates to label
	
	optional arguments:
	  -h, --help         show this help message and exit
	  -i ANNOTATED_FILE  file containing the annotated candidates to sample
	  -t TRAIN_FILE      file that will contain candidates for training purposes
	  -s TEST_FILE       file that will contain candidates for testing purposes
	  -p SAMPLE_RATIO    percentage of the annotated_file that will be output as
	                     training data
	  -r SEED            seed to use to randomize

train_model.py
==============
	usage: train_model.py [-h] -t INPUT_TRAIN -o OUTPUT_MODEL -p
	                      {logistic_regression,LDA,SVM}
	
	Trains model
	
	optional arguments:
	  -h, --help            show this help message and exit
	  -t INPUT_TRAIN        file containing annotated candidates
	  -o OUTPUT_MODEL       file that will contain model configuration
	  -p {logistic_regression,LDA,SVM}
	                        model to fit

The output, the model, is dumped as a python tuple containing the standardizer and the model itself. The standardizer is applied on every score independently, allowing the scores to have a mean of 0 and a variance of 1. You can inspect both object with *modify_model*, which is presented in the next section.

The different models can produce very different effects if used with different parameters. Please refer to the documentation of scikit for each model to tune it properly.

modify_model.py
===============
	usage: modify_model.py [-h] -m MODEL
	
	View, modify a model and its parameter in a python interpreter
	
	optional arguments:
	  -h, --help  show this help message and exit
	  -m MODEL    file that will contain model configuration

This tool allows you to modify and save your model (possibly under another filename).
Use save() to save the model in the same file.
Use save("filename") to save the model in filename.
Use exit() or Ctrl-D to exit.

apply_model.py
==============
	usage: apply_model.py [-h] -i INPUT_SCORES -m INPUT_MODEL -o OUTPUT_RANK
	                      [-n MAX_CANDIDATES] [-t THRESHOLD]
	
	Applies model and output ranking
	
	optional arguments:
	  -h, --help         show this help message and exit
	  -i INPUT_SCORES    file containing candidates with scores
	  -m INPUT_MODEL     file containing model configuration
	  -o OUTPUT_RANK     file that will contain ranked candidates
	  -n MAX_CANDIDATES  maximum size of the n-best list to be output
	  -t THRESHOLD       file containing model configuration

test_model.py
=============
	usage: test_model.py [-h] -i INPUT_TEST -m INPUT_MODEL -o OUTPUT_EVAL
	
	Tests model
	
	optional arguments:
	  -h, --help      show this help message and exit
	  -i INPUT_TEST   file of scores and annotated test data
	  -m INPUT_MODEL  file containing model configuration
	  -o OUTPUT_EVAL  file that will contain evaluation candidates

The output is an annotated candidate list with ranking, with precision and recall values in the 2 last columns.

To do :
=======
* Implement a proper smoothing such as "Simple Good Turing"
* Fix extract_statistics -t option. In the current implementation, the regex match function is applied every time a word is to be added in a context. Instead, we should store the result of the match in a boolean in the WordType itself in order to match the regex only once.
* Implement a filter that does not load everything in the memory, as it is currently, but read fixed size batch of lines.
* Change broad context size (hardcoded value = 1 sentence). 

FAQ :
======
How are the filters implemented ?
----------------------------------
Type filtering is done, but never token filtering. That means that all the filtering is done when all the text is read and processed. It's easier this way. Also, token filtering can mess with marginal law counts and such.

Why do we allow to filter distance based on the distance in the sentence but not on the syntactic distance ?
------------------------------------------------------------------------------------------------------------
Such a filter would only be useful on 4/5/6 grams but not on bigrams.  

In dependency extraction, to build a candidate, we need to build a tree representation. Then, the candidate would be a subtree itself. MWE must have syntactical "unity", that is the path between every types (nodes) in a candidate (tree) consists of types actually belonging to the candidate. We can't have a bigram syntactically separated by a word.

If you build different trees with :
* 2 nodes (bigrams) : syntactic distance can only be 1
* 3 nodes (trigrams) : syntactic distance can only be 2
* 4 nodes (4-grams) : syntactic distance is 2 or 3

Implementing such a complex filter would only be used in n=4 cases. Although, n=4 is not often used in language modelling.
