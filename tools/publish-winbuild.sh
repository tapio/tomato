#!/bin/sh
# Script name: 
# Description: 
# Version:     

DATE=`date -R`

make install && git checkout winbuild && cp --recursive tomaatti/* .. && git commit -a -m "Win32 build $DATE" && git push github winbuild && git checkout master

