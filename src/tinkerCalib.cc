#include <node.h>
#include <iostream>
#include "calibration.h"

using namespace std;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Number;
using v8::Boolean;
using v8::Context;

Isolate* isolate;
Local<Context> context;

//NAN_METHOD(Calibrate) {
//    NanScope();
//    Calibration calib;
//    string filename = "";
//
//    if (args[0]->IsObject()) {
//        Local<Object> obj = Local<Object>::Cast(args[0]);
//        if (obj->Has(NanNew<String>("frameSize"))) {
//            std::vector<int> vect = getArray<int>(obj, "frameSize");
//            cv::Size frameSize = cv::Size(vect[0], vect[1]);
//            calib.setFrameSize(frameSize);
//        }
//        if (obj->Has(NanNew<String>("nearPlane"))) {
//            int nearPlane = getNumber<int>(obj, "nearPlane");
//            calib.setNearPlane(nearPlane);
//        }
//        if(obj->Has(NanNew<String>("farPlane"))) {
//            int farPlane = getNumber<int>(obj, "farPlane");
//            calib.setFarPlane(farPlane);
//        }
//        if(obj->Has(NanNew<String>("chessboardSize"))) {
//            std::vector<int> vect = getArray<int>(obj, "chessboardSize");
//            cv::Size chessboardSize = cv::Size(vect[0], vect[1]);
//            calib.setChessboardSize(chessboardSize);
//        }
//        if (obj->Has(NanNew<String>("chessboardCellSize"))) {
//            int cellSize = getNumber<int>(obj, "chessboardCellSize");
//            calib.setChessboardCellSize(cellSize);
//        }
//        if (obj->Has(NanNew<String>("intrinsic"))) {
//            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
//            calib.setIntrinsicMatrix(intrinsic);
//        }
//        if (obj->Has(NanNew<String>("distortion"))) {
//            vector<float> distortion = getArray<float>(obj, "distortion");
//            calib.setDistortionMatrix(distortion);
//        }
//        if (obj->Has(NanNew<String>("filename"))) {
//            filename = getString(obj, "filename");
//        }
//    }
//
//    Local<Object> result;
//    bool success = true;
//    bool isCamera = true;
//    if (calib.calibrate(filename,isCamera)) {
//        Handle<Object> camera = getCalibrationResult(calib,isCamera);
//        result->Set(NanNew<String>("camera"), camera);
//    } else {
//        success = false;
//    }
//
//    isCamera = false;
//    if (success && calib.calibrate(filename,isCamera)) {
//        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
//        result->Set(NanNew<String>("renderer"), renderer);
//    } else {
//        success = false;
//    }
//
//    result->Set(NanNew<String>("success"), NanNew<Boolean>(success));
//    NanReturnValue(result);
//}
//
//NAN_METHOD(CalibrateFromPoints) {
//    NanScope();
//    Calibration calib;
//    vector<float> imagePoints, objectPoints;
//
//    if (args[0]->IsObject()) {
//        Local<Object> obj = Local<Object>::Cast(args[0]);
//        if (obj->Has(NanNew<String>("frameSize"))) {
//            std::vector<int> vect = getArray<int>(obj, "frameSize");
//            cv::Size frameSize = cv::Size(vect[0], vect[1]);
//            calib.setFrameSize(frameSize);
//        }
//        if (obj->Has(NanNew<String>("nearPlane"))) {
//            int nearPlane = getNumber<int>(obj, "nearPlane");
//            calib.setNearPlane(nearPlane);
//        }
//        if(obj->Has(NanNew<String>("farPlane"))) {
//            int farPlane = getNumber<int>(obj, "farPlane");
//            calib.setFarPlane(farPlane);
//        }
//        if (obj->Has(NanNew<String>("intrinsic"))) {
//            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
//            calib.setIntrinsicMatrix(intrinsic);
//        }
//        if (obj->Has(NanNew<String>("distortion"))) {
//            vector<float> distortion = getArray<float>(obj, "distortion");
//            calib.setDistortionMatrix(distortion);
//        }
//        if (obj->Has(NanNew<String>("imagePoints"))) {
//            imagePoints = getArray<float>(obj,"imagePoints");
//        }
//        if (obj->Has(NanNew<String>("objectPoints"))) {
//            objectPoints = getArray<float>(obj,"objectPoints");
//        }
//    }
//
//    Local<Object> result;
//    bool success = true;
//    bool isCamera = true;
//    if (calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
//        Handle<Object> camera = getCalibrationResult(calib,isCamera);
//        result->Set(NanNew<String>("camera"), camera);
//    } else {
//        success = false;
//    }
//
//    isCamera = false;
//    if (success && calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
//        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
//        result->Set(NanNew<String>("renderer"), renderer);
//    } else {
//        success = false;
//    }
//
//    result->Set(NanNew<String>("success"), NanNew<Boolean>(success));
//    NanReturnValue(result);
//}

