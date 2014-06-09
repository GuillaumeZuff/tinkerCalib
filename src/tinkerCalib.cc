#include <node.h>
#include <iostream>
#include "calibration.h"

using namespace v8;
using namespace std;

template<class T>
std::vector<T> getArray(const Local<Object> &obj, const std::string &name) {
    std::vector<T> vect;
    Local<Value> val = obj->Get(String::New(name.c_str()));
    Local<Array> array = Local<Array>::Cast(val);
    for (unsigned int i=0; i<array->Length(); ++i) {
        Local<Number> n = Local<Number>::Cast(array->Get(i));
        vect.push_back(n->Value());
    }
    return vect;
}

template<class T>
T getNumber(const Local<Object> &obj, const std::string &name) {
    Local<Value> val = obj->Get(String::New(name.c_str()));
    T number = Local<Number>::Cast(val)->NumberValue();
    return number;
}

std::string getString(const Local<Object> &obj, const std::string &name) {
    Local<Value> val = obj->Get(String::New(name.c_str()));
    String::Utf8Value str(val->ToString());
    return string(*str);
}

Handle<Object> getCalibrationResult(Calibration &calib, bool isCamera) {
    HandleScope scope;
    // projection matrix
    vector<float> projVect = calib.getProjectionMatrix(isCamera);
    Local<Array> projection = Array::New(projVect.size());
    for (int i=0; i<(int)projVect.size(); ++i) {
        projection->Set(i, Number::New(projVect[i]));
    }
    // modelview matrix
    vector<float> mvVect = calib.getMvMatrix(isCamera);
    Local<Array> mv = Array::New(mvVect.size());
    for (int i=0; i<(int)mvVect.size(); i++) {
        mv->Set(i, Number::New(mvVect[i]));
    }
    // create result object
    Local<Object> result = Object::New();
    result->Set(String::New("projection"), projection);
    result->Set(String::New("modelview"), mv);

    return scope.Close(result);
}

Handle<Value> Calibrate(const Arguments &args) {
    HandleScope scope;
    Calibration calib;
    string filename = "";

    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        if (obj->Has(String::New("frameSize"))) {
            std::vector<int> vect = getArray<int>(obj, "frameSize");
            cv::Size frameSize = cv::Size(vect[0], vect[1]);
            calib.setFrameSize(frameSize);
        }
        if (obj->Has(String::New("nearPlane"))) {
            int nearPlane = getNumber<int>(obj, "nearPlane");
            calib.setNearPlane(nearPlane);
        }
        if(obj->Has(String::New("farPlane"))) {
            int farPlane = getNumber<int>(obj, "farPlane");
            calib.setFarPlane(farPlane);
        }
        if(obj->Has(String::New("chessboardSize"))) {
            std::vector<int> vect = getArray<int>(obj, "chessboardSize");
            cv::Size chessboardSize = cv::Size(vect[0], vect[1]);
            calib.setChessboardSize(chessboardSize);
        }
        if (obj->Has(String::New("chessboardCellSize"))) {
            int cellSize = getNumber<int>(obj, "chessboardCellSize");
            calib.setChessboardCellSize(cellSize);
        }
        if (obj->Has(String::New("intrinsic"))) {
            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
            calib.setIntrinsicMatrix(intrinsic);
        }
        if (obj->Has(String::New("distortion"))) {
            vector<float> distortion = getArray<float>(obj, "distortion");
            calib.setDistortionMatrix(distortion);
        }
        if (obj->Has(String::New("filename"))) {
            filename = getString(obj, "filename");
        }
    }

    Local<Object> result = Object::New();
    bool success = true;
    bool isCamera = true;
    if (calib.calibrate(filename,isCamera)) {
        Handle<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(String::New("camera"), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrate(filename,isCamera)) {
        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
        result->Set(String::New("renderer"), renderer);
    } else {
        success = false;
    }

    result->Set(String::New("success"), Boolean::New(success));
    return scope.Close(result);
}

Handle<Value> CalibrateFromPoints(const Arguments &args) {
    HandleScope scope;
    Calibration calib;
    vector<float> imagePoints, objectPoints;

    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        if (obj->Has(String::New("frameSize"))) {
            std::vector<int> vect = getArray<int>(obj, "frameSize");
            cv::Size frameSize = cv::Size(vect[0], vect[1]);
            calib.setFrameSize(frameSize);
        }
        if (obj->Has(String::New("nearPlane"))) {
            int nearPlane = getNumber<int>(obj, "nearPlane");
            calib.setNearPlane(nearPlane);
        }
        if(obj->Has(String::New("farPlane"))) {
            int farPlane = getNumber<int>(obj, "farPlane");
            calib.setFarPlane(farPlane);
        }
        if (obj->Has(String::New("intrinsic"))) {
            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
            calib.setIntrinsicMatrix(intrinsic);
        }
        if (obj->Has(String::New("distortion"))) {
            vector<float> distortion = getArray<float>(obj, "distortion");
            calib.setDistortionMatrix(distortion);
        }
        if (obj->Has(String::New("imagePoints"))) {
            imagePoints = getArray<float>(obj,"imagePoints");
        }
        if (obj->Has(String::New("objectPoints"))) {
            objectPoints = getArray<float>(obj,"objectPoints");
        }
    }

    Local<Object> result = Object::New();
    bool success = true;
    bool isCamera = true;
    if (calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Handle<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(String::New("camera"), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
        result->Set(String::New("renderer"), renderer);
    } else {
        success = false;
    }

    result->Set(String::New("success"), Boolean::New(success));
    return scope.Close(result);
}

void Init(Handle<Object> exports)
{
	exports->Set(String::NewSymbol("calibrate"), FunctionTemplate::New(Calibrate)->GetFunction());
	exports->Set(String::NewSymbol("calibrateFromPoints"), FunctionTemplate::New(CalibrateFromPoints)->GetFunction());

}

NODE_MODULE(simpliCalib, Init)
