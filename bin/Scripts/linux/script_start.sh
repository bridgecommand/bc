#!/bin/bash

#Start server
ps aux | grep bridgecommand-es | grep -v grep > serverStatus.log
if [ -s serverStatus.log ]; then
        # EnetServer is already running
        echo ">>>>>> EnetServer is already running"
else
        # EnetServer is not running
        echo ">>>>>> Start EnetServer"
        cd ~/bc/bin/
        ./bridgecommand-es &
fi

rm -rf serverStatus.log
