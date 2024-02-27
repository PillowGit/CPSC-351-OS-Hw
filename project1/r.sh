#!/usr/bin/env bash

# If there's no parameters or flag is to compile all
if [[ $# -eq 0 || "$1" == "all" ]]; then
	# Compile and link all cpp files in cwd and subdirectories
	cpp_files=$(find . -name "*.cc" -o -name "*.cpp")
	if [[ -z "$cpp_files" ]]; then
		echo "Didn't find any C++ files."
	else
		clang++ $cpp_files -std=c++23 -g -o src
	fi
else
	# Compile and link all given files
	clang++ "$@" -std=c++23 -g -o src
fi
