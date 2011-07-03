#!/bin/bash

cd ../../src/client/shaders

for file in *.fp *.vp
do
  m4 "$file" > "../../../resources/$file"
done
