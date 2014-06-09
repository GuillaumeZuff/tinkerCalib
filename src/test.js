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
