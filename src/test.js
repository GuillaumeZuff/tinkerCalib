var addon = require('../build/Release/tinkercalib');

var result = addon.calibrate({
  nearPlane: 100,
  farPlane: 1200,
  chessboardSize: [10, 6],
  chessboardCellSize: 25,
  filename: 'testData/calib.png',
  intrinsic: [1061.64, 0.0, 449.264, 0.0, 1061.64, 361.303, 0.0, 0.0, 1.0],
  distortion: [0.00436111, 1.4013e-45, 0.0, 7.54728e-34, 2.8026e-45]
});

console.log("Success: ", result.success);
if (result.success) {
    console.log("Projection: ", result.projection);
    console.log("Modelview: ", result.modelview);
}

result = addon.calibrateFromPoints({
    frameSize: [ 640, 480 ],
    imagePoints: [
        489.51717499999995,
        59.588125,
        327.63941666666665,
        57.302975,
        165.40420833333337,
        53.272125,
        481.890575,
        162.326525,
        325.59499166666666,
        159.64954999999998,
        169.28835833333338,
        156.7088,
        475.0754,
        258.10405000000003,
        323.73726666666664,
        255.4805,
        172.2098333333334,
        253.03025
    ],
    objectPoints: [
        -125, -80, 0, -80, 125, -80, -125, 0, 0, 0, 125, 0, -125, 80, 0, 80, 125, 80
    ],
    intrinsic: [ 846.538, 0, 331.012, 0, 846.538, 228.192, 0, 0, 1 ],
    distortion: [ 0, 0, 0, 0, 0 ],
    nearPlane: 1,
    farPlane: 12000,
    chessboardSize: [ 10, 6 ],
    chessboardCellSize: 25
});

console.log("Success points: ", result.success);
if (result.success) {
    console.log("Projection: ", result.camera);
    console.log("Modelview: ", result.renderer);
}

