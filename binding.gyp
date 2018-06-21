{
    "targets": [{
    	"target_name": "tinkercalib",
    	"sources": ["src/tinkerCalib.cc", "src/calibration.cpp", "src/posConverter.cpp"],
        "include_dirs": [
            '/usr/local/include'
        ],
    	"libraries": [
            '<!@(pkg-config --libs opencv)'
    	],
        "cflags_cc!": ['-fno-rtti', '-fno-exceptions'
        ],
        'cflags!': ['-fno-exceptions'],
    }],
}
