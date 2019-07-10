{
	"targets" : [
		{
			'target_name': 'metacall',
			"include_dirs":[
				"<!@(node -p \"require('node-addon-api').include\")",
				"include/"
			],
			"libraries" : [
				"<!@(node ./lib.js)"
			],
			'dependencies': [
				"<!(node -p \"require('node-addon-api').gyp\")"
			],
			"sources": [ "./source/node_port.cc"],
			"cflags!": [ "-fno-exceptions" ],
			"cflags_cc!": [ "-fno-exceptions" ]
		}
	]
}
