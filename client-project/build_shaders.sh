#!/bin/bash

cd ../..
export project_root=`pwd`
export source_dir="${project_root}/src/client/shaders"
export macro_processor="${project_root}/utils/Macro_processor/build/macro_processor"
export output_dir="${project_root}/resources"

cd $source_dir
for file in *.fp *.vp
do
  $macro_processor "$file" > "$output_dir/$file"
done
