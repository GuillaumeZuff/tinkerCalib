{
    "targets": [{
        "target_name": "tinkercalib",
        "sources": ["src/tinkerCalib.cc", "src/calibration.cpp", "src/posConverter.cpp"],
        "include_dirs": [
            "<!(node -p \"require('node-addon-api').include_dir\")",
            "<!@(node utils/find-opencv.js --include)"
        ],
        "libraries": [
            "<!@(node utils/find-opencv.js --libs)"
        ],
        "defines": ["NAPI_VERSION=8", "NAPI_CPP_EXCEPTIONS"],
        "cflags!": ["-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions", "-fno-rtti"],
        "cflags_cc": ["-std=c++17"],
        "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "GCC_ENABLE_CPP_RTTI": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
            "MACOSX_DEPLOYMENT_TARGET": "11.0"
        }
    }]
}
