#!/bin/dash
# Clean up old files
scripts=$(pwd)
base=$(pwd)/..
cd "$base" || exit
echo "Cleaning up past build!"
rm -r ./docs/*

# Build with emacs
echo "\nBuilding with emacs!"
emacs -Q --script "$scripts/make_docs.el"

# Time to shrink the files!
echo "\nMinify-ing files!"
cd "$base/docs/" || exit
ls -1 | grep html | parallel 'minify {} -o {}.new; mv {}.new {}'
cd "$base" || exit

# Cleanup
echo "\nCleaning up latex/pdf"
cd "$base/src/docs" || exit
rm ./*.tex ./*.pdf

# Compress and rename pdf
cd "$base/docs" || exit
echo "\nCompressing pdf"
orig_size=$(ls -l | grep index | grep pdf | awk '{print $5}')
gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.5 -dNOPAUSE -dQUIET -dBATCH -dPrinted=false -sOutputFile=index-compressed.pdf index.pdf 
mv index-compressed.pdf PsSp_manual.pdf
rm index.pdf
new_size=$(ls -l | grep manual | grep pdf | awk '{print $5}')

percent=$(echo 'scale=4; ('"$new_size"'/'"$orig_size"') * 100' | bc)
percent=$(echo "$percent" | python -c "print(round(float(input()), 1))")
orig_size=$(echo 'scale=4; '"$orig_size"' / 1000.0' | bc)
orig_size=$(echo "$orig_size" | python -c "print(round(float(input()), 2))")
new_size=$(echo 'scale=4; '"$new_size"' / 1000.0' | bc)
new_size=$(echo "$new_size" | python -c "print(round(float(input()), 2))")

echo "($orig_size kB, $new_size kB, $percent%)\n"
cd "$scripts" || exit
