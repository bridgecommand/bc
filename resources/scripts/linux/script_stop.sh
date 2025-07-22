#!/bin/bash

#Stop server
cd
ps aux | grep bridgecommand-es | grep -v grep > serverStatus.log
if [ -s serverStatus.log ]; then
        # EnetServer is already running
        echo ">>>>>> Stop EnetServer"
        PID_BC_ES="$(pidof bridgecommand-es)"
        sudo kill -9 $PID_BC_ES
else
        # EnetServer is not running
        echo ">>>>>> EnetServer is currently not running"
fi

rm -rf serverStatus.log
cd -
