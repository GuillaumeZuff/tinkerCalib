#include <node.h>
#include <nan.h>
#include <iostream>
#include "calibration.h"

using namespace v8;
using namespace std;

template<class T>
std::vector<T> getArray(const Local<Object> &obj, const std::string &name) {
    std::vector<T> vect;
    Local<Value> val = obj->Get(NanNew<String>(name.c_str()));
    Local<Array> array = Local<Array>::Cast(val);
    for (unsigned int i=0; i<array->Length(); ++i) {
        Local<Number> n = Local<Number>::Cast(array->Get(i));
        vect.push_back(n->Value());
    }
    return vect;
}

template<class T>
T getNumber(const Local<Object> &obj, const std::string &name) {
    Local<Value> val = obj->Get(NanNew<String>(name.c_str()));
    T number = Local<Number>::Cast(val)->NumberValue();
    return number;
}

std::string getString(const Local<Object> &obj, const std::string &name) {
    Local<Value> val = obj->Get(NanNew<String>(name.c_str()));
    String::Utf8Value str(val->ToString());
    return string(*str);
}

Handle<Object> getCalibrationResult(Calibration &calib, bool isCamera) {
    NanEscapableScope();
    // projection matrix
    vector<float> projVect = calib.getProjectionMatrix(isCamera);
    Local<Array> projection = NanNew<Array>(projVect.size());
    for (int i=0; i<(int)projVect.size(); ++i) {
        projection->Set(i, NanNew<Number>(projVect[i]));
    }
    // modelview matrix
    vector<float> mvVect = calib.getMvMatrix(isCamera);
    Local<Array> mv = NanNew<Array>(mvVect.size());
    for (int i=0; i<(int)mvVect.size(); i++) {
        mv->Set(i, NanNew<Number>(mvVect[i]));
    }
    // create result object
    Local<Object> result;
    result->Set(NanNew<String>("projection"), projection);
    result->Set(NanNew<String>("modelview"), mv);

    return NanEscapeScope(result);
}

NAN_METHOD(Calibrate) {
    NanScope();
    Calibration calib;
    string filename = "";

    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        if (obj->Has(NanNew<String>("frameSize"))) {
            std::vector<int> vect = getArray<int>(obj, "frameSize");
            cv::Size frameSize = cv::Size(vect[0], vect[1]);
            calib.setFrameSize(frameSize);
        }
        if (obj->Has(NanNew<String>("nearPlane"))) {
            int nearPlane = getNumber<int>(obj, "nearPlane");
            calib.setNearPlane(nearPlane);
        }
        if(obj->Has(NanNew<String>("farPlane"))) {
            int farPlane = getNumber<int>(obj, "farPlane");
            calib.setFarPlane(farPlane);
        }
        if(obj->Has(NanNew<String>("chessboardSize"))) {
            std::vector<int> vect = getArray<int>(obj, "chessboardSize");
            cv::Size chessboardSize = cv::Size(vect[0], vect[1]);
            calib.setChessboardSize(chessboardSize);
        }
        if (obj->Has(NanNew<String>("chessboardCellSize"))) {
            int cellSize = getNumber<int>(obj, "chessboardCellSize");
            calib.setChessboardCellSize(cellSize);
        }
        if (obj->Has(NanNew<String>("intrinsic"))) {
            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
            calib.setIntrinsicMatrix(intrinsic);
        }
        if (obj->Has(NanNew<String>("distortion"))) {
            vector<float> distortion = getArray<float>(obj, "distortion");
            calib.setDistortionMatrix(distortion);
        }
        if (obj->Has(NanNew<String>("filename"))) {
            filename = getString(obj, "filename");
        }
    }

    Local<Object> result;
    bool success = true;
    bool isCamera = true;
    if (calib.calibrate(filename,isCamera)) {
        Handle<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(NanNew<String>("camera"), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrate(filename,isCamera)) {
        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
        result->Set(NanNew<String>("renderer"), renderer);
    } else {
        success = false;
    }

    result->Set(NanNew<String>("success"), NanNew<Boolean>(success));
    NanReturnValue(result);
}

NAN_METHOD(CalibrateFromPoints) {
    NanScope();
    Calibration calib;
    vector<float> imagePoints, objectPoints;

    if (args[0]->IsObject()) {
        Local<Object> obj = Local<Object>::Cast(args[0]);
        if (obj->Has(NanNew<String>("frameSize"))) {
            std::vector<int> vect = getArray<int>(obj, "frameSize");
            cv::Size frameSize = cv::Size(vect[0], vect[1]);
            calib.setFrameSize(frameSize);
        }
        if (obj->Has(NanNew<String>("nearPlane"))) {
            int nearPlane = getNumber<int>(obj, "nearPlane");
            calib.setNearPlane(nearPlane);
        }
        if(obj->Has(NanNew<String>("farPlane"))) {
            int farPlane = getNumber<int>(obj, "farPlane");
            calib.setFarPlane(farPlane);
        }
        if (obj->Has(NanNew<String>("intrinsic"))) {
            vector<float> intrinsic = getArray<float>(obj, "intrinsic");
            calib.setIntrinsicMatrix(intrinsic);
        }
        if (obj->Has(NanNew<String>("distortion"))) {
            vector<float> distortion = getArray<float>(obj, "distortion");
            calib.setDistortionMatrix(distortion);
        }
        if (obj->Has(NanNew<String>("imagePoints"))) {
            imagePoints = getArray<float>(obj,"imagePoints");
        }
        if (obj->Has(NanNew<String>("objectPoints"))) {
            objectPoints = getArray<float>(obj,"objectPoints");
        }
    }

    Local<Object> result;
    bool success = true;
    bool isCamera = true;
    if (calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Handle<Object> camera = getCalibrationResult(calib,isCamera);
        result->Set(NanNew<String>("camera"), camera);
    } else {
        success = false;
    }

    isCamera = false;
    if (success && calib.calibrateFromPoints(imagePoints,objectPoints,isCamera)) {
        Handle<Object> renderer = getCalibrationResult(calib,isCamera);
        result->Set(NanNew<String>("renderer"), renderer);
    } else {
        success = false;
    }

    result->Set(NanNew<String>("success"), NanNew<Boolean>(success));
    NanReturnValue(result);
}

void Init(Handle<Object> exports)
{
	exports->Set(
            NanNew<String>("calibrate"),
            NanNew<FunctionTemplate>(Calibrate)->GetFunction()
        );
	exports->Set(
            NanNew<String>("calibrateFromPoints"), 
            NanNew<FunctionTemplate>(CalibrateFromPoints)->GetFunction()
        );

}

NODE_MODULE(tinkercalib, Init)
