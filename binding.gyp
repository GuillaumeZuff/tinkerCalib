{
    "targets": [{
    	"target_name": "tinkercalib",
    	"sources": ["src/tinkerCalib.cc", "src/calibration.cpp", "src/posConverter.cpp"],
    	"libraries": [
            '<!@(pkg-config --libs opencv)'
    	]
    }],
}
