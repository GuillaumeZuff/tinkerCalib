{
    "targets": [{
    	"target_name": "tinkercalib",
    	"sources": ["src/tinkerCalib.cc", "src/calibration.cpp", "src/posConverter.cpp"],
        "include_dirs": [
            "<!(node -e \"require('nan')\")"
        ],
    	"libraries": [
            '<!@(pkg-config --libs opencv)'
    	]
    }],
}



            "<!@(node utils/find-opencv.js --libs)"
