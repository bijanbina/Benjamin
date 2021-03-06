# written in python 3
# Author: MP Zanoosi Aug-2020

import random
import os
import time
import sys

AUDIO_DIR  = sys.argv[1]
SPEAKER    = sys.argv[2]
REC_NUMBER = int(sys.argv[3])
WORD_COUNT = int(sys.argv[4])

REC_TIME = "3"

directory = AUDIO_DIR + "/train/" + str(SPEAKER)
print("Directory = " + directory)

# create random name from 3 digit
def getRandomFileName(digit1, digit2, digit3):
	rnd = random.randint(0,5)

	if( rnd==0 ):
		return str(digit1) + "_" + str(digit2) + "_" + str(digit3) + ".wav"
	elif( rnd==1 ):
		return str(digit1) + "_" + str(digit3) + "_" + str(digit2) + ".wav"
	elif( rnd==2 ):
		return str(digit2) + "_" + str(digit3) + "_" + str(digit1) + ".wav"
	elif( rnd==3 ):
		return str(digit2) + "_" + str(digit1) + "_" + str(digit3) + ".wav"
	elif( rnd==4 ):
		return str(digit3) + "_" + str(digit1) + "_" + str(digit2) + ".wav"
	else:
		return str(digit3) + "_" + str(digit2) + "_" + str(digit1) + ".wav"
	

i = 0
while i<REC_NUMBER:
	i += 1
	digit1 = random.randint(0,WORD_COUNT)
	digit2 = random.randint(0,WORD_COUNT)
	#biased digit
	#digit3 = random.randint(0,WORD_COUNT-67) + 67
	#digit3 = random.randint(0,66)
	if( len(sys.argv)>5 ):
		digit3 = int(sys.argv[5])
	else:
		digit3 = random.randint(0,WORD_COUNT)
	
	file_name = getRandomFileName(digit1, digit2, digit3)
	file_path = str(directory) + "/" + file_name
	script = "find " + directory + " -name \"" + file_name + "\" > files"
	os.system(script)
	
	count = 0
	for line in open('files'): count += 1
	if count != 0:
		i -=1
		continue

	#print utterance
	script  = "scripts/train/putterance.sh "
	script += file_name
	script +=  " " + str(i) + " " + str(REC_NUMBER)
	os.system(script)

	# Record
	script  = "scripts/decode/record.sh "
	script += file_path + " " + REC_TIME
	ret = os.system(script)
	ret = os.waitstatus_to_exitcode(ret)

	if ret == 1: #pause
		i -=1
		continue
	elif ret == 2: #quit
		break
