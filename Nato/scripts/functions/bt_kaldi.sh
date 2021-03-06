#!/bin/bash
# helper functions for reading from input

VLC_OPT="-q --gain 5 --play-and-exit --no-repeat --no-loop"

function checkAudio()
{
	FILE_PATH="$1"
	FILENAME=$(echo "$FILE_PATH" | awk -F '/' '{print $NF}')
	
	WORDS=$(python3 scripts/train/print_name.py "$FILENAME")
	# \e[A: one line up - \e[K: clear line
	U_COUNT=$(find "$AUD_DIR" -type f | wc -l)
	V_COUNT=$(find "$VER_DIR" -type f | wc -l)
	printf "\r\e[A\e[K\r($U_COUNT -> $V_COUNT) $WORDS     \n"
	printf "\r\e[K\r"
	sleep 1
	cvlc $VLC_OPT $FILE_PATH 2>/dev/null &

	LEN_WAV=$(soxi -D "$FILE_PATH")
	LEN_WAV=$(echo "$LEN_WAV*5/1" | bc) #float to int
	LEN_WAV=$(( $LEN_WAV + 15 ))
	
	LEN_REMAIN="0"
	for (( i = 0 ; i <= $LEN_WAV ; i++ )) ; do 
		read -rs -N1 -t0.2 REP </dev/tty
		
		if [[ "$REP" ]]; then
			printf "\e[A\e[K\r$FILENAME Removed: $WORDS\n\n"
			rm "$FILE_PATH"
			LEN_REMAIN=$(echo "($LEN_WAV-$i)/5" | bc)
			break
		fi
		printf "|"
	done

	if [[ "$LEN_REMAIN" == 0 ]]; then
		mv "$FILE_PATH" "audio/train/online/"
	else
		sleep "$LEN_REMAIN"
	fi
}