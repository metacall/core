#!/usr/bin/env bash

#
#	MetaCall Python Port Deploy Script by Parra Studios
#	Script utility for deploying MetaCall Python Port to PyPi.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

set -exuo pipefail

PYPI_VERSION=$(curl -s https://pypi.org/rss/project/metacall/releases.xml | sed -n 's/\s*<title>\([0-9.]*\).*/\1/p' | sed -n '2 p')
PORT_VERSION=$(cat VERSION)

if [[ "$PYPI_VERSION" == "$PORT_VERSION" ]]; then
	echo "Current package version is the same as PyPI version, skipping upload."
	exit 0
fi

TWINE_USERNAME=${PYTHON_PYPI_USERNAME:-}
TWINE_PASSWORD=${PYTHON_PYPI_PASSWORD:-}

# Install dependencies and upload MetaCall package
python3 -m pip install --user --upgrade twine setuptools wheel
python3 setup.py sdist bdist_wheel
python3 -m twine check dist/*
python3 -m twine upload -u ${TWINE_USERNAME} -p ${TWINE_PASSWORD} dist/*

# Delete output
rm -rf dist/* build/*
