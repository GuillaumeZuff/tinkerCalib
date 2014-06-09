{
  "targets": [{
    "target_name": "tinkerCalib",
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
        	"/usr/lib/libopencv_core.so",
        	"/usr/lib/libopencv_calib3d.so"
    		]
			}],

		]
  }],
}