template<class T>
std::vector<T> getArray(const Local<Object> &obj, const std::string &name) {
    std::vector<T> vect;
    //Local<Value> val = obj->Get(String::Cast(name.c_str()));
    Local<Value> val = obj->Get(String::NewFromUtf8(isolate, name.c_str()));
    Local<Array> array = Local<Array>::Cast(val);
    for (unsigned int i=0; i<array->Length(); ++i) {
        Local<Number> n = Local<Number>::Cast(array->Get(i));
        vect.push_back(n->Value());
    }
    return vect;
}

template<class T>
T getNumber(const Local<Object> &obj, const std::string &name) {
    //Local<Value> val = obj->Get(NanNew<String>(name.c_str()));
    Local<Value> val = obj->Get(String::NewFromUtf8(isolate, name.c_str()));
    T number = Local<Number>::Cast(val)->Value();
    return number;
}

std::string getString(const Local<Object> &obj, const std::string &name) {
    //Local<Value> val = obj->Get(NanNew<String>(name.c_str()));
    Local<Value> val = obj->Get(String::NewFromUtf8(isolate, name.c_str()));
    std::string s = *(String::Utf8Value(isolate, val));
    return s;
}

Local<Object> getCalibrationResult(Calibration &calib, bool isCamera) {
    // projection matrix
    vector<float> projVect = calib.getProjectionMatrix(isCamera);
    Local<Array> projection = Array::New(isolate, projVect.size());
    for (int i=0; i<(int)projVect.size(); ++i) {
        projection->Set(i, Number::New(isolate, projVect[i]));
    }
    // modelview matrix
    vector<float> mvVect = calib.getMvMatrix(isCamera);
    Local<Array> mv = Array::New(isolate, mvVect.size());
    for (int i=0; i<(int)mvVect.size(); i++) {
        mv->Set(i, Number::New(isolate, mvVect[i]));
    }
    // create result object
    Local<Object> result = Object::New(isolate);
    result->Set(String::NewFromUtf8(isolate, "projection"), projection);
    result->Set(String::NewFromUtf8(isolate, "modelview"), mv);

    return result;
}

void getCalibrationPoints(const FunctionCallbackInfo<Value>& args, vector<float>& imagePoints, vector<float>& objectPoints) {
    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        Local<Value> imageStr = String::NewFromUtf8(isolate, "imagePoints");
        Local<Value> objectStr = String::NewFromUtf8(isolate, "objectPoints");
        if (obj->Has(context, imageStr).FromMaybe(false)) {
            imagePoints = getArray<float>(obj, "imagePoints");
        }
        if (obj->Has(context, objectStr).FromMaybe(false)) {
            objectPoints = getArray<float>(obj, "objectPoints");
        }
    }
}

