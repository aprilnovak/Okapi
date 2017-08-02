#!/bin/bash
EXT=fld
ext2=1
full_ext=

for i in *.${EXT}*; do

  if [ "${ext2}" -lt "10" ]; then
    full_ext=${EXT}0${ext2}
    echo ${full_ext}
  else
    full_ext=${EXT}${ext2}
 #   mv *.${EXT}${ext2} okapi-moose-nek.${EXT}
    echo ${full_ext}
  fi

  mv *.${full_ext} okapi-moose-nek.${full_ext}

  ext2=$((ext2+1))

done
