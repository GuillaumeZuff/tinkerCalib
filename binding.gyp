{
  "targets": [{
    "target_name": "tinkercalib",
    "sources": ["src/tinkerCalib.cc", "src/calibration.cpp", "src/posConverter.cpp"],
		'conditions': [
			['OS=="mac"', {
                "include_dirs": [
                    '/usr/local/include'
                ],
    		"libraries": [
        	"/usr/local/lib/libopencv_core.dylib",
        	"/usr/local/lib/libopencv_calib3d.dylib"
    		]
			}],
			['OS=="linux"', {
    		"libraries": [
        	"/usr/lib/x86_64-linux-gnu/libopencv_core.so",
        	"/usr/lib/x86_64-linux-gnu/libopencv_calib3d.so"
    		]
			}],

		]
  }],
}
