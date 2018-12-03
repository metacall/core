#!/usr/bin/python3

#
#	RSA String Encriptation Decriptation Sample by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
