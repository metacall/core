#!/bin/bash

#
#	MetaCall License Bash Script by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	License bash script utility for MetaCall.
#

# Warning
echo "WARNING: Do not run the script multiple times. Uncomment the 'exit 0' in the code to continue."
exit 0

# Execution path
EXEC_PATH="`pwd`"

# Replacement
find "$EXEC_PATH" -type f \
	-not -path "*/build*" \
	-not -path "*/source/tests/googletest*" \
	-not -name "LICENSE" \
	-not -name "COPYRIGHT" \
	-exec sh -c ' \

	# Copyright
	COPYRIGHT="Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>$"

	# License
	LICENSE=$(cat <<-END

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
	END
	)

	comment=$(grep "$COPYRIGHT" {})

	if [ ! -z "$comment" ]
	then
		file=$(grep -lrnw {} -e "$COPYRIGHT")
		linenum=$(grep -n {} -e "$COPYRIGHT" | cut -d : -f 1)

		# Swap description and copyright from the header
		printf %s\\n $(($linenum + 2))m$(($linenum - 1)) w q | ed -s $file
		printf %s\\n $(($linenum + 1))m$(($linenum + 2)) w q | ed -s $file

		# Select between comment type
		expr match "$comment" "\#*" >/dev/null
		# expr match "$comment" " \**" >/dev/null

		if [ $? -eq 0 ]
		then
			lineliteral="i"

			# Apply prefix depending on comment type
			license=$(echo "$LICENSE" | sed "s/^/#\t/g")
			# license=$(echo "$LICENSE" | sed "s/^/ \*\t/g")

			expression="$(($linenum + 3))$lineliteral|$license"

			# TODO: Review $linenum expansion error, remove the pipe to null when solved
			ex -s -c "$expression" -c x "$file" &> /dev/null
		fi
	fi
' \;
