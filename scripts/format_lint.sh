#!/bin/dash
scripts=$(pwd)
base=$(pwd)/..
cd "$base" || exit
ct_cmd () {
    clang-tidy --checks="bugprone-*,performance-*,readability-*,portability-*,\
        clang-analyzer-*,cpp-coreguidelines-*,modernize-a*,modernize-c*,\
        modernize-d*,modernize-l*,modernize-m*,modernize-p*,modernize-r*,\
        modernize-s*,modernize-t*,modernize-un*,modernize-use-a*,\
        modernize-use-b*,modernize-use-c*,modernize-use-d*,modernize-use-e*,\
        modernize-use-n*,modernize-use-o*,modernize-use-s*,modernize-use-tran*,\
        modernize-use-u*" --extra-arg="-std=c++20" -p \
        "$base/compile_commands.json" "$1"
}

cf_cmd() {
    clang-format -style=file -i "$1"
}

echo "Formatting files..."
dir="$base/src/"
cf_cmd "$dir"*.?pp

dir="$base/src/Windows/"
cf_cmd "$dir"*.?pp

dir="$base/src/Application/"
cf_cmd "$dir"*.?pp

dir="$base/src/Logging/"
cf_cmd "$dir"*.?pp

dir="$base/src/Widgets/"
cf_cmd "$dir"*.?pp
# Excluded checks:
#   modernize-use-trailing-return-type
#   Because I don't like that style
#       float my_function(int number) {}
#   is better than
#       auto my_function(int number) -> float {}
dir="$base/src/"
ct_cmd "$dir"*.?pp
cpplint "$dir"*.?pp

dir="$base/src/Windows/"
ct_cmd "$dir"*.?pp
cpplint "$dir"*.?pp

dir="$base/src/Logging/"
ct_cmd "$dir"*.?pp
cpplint "$dir"*.?pp

dir="$base/src/Widgets/"
ct_cmd "$dir"*.?pp
cpplint "$dir"*.?pp

shellcheck "$scripts/"*.sh
cd "$scripts" || exit
