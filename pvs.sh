#!/bin/sh

pvs-studio-analyzer trace -- qmake
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze -o hydroid.log
plog-converter -a GA:1,2 -d V1042 -t tasklist -o hydroid.tasks hydroid.log
