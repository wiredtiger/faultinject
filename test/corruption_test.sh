#!/bin/sh
set -e

run_corruption_test() {
	dir=$1 
	tables=`./wt -h $dir list`
	echo "tables listed for $dir:"
	echo $tables
	for table in $tables; do
		if [[ "$table" =~ "*:*" ]]; then
			echo "Run corruption test on $dir, table:$table"
			./wt -h $dir verify $table
		fi
	done
}       

dirs=`find $1 -name "WiredTiger.wt" -exec dirname {} \;`
for dir in $dirs; do
	run_corruption_test $dir
done

