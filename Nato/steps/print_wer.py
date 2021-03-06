# written in python 3
# Author: MP Zanoosi Aug-2020
# Example: python print_wer.py exp/tri1/graph exp/tri1/decode

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float

GRA = sys.argv[1] #1
DIR = sys.argv[2] #2
AUD_PATH = "audio/train"
WRD_FILE = "word_list"
SYM_FILE = f"{GRA}/words.txt"
OUT_FILE = f"input"
MIN_LMWT = 7
MAX_LMWT = 17

def printWord(word):
	if( len(word)<2 ):
		return

	w_filename = word[0]
	w_filename = w_filename.split('.')[0]
	word_id = w_filename.split('_')

	orig_id = ""
	orig = "| "
	for i in range(1,len(word_id)):
		orig += lexicon[int(word_id[i])]
		orig += ","
		orig_id += word_id[i]
		orig_id += "_"
	orig = orig[:-1] #remove last char
	orig_id = orig_id[:-1] #remove last char

	det = " ->  "
	for i in range(1,len(word)):
		det += symbols[int(word[i])]
		det += ","
	det = det[:-1] #remove last char

	is_wrong = 0
	if( len(word)!=len(word_id) ):
		is_wrong = 1

	for i in range(1,len(word_id)):
		word1 = lexicon[int(word_id[i])]
		word2 = ""
		if( i<len(word)):
			word2 = symbols[int(word[i])]
		if( word1!=word2 ):
			is_wrong = 1
			break

	if( is_wrong ):
		print(f"{orig_id:<8} {orig:<25} {det}")
		out_file.write(f"{AUD_PATH}/{word_id[0]}/{orig_id}.wav {orig:<25} {det}\n")

word_file = open(WRD_FILE)
syms_file = open(SYM_FILE)
out_file  = open(OUT_FILE, "w")
lexicon = word_file.read().splitlines()
symbols = syms_file.read().splitlines()

for i in range(1,len(symbols)):
	symbols[i] = symbols[i].split()[0]


for i in range(MIN_LMWT, MAX_LMWT+1):
	print(f"####### lmwt={i:<2} #######")

	#print WER and SER
	cmd = f"egrep '(WER)|(SER)' < {DIR}/wer_{i}"
	subprocess.run(cmd, shell=True)

	sc_filename = f"{DIR}/scoring/{i}.tra"
	for line in open(sc_filename):
		word = line.split()
		printWord(word)
  
out_file.close()

#remove duplicates
os.system("sort -u -o input input")