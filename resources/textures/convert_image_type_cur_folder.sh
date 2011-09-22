#!/bin/bash

if [ $# != 2 ]
then
  echo "Usage: `basename $0` old_file_suffix new_file_suffix"
  exit 1
fi


for filename in *.$1
do
  # Strip off part of filename matching 1st argument, then append 2nd argument.
  convert $filename ${filename%$1}$2
done

exit 0
