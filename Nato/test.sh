#!/bin/bash
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

elif [[ "$1" == "v" ]]; then

	echo "Verify Mode"
    
    steps/decode.sh --config conf/decode.config --nj 1 exp/tri1/graph data/train exp/tri1/decode
    
    exit 0

fi

time $SD/decode.sh "$DECODE_PATH" "$AUDIO_PATH" "$RESULT_PATH" 1
#time $SD/create_conf.sh decode 0.08 0.027
#$SK/visualize.sh "$LAT_CONF" "$RESULT_PATH" #visualize
#$SD/print_words.sh decode 0.9
#cat "$RESULT_PATH/confidence"
