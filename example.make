###########################
## PARAMETERS TO FILL IN ##
###########################
#where to store all the files
OUT_DIR=demo
# 2, 3 or 4
N=2
CORPUS=data/czeng-navajo.en
# if .gz => files will be compressed
OUT_SUFFIX=.gz
# size of the candidates to annotate, percent or absolute
CANDIDATE_SAMPLE_SIZE=100
SCORES_TO_COMPUTE=1 2 3 4 5 6 9 17 18 19 20 21 22 24 25 26 29 30 31 32 33 34 35 36 38 40 42 44
# smoothing parameters for some of the scores
SMOOTH_PARAM=0.5
# logistic_regression, LDA, SVM
MODEL_TYPE=logistic_regression
# percentage of training data among annotated data
TRAIN_RATIO=80
# identical pseudo-randomness for identical seed
SEED=0
# 1 if you don't want to filter out based on freq counts
MIN_FREQ_COUNT=1
# nothing or --immediate to extract immediate context
IMM_CONTEXT=--immediate
# nothing or --broad to extract broad context
BROAD_CONTEXT=--broad
# Careful with those : If you don't want those 2 filters, remove from command line also the -t (tag filtering)
# remove all those tags from both sides.
TAG_FILTER_OUT="(CC|CD|DT|POS|PRP|\W+|-.RB-)"
# only select NN.* tags in context
CONTEXT_TAG_FILTER_IN="NN.*"

############################
##       FILE NAMES       ##
############################
CANDIDATES_F=$(OUT_DIR)/candidates_filtered$(OUT_SUFFIX)
STATS=$(OUT_DIR)/stats$(OUT_SUFFIX)
CANDIDATES=$(OUT_DIR)/candidates$(OUT_SUFFIX)
SCORES=$(OUT_DIR)/scores$(OUT_SUFFIX)
SAMPLED_C=$(OUT_DIR)/sampled_cand$(OUT_SUFFIX)
TRAIN=$(OUT_DIR)/train$(OUT_SUFFIX)
TEST=$(OUT_DIR)/test$(OUT_SUFFIX)
APPLIED_TEST=$(OUT_DIR)/applied_test$(OUT_SUFFIX)
TEST_RESULTS=$(OUT_DIR)/results$(OUT_SUFFIX)
MODEL=$(OUT_DIR)/model_$(MODEL_TYPE)

all: 
	@echo "type make phase1 (steps before annotation) or make phase2 (steps after annotation"

phase1: $(CANDIDATES) $(CANDIDATES_F) $(STATS) $(SCORES) $(SAMPLED_C)

phase2: $(TRAIN) $(MODEL) $(APPLIED_TEST) $(TEST_RESULTS)

.DELETE_ON_ERROR:

$(CANDIDATES): $(CORPUS)
	mkdir -p $(OUT_DIR)
	extract_candidates -d -n $(N) -c $(CORPUS) -o $(CANDIDATES)

$(CANDIDATES_F): $(CANDIDATES)
	filter_candidates -n $(N) -i $(CANDIDATES) -o $(CANDIDATES_F) -f $(MIN_FREQ_COUNT)-10000000
	filter_candidates -n $(N) -i $(CANDIDATES_F) -o $(CANDIDATES_F) -t $(TAG_FILTER_OUT):.* -r
	filter_candidates -n $(N) -i $(CANDIDATES_F) -o $(CANDIDATES_F) -t .*:$(TAG_FILTER_OUT) -r

$(STATS): $(CORPUS) $(CANDIDATES_F) 
	extract_statistics -d -n $(N) $(IMM_CONTEXT) $(BROAD_CONTEXT) -c $(CORPUS) -i $(CANDIDATES_F) -o $(STATS) # -t $(CONTEXT_TAG_FILTER_IN)

$(SCORES): $(STATS) 
	compute_scores $(SCORES_TO_COMPUTE) -i $(STATS) -o $(SCORES) -s $(SMOOTH_PARAM)

$(SAMPLED_C): $(SCORES) 
	sample_candidates.py -i $(SCORES) -s $(CANDIDATE_SAMPLE_SIZE) -o $(SAMPLED_C) -r $(SEED)

$(TRAIN): $(SAMPLED_C)
	sample_data.py -i $(SAMPLED_C) -t $(TRAIN) -s $(TEST) -p $(TRAIN_RATIO) -r $(SEED)

$(MODEL): $(TRAIN)
	train_model.py -t $(TRAIN) -p $(MODEL_TYPE) -o $(MODEL)

$(APPLIED_TEST): $(TRAIN) $(MODEL) 
	apply_model.py -i $(TEST) -m $(MODEL) -o $(APPLIED_TEST)

$(TEST_RESULTS): $(APPLIED_TEST)
	test_model.py -i $(TEST) -m $(MODEL) -o $(TEST_RESULTS)
