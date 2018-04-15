{
  'targets': [
    {
      'target_name': 'trampoline',
      'win_delay_load_hook': 'true',
      'sources': [ 'source/trampoline.cc' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
           'ExceptionHandling': 1
        },
        'VCLinkerTool': {
           'SetChecksum': 'true'
        }
      },
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS': [
          '-std=c++11'
        ],
        'CLANG_CXX_LIBRARY': 'libc++',
        'OTHER_LDFLAGS': [],
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'include_dirs': [
        "include"
      ],
      'conditions': [
        ['OS == "win" and MSVS_VERSION == "2015"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'AdditionalOptions': [
                # disable Thread-Safe "Magic" for local static variables
                '/Zc:threadSafeInit-',
              ],
            },
          },
        }],
        ['OS!="win"', {
          'cflags_cc+': [
            '-std=c++0x'
          ]
        }]
      ]
    }
  ]
}
