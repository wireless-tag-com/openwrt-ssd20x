#!/bin/sh

INCLUDE_MAGIC="wtinclude"

usage() {
	echo "$0 config_dir intput_file output_file"
	echo "config_dir: config file dir"
	echo "input_file: default config file"
	echo "output_file: output result to output_file"

	exit 1
}

# $1 config file base dir
# $2 file need to parse
# $3 output file
parse_file() {
	cat "$2" | while read line
	do
		inc_file=$(echo "$line" | grep "$INCLUDE_MAGIC" | awk '{printf $2}')

		if [ -n "$inc_file" ]; then
			if [ -f "$1/$inc_file" ]; then
				parse_file "$1" "$1/$inc_file" "$3"
			fi
		else
			echo "$line" >>"$3"
		fi
	done
}

if [ "$#" != "3" ]; then
	echo "$#"
	usage
fi

rm -f "$3"

parse_file "$1" "$2" "$3"

echo "" >>"$3"
