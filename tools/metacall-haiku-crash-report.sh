#!/usr/bin/env sh

#
#	MetaCall Haiku Crash Report Script by Parra Studios
#	Configure and install MetaCall environment script utility.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

echo
echo "===== HAIKU CRASH REPORTs ====="
echo
cat ~/Desktop/*report* 2>/dev/null
echo
echo "===== HAIKU SYSLOG TAIL ====="
echo
tail -n 200 /boot/system/var/log/syslog 2>/dev/null
