#!/bin/sh
if [[ -e "/tmp/logs2m.pid" ]]; then
	status=$(ps up `cat /tmp/logs2m.pid ` >/dev/null && echo "1" || echo "0")
	if [[ $status -eq "0" ]]; then
		rm /tmp/logs2m.pid
		nohup /usr/local/logs2m/logs2m.py &
	fi
else
	nohup /usr/local/logs2m/logs2m.py &
fi
