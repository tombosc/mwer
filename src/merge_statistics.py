#! /usr/bin/env python
import sys
import argparse

from tools import StatisticLine, FileType

if __name__ == "__main__":
    stat_files = []
    parser = argparse.ArgumentParser(description="""
        Merges several statistic files into one file""")
    parser.add_argument('-i', nargs = '+', dest = 'stat_files',
                        type = FileType('r'), required = True,
                        help = ("2 or more files containing the statistics to "
                        "merge"))
    parser.add_argument('-o', dest = 'merged_file',
                        type = FileType('w'), required = True,
                        help = "file that will contain merged statistics")
    try: 
        args = parser.parse_args()
    except IOError as e:
        print "Error: " + e.filename + " : " + e.strerror
        sys.exit(1)


    if len(args.stat_files) == 1:
        print "Error: only 1 file supplied. Need at least 2 files to merge."
        sys.exit(1)

    print "Merging statistics from" , ",".join([f.name for f in args.stat_files])
    print "Ouputting results into" , args.merged_file.name

    sentence_counter = 0

    lines = [f.readline() for f in args.stat_files]

    stats = [StatisticLine(l) for l in lines]

    while args.stat_files: # while there is a file not entirely read

        sentence_counter += 1
        #print "debug : sentence number : " , sentence_counter
        
        write_to_merged_file = lambda stat: args.merged_file.write(str(stat) + '\n')

        # we pick the smallest (lexicographically speaking) candidate(s)
        min_value = min(stats)
        min_values = [i for i,s in enumerate(stats) if s == min_value]
        
        # if there are several equal candidates, we add them to the first
        if len(min_values) > 1:
            for l in min_values[1:]:   
                stats[min_values[0]].add(stats[l])

        write_to_merged_file(stats[min_values[0]])           
        
        # we override the previously minimal candidates(s)


        to_delete = []
        for i in min_values:
            lines[i] = args.stat_files[i].readline()
            # if the line is empty, we assume it's the end of the file (EOF)
            if not lines[i]:
                to_delete.append(i)
                args.stat_files[i].close()
            else: 
                stats[i] = StatisticLine(lines[i])

        # we remove files & stats that are EOF 
        args.stat_files = [i for i in args.stat_files if args.stat_files.index(i) not in to_delete]
        lines = [i for i in lines if lines.index(i) not in to_delete]
        stats = [s for s in stats if stats.index(s) not in to_delete]
