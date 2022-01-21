#!/usr/bin/env python3

#
#	RSA String Encriptation Decriptation Sample by Parra Studios
#	Python RSA encriptation decriptation sample.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

import rsa

unicode_string = u"Euro=\u20ac ABCDEFGHIJKLMNOPQRSTUVWXYZ"

def encript_decript_strings():
	(pub, priv) = rsa.newkeys(384)

	message = unicode_string.encode('utf-8')

	buf = "\tMessage:   %s\n" % message

	encrypted = rsa.encrypt(message, pub)

	buf += "\tEncrypted: %s\n" % encrypted

	decrypted = rsa.decrypt(encrypted, priv)

	buf += "\tDecrypted: %s\n" % decrypted

	if message == decrypted:
		buf += "\tRSA encriptation / decriptation success"
	else:
		buf += "\tRSA encriptation / decriptation error"

	return buf
