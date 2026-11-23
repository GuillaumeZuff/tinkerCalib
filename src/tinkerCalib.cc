// Node-API binding (node-addon-api) for the calibration code.
//
// JS interface (unchanged since 0.2.x):
//   calibrate(options)            -> { camera?, renderer?, success }
//   calibrateFromPoints(options)  -> { camera?, renderer?, success }
// where camera/renderer are { projection: number[16], modelview: number[16] }.
//
// options: frameSize [w, h], nearPlane, farPlane, chessboardSize [w, h],
// chessboardCellSize, intrinsic number[9], distortion number[<=5],
// filename (calibrate), imagePoints / objectPoints (calibrateFromPoints,
// flat [x0, y0, x1, y1, ...] arrays of the same length).
//
// Unlike 0.2.x, invalid input and OpenCV errors throw a JS Error instead of
// crashing the Node process.

#include <napi.h>
#include <exception>
#include <string>
#include <vector>
#include "calibration.h"

namespace {

template<class T>
std::vector<T> getArray(const Napi::Object &obj, const char *name) {
    Napi::Value val = obj.Get(name);
    if (!val.IsArray()) {
        throw Napi::TypeError::New(obj.Env(), std::string(name) + " must be an array of numbers");
    }
    Napi::Array array = val.As<Napi::Array>();
    std::vector<T> vect;
    vect.reserve(array.Length());
    for (uint32_t i = 0; i < array.Length(); ++i) {
        vect.push_back(static_cast<T>(array.Get(i).ToNumber().DoubleValue()));
    }
    return vect;
}

template<class T>
std::vector<T> getSizedArray(const Napi::Object &obj, const char *name, size_t minSize, size_t maxSize) {
    std::vector<T> vect = getArray<T>(obj, name);
    if (vect.size() < minSize || vect.size() > maxSize) {
        std::string msg = std::string(name) + " must have " + std::to_string(minSize);
        if (maxSize != minSize) msg += " to " + std::to_string(maxSize);
        throw Napi::TypeError::New(obj.Env(), msg + " entries");
    }
    return vect;
}

int getInt(const Napi::Object &obj, const char *name) {
    return static_cast<int>(obj.Get(name).ToNumber().DoubleValue());
}

Napi::Array toJsArray(Napi::Env env, const std::vector<float> &values) {
    Napi::Array array = Napi::Array::New(env, values.size());
    for (uint32_t i = 0; i < values.size(); ++i) {
        array.Set(i, Napi::Number::New(env, values[i]));
    }
    return array;
}

Napi::Object getCalibrationResult(Napi::Env env, const Calibration &calib, bool isCamera) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("projection", toJsArray(env, calib.getProjectionMatrix(isCamera)));
    result.Set("modelview", toJsArray(env, calib.getMvMatrix(isCamera)));
    return result;
}

void configureCalibration(const Napi::CallbackInfo &info, Calibration &calib, std::string &filename) {
    if (info.Length() < 1 || !info[0].IsObject()) return;
    Napi::Object obj = info[0].As<Napi::Object>();

    if (obj.Has("frameSize")) {
        std::vector<int> v = getSizedArray<int>(obj, "frameSize", 2, 2);
        calib.setFrameSize(cv::Size(v[0], v[1]));
    }
    if (obj.Has("nearPlane")) {
        calib.setNearPlane(getInt(obj, "nearPlane"));
    }
    if (obj.Has("farPlane")) {
        calib.setFarPlane(getInt(obj, "farPlane"));
    }
    if (obj.Has("chessboardSize")) {
        std::vector<int> v = getSizedArray<int>(obj, "chessboardSize", 2, 2);
        calib.setChessboardSize(cv::Size(v[0], v[1]));
    }
    if (obj.Has("chessboardCellSize")) {
        calib.setChessboardCellSize(getInt(obj, "chessboardCellSize"));
    }
    if (obj.Has("intrinsic")) {
        calib.setIntrinsicMatrix(getSizedArray<float>(obj, "intrinsic", 9, 9));
    }
    if (obj.Has("distortion")) {
        calib.setDistortionMatrix(getSizedArray<float>(obj, "distortion", 0, 5));
    }
    if (obj.Has("filename")) {
        filename = obj.Get("filename").ToString().Utf8Value();
    }
}

void getCalibrationPoints(const Napi::CallbackInfo &info, std::vector<float> &imagePoints, std::vector<float> &objectPoints) {
    if (info.Length() < 1 || !info[0].IsObject()) return;
    Napi::Object obj = info[0].As<Napi::Object>();
    if (obj.Has("imagePoints")) {
        imagePoints = getArray<float>(obj, "imagePoints");
    }
    if (obj.Has("objectPoints")) {
        objectPoints = getArray<float>(obj, "objectPoints");
    }
    if (imagePoints.size() % 2 != 0 || imagePoints.size() != objectPoints.size()) {
        throw Napi::TypeError::New(info.Env(),
            "imagePoints and objectPoints must be flat [x, y, ...] arrays of the same length");
    }
}

// Runs the camera then the renderer calibration, building the same result
// object as 0.2.x: {camera, renderer, success}, stopping at the first failure.
template<class F>
Napi::Value runCalibration(Napi::Env env, F calibrateOne, Calibration &calib) {
    Napi::Object result = Napi::Object::New(env);
    bool success = true;
    try {
        if (calibrateOne(true)) {
            result.Set("camera", getCalibrationResult(env, calib, true));
        } else {
            success = false;
        }
        if (success && calibrateOne(false)) {
            result.Set("renderer", getCalibrationResult(env, calib, false));
        } else {
            success = false;
        }
    } catch (const Napi::Error &) {
        throw;
    } catch (const std::exception &e) {
        throw Napi::Error::New(env, std::string("tinkercalib: ") + e.what());
    }
    result.Set("success", Napi::Boolean::New(env, success));
    return result;
}

Napi::Value Calibrate(const Napi::CallbackInfo &info) {
    Calibration calib;
    std::string filename;
    configureCalibration(info, calib, filename);
    return runCalibration(info.Env(), [&](bool isCamera) {
        return calib.calibrate(filename, isCamera);
    }, calib);
}

Napi::Value CalibrateFromPoints(const Napi::CallbackInfo &info) {
    Calibration calib;
    std::string filename;
    configureCalibration(info, calib, filename);
    std::vector<float> imagePoints, objectPoints;
    getCalibrationPoints(info, imagePoints, objectPoints);
    return runCalibration(info.Env(), [&](bool isCamera) {
        return calib.calibrateFromPoints(imagePoints, objectPoints, isCamera);
    }, calib);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("calibrate", Napi::Function::New(env, Calibrate, "calibrate"));
    exports.Set("calibrateFromPoints", Napi::Function::New(env, CalibrateFromPoints, "calibrateFromPoints"));
    return exports;
}

}  // namespace

NODE_API_MODULE(tinkercalib, Init)
