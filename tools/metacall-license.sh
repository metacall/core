#!/bin/bash

#
#	MetaCall License Bash Script by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	License bash script utility for MetaCall.
#

EXEC_PATH="`pwd`"

find "$EXEC_PATH" -type f -not -path "*/build*" -not -path "*/source/tests/googletest*" -exec sh -c ' \

	COPYRIGHT="Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>$"

	comment=$(grep "$COPYRIGHT" {})

	if [ ! -z "$comment" ]
	then
		file=$(grep -lrnw "{}" -e "$COPYRIGHT")

		linenum=$(grep -n "{}" -e "$COPYRIGHT" | grep -Eo "^[^:]+")

		echo $file
		echo $linenum

		# expr match "$comment" "\#*" >/dev/null
		# # expr match "$comment" "\ \**" >/dev/null

		# if [ $? -eq 0 ]
		# then
		# 	# echo "$comment"
		# 	echo "$file"
		# 	# filename=$(basename "$file")
		# 	# extension=${filename##*.}
		# 	# echo "$filename -- $extension"
		# fi
	fi
' \;
