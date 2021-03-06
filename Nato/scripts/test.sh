#!/bin/bash
#test.sh <Mode>
#r: record and decode
#t: use t2 directory to decode
source path.sh

DECODE_PATH="decode"
WAV_DIR="$DECODE_PATH/wav"
AUDIO_PATH="$DECODE_PATH/audio"
RESULT_PATH="$DECODE_PATH/result"
LAT_CONF="$RESULT_PATH/lat_conf.ark"
#WORDS=$($SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH")

if [[ "$1" == "r" ]]; then

	echo "Rec Mode"
	REC_TIME="5"
	$SD/record.sh "$DECODE_PATH/wav/rec1.wav" $REC_TIME

elif [[ "$1" == "t" ]]; then

	echo "Test Mode"
	TEST_DIR="audio/t2/"
    
    find "${TEST_DIR}" -type f > list_file

    while read in_line; do
	    
	    cp "$in_line" "$DECODE_PATH/wav/rec1.wav"
		echo "$in_line"
		#ffmpeg -i "$in_line" -hide_banner -loglevel error -y -af "pan=stereo|c0=c0|c1=c0" "$DECODE_PATH/wav/rec1.wav" 
        $SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" 1
		
    done <list_file

    rm list_file
    exit 0

fi

time $SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" 1