#!/bin/bash
original_dir=$(pwd)
tsung -f $1 start
cd ~/.tsung/log
rm latest
arr=( $(ls) )
last_log=${arr[${#arr[@]}-1]}
cd $last_log
/usr/lib/tsung/bin/tsung_stats.pl

# Save configuration file
cp $original_dir/$1 .
ln -s $last_log ../latest
cd $original_dir
