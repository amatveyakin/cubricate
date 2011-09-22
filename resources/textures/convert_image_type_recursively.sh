#!/bin/bash

if [ $# != 2 ]
then
  echo "Usage: `basename $0` old_file_suffix new_file_suffix"
  exit 1
fi


find . -name "*$1" -exec `pwd`/convert_single_file.sh \{\} $1 $2 \;

exit 0