void configureCalibration(const FunctionCallbackInfo<Value>& args, Calibration &calib, string &filename) {
    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        Local<String> frameSize = String::NewFromUtf8(isolate, "frameSize");
        Local<String> nearPlane = String::NewFromUtf8(isolate, "nearPlane");
        Local<String> farPlane = String::NewFromUtf8(isolate, "farPlane");
        Local<String> chessboardSize = String::NewFromUtf8(isolate, "chessboardSize");
        Local<String> chessboardCellSize = String::NewFromUtf8(isolate, "chessboardCellSize");
        Local<String> intrinsic = String::NewFromUtf8(isolate, "intrinsic");
        Local<String> distortion = String::NewFromUtf8(isolate, "distortion");
        Local<String> filenameMember = String::NewFromUtf8(isolate, "filename");
        if (obj->Has(context, frameSize).FromMaybe(false)) {
            std::vector<int> vect = getArray<int>(obj, "frameSize");
            cv::Size frameSize = cv::Size(vect[0], vect[1]);
            calib.setFrameSize(frameSize);
        }
        if (obj->Has(context, nearPlane).FromMaybe(false)) {
            int nearPlane = getNumber<int>(obj, "nearPlane");
            calib.setNearPlane(nearPlane);
        }
        if(obj->Has(context, farPlane).FromMaybe(false)) {
            int farPlane = getNumber<int>(obj, "farPlane");
            calib.setFarPlane(farPlane);
        }
        if(obj->Has(context, chessboardSize).FromMaybe(false)) {
            std::vector<int> vect = getArray<int>(obj, "chessboardSize");
            cv::Size chessboardSize = cv::Size(vect[0], vect[1]);
            calib.setChessboardSize(chessboardSize);
        }
        if (obj->Has(context, chessboardCellSize).FromMaybe(false)) {
            int cellSize = getNumber<int>(obj, "chessboardCellSize");
            calib.setChessboardCellSize(cellSize);
        }
        if (obj->Has(context, intrinsic).FromMaybe(false)) {
            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
            calib.setIntrinsicMatrix(intrinsic);
        }
        if (obj->Has(context, distortion).FromMaybe(false)) {
            vector<float> distortion = getArray<float>(obj, "distortion");
            calib.setDistortionMatrix(distortion);
        }
        if (obj->Has(context, filenameMember).FromMaybe(false)) {
            filename = getString(obj, "filename");
        }
    }
}


void Calibrate(const FunctionCallbackInfo<Value>& args) {
    isolate = args.GetIsolate();
    context = Context::New(isolate);
    Calibration calib;
    string filename = "";
    configureCalibration(args, calib, filename);

    Local<Object> result = Object::New(isolate);
    bool success = true;
    bool isCamera = true;
    if (calib.calibrate(filename,isCamera)) {
        Local<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(Local<String>(String::NewFromUtf8(isolate, "camera")), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrate(filename,isCamera)) {
        Local<Object> renderer = getCalibrationResult(calib, isCamera);
        result->Set(String::NewFromUtf8(isolate,"renderer"), renderer);
    } else {
        success = false;
    }

    Local<Boolean> successValue = Boolean::New(isolate,success);
    result->Set(String::NewFromUtf8(isolate,"success"), successValue);
    args.GetReturnValue().Set(result);
}


void CalibrateFromPoints(const FunctionCallbackInfo<Value>& args) {
    isolate = args.GetIsolate();
    context = Context::New(isolate);
    Calibration calib;
    string filename = "";
    configureCalibration(args, calib, filename);

    vector<float> imagePoints, objectPoints;
    getCalibrationPoints(args, imagePoints, objectPoints);


    Local<Object> result = Object::New(isolate);
    bool success = true;
    bool isCamera = true;
    if (calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Local<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(String::NewFromUtf8(isolate,"camera"), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Local<Object> renderer = getCalibrationResult(calib,isCamera);
        result->Set(String::NewFromUtf8(isolate, "renderer"), renderer);
    } else {
        success = false;
    }

    result->Set(String::NewFromUtf8(isolate,"success"), Boolean::New(isolate, success));
    args.GetReturnValue().Set(result);
}

void init(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "calibrate", Calibrate);
    NODE_SET_METHOD(exports, "calibrateFromPoints", CalibrateFromPoints);
}

NODE_MODULE(tinkercalib, init)
