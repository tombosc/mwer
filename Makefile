CC=g++
CXX0X=-std=c++0x
BOOST_REGEX=-lboost_regex
BOOST_FS=-lboost_filesystem -lboost_system
BOOST_IO=-lboost_iostreams
OBJ_DIR=obj/
OBJS=obj/parser.o obj/word_type.o obj/abstract_candidate.o obj/candidate.o obj/shared.o obj/shared.o obj/token.o obj/candidate_filter.o obj/context_candidate.o obj/candidate_extractor.o obj/statistic_extractor.o obj/score_calculator.o

HEADERS=$(wildcard src/*.h)

LD_FLAGS=$(BOOST_REGEX) $(BOOST_FS) $(BOOST_IO) 
CFLAGS=-c -Wall $(CXX0X) -g -Werror -Isrc/ -Itest/ -O3
EXEC=extract_candidates filter_candidates extract_statistics compute_scores
EXEC_TEST=extractor_test extract_candidates_test merge_statistics_test

all: $(OBJ_DIR) $(EXEC)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

obj/shared.o: src/shared.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

obj/%.o: src/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

obj/%.o: test/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@


check: $(EXEC_TEST)
	rm -rf tmp

shared_test: obj/shared.o obj/shared_test.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

parser_test: obj/parser.o obj/parser_test.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

scores_test: obj/score_calculator.o obj/scores_test.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

extract_candidates: $(OBJS) obj/extract_candidates.o 
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

# non regression tests
extract_candidates_test: extract_candidates data/czeng-navajo.en
	mkdir -p tmp
	./extract_candidates -d -n 2 -c data/czeng-navajo.en -o tmp/out1.txt
	diff candidates/czeng-navajo.en.dn2.txt tmp/out1.txt
	rm -rf tmp/out1.txt
	
extractor_test: obj/candidate_extractor.o obj/candidate_filter.o obj/word_type.o obj/candidate.o obj/extractor_test.o obj/shared.o obj/token.o obj/abstract_candidate.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)
	mkdir -p tmp
	sh scripts/extractor_test.sh tmp/out2.txt
	diff scripts/expected_extractor_test.txt tmp/out2.txt
	rm -rf tmp/out2.txt

# Run this non deterministic test and diff by hand :
# Reason : Types in a context are not ordered.
extract_statistics_test: extract_statistics data/czeng-navajo.en candidates/czeng-navajo.en.dn2.txt
	mkdir -p tmp
	./extract_statistics -d -n 2 --immediate -i candidates/czeng-navajo.en.dn2.txt -c data/czeng-navajo.en -o tmp/out3.txt
	diff statistics/czeng-navajo.en.dn2.i.txt tmp/out3.txt
	./extract_statistics -s --adjacent -n 3 --immediate -i candidates/czeng-navajo.en.sn3adj.txt -c data/czeng-navajo.en -o tmp/out4.txt
	diff statistics/czeng-navajo.en.sn3adj.i.txt tmp/out4.txt
	rm -rf tmp/out4.txt
	
merge_statistics_test: statistics/czeng-navajo.en.dn2.i.txt
	mkdir -p tmp
	sh scripts/merge_statistics_test.sh statistics/czeng-navajo.en.dn2.i.txt tmp/out5.txt
	diff statistics/czeng-navajo.en.dn2.i.txt tmp/out5.txt
	rm -rf tmp/out5.txt

filter_candidates: $(OBJS) obj/filter_candidates.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

extract_statistics: $(OBJS) obj/extract_statistics.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

compute_scores: $(OBJS) obj/compute_scores.o
	$(CC) $(CXX0X) $^ -o $@ $(LD_FLAGS)

clean:
	rm -f obj/*.o $(EXEC) $(EXEC_TEST)

doc: 
	doxygen doxyfilerc	

.PHONY: all clean doc
