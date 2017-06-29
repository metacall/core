#
#	RSA String Encriptation Decriptation Sample by Parra Studios
#	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

import rsa

unicode_string = u"Euro=\u20ac ABCDEFGHIJKLMNOPQRSTUVWXYZ"

def encript_decript_strings():
	(pub, priv) = rsa.newkeys(384)

	message = unicode_string.encode('utf-8')

	print("\tMessage:   %s" % message)

	encrypted = rsa.encrypt(message, pub)

	print("\tEncrypted: %s" % encrypted)

	decrypted = rsa.decrypt(encrypted, priv)

	print("\tDecrypted: %s" % decrypted)

	if message == decrypted:
		print("\tRSA encriptation / decriptation success")
	else:
		print("\tRSA encriptation / decriptation error")
