#!/usr/bin/env sh

#
#	MetaCall Python Port Deploy Script by Parra Studios
#	Script utility for deploying MetaCall Python Port to PyPi.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

# TODO: Update version in setup.py
# TODO: Automate for CD/CI

# Define exit code
fail=0

# Install dependencies and upload MetaCall package
python3 -m pip install --user --upgrade twine setuptools wheel \
	&& python3 setup.py sdist bdist_wheel \
	&& python3 -m twine check dist/* \
	&& python3 -m twine upload dist/* || fail=1

# Delete output
rm -rf dist/* build/* || fail=1

# Exit
exit ${fail}
