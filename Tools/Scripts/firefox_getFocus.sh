#!/bin/bash
# firefox need to execute "Runtime.enable" first on target
# then with an added delay execute "Runtime.evaluate"!

function delayed_pipe()
{
	while read l ; do
		echo $l
		sleep 0.1
	done
}

function process_result()
{
	while read l ; do
		result=$(echo $l | grep '"result":{"type":"string"')

		if [[ "$result" ]]; then
			result=$(echo $result | grep visible)

			if [[ "$result" ]]; then
				URL=$(echo "$URL" | cut -d\" -f4)
				echo $URL
			fi
		fi
	done
}

CURL_OUT=$(curl -s http://127.0.0.1:9222/json/list)
WS_URL=$(echo "$CURL_OUT" | grep -A 2 '"type": "page",')
WS_URL=$(echo "$WS_URL" | grep '"webSocketDebuggerUrl"')
WS_URL=$(echo "$WS_URL" | cut -d\" -f4)
#echo "$WS_URL"
#WS_URL=$(echo "$WS_URL" | grep "webSocket" | cut -d\" -f4)

# {"id": 1, "method": "Runtime.evaluate" , "params": {"expression": "document.visibilityState"}}


#websocat known bug: https://github.com/vi/websocat/issues/23
for i in $WS_URL; do
	
	URL=$(echo "$CURL_OUT" | grep -B 1 $i | head -n1)
	#replace localhost with 127.0.0.1
	i=${i/"localhost"/"127.0.0.1"}
	cat cmd | delayed_pipe | websocat "$i" | process_result
	#exit 0
	if [[ "$?" == 1 ]]; then
		exit 0
	fi
done