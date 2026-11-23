// Run with: npm test   (needs a built addon: npm install / node-gyp rebuild)
//
// The reference values in fixtures/reference-0.2.1.json were produced by the
// original V8 binding (tinkercalib 0.2.1, Node 8, OpenCV 3.2) running on the
// old Tinkerlamp server, with exactly the input below (the same input the
// Meteor app sends from imports/api/tinker/calib/calibrations.js).
const test = require('node:test');
const assert = require('node:assert/strict');
const path = require('node:path');

const addon = require('..');
const reference = require('./fixtures/reference-0.2.1.json');

const pointsInput = () => ({
    frameSize: [640, 480],
    imagePoints: [
        489.51717499999995, 59.588125,
        327.63941666666665, 57.302975,
        165.40420833333337, 53.272125,
        481.890575, 162.326525,
        325.59499166666666, 159.64954999999998,
        169.28835833333338, 156.7088,
        475.0754, 258.10405000000003,
        323.73726666666664, 255.4805,
        172.2098333333334, 253.03025,
    ],
    objectPoints: [-125, -80, 0, -80, 125, -80, -125, 0, 0, 0, 125, 0, -125, 80, 0, 80, 125, 80],
    intrinsic: [846.538, 0, 331.012, 0, 846.538, 228.192, 0, 0, 1],
    distortion: [0, 0, 0, 0, 0],
    nearPlane: 1,
    farPlane: 12000,
    chessboardSize: [10, 6],
    chessboardCellSize: 25,
});

const fileInput = (filename) => ({
    nearPlane: 100,
    farPlane: 1200,
    chessboardSize: [10, 6],
    chessboardCellSize: 25,
    filename,
    intrinsic: [1061.64, 0.0, 449.264, 0.0, 1061.64, 361.303, 0.0, 0.0, 1.0],
    distortion: [0.00436111, 1.4013e-45, 0.0, 7.54728e-34, 2.8026e-45],
});

function assertMatrixClose(actual, expected, tolerance, label) {
    assert.ok(Array.isArray(actual), `${label} is an array`);
    assert.equal(actual.length, 16, `${label} has 16 entries`);
    actual.forEach((value, i) => {
        assert.equal(typeof value, 'number', `${label}[${i}] is a number`);
        const diff = Math.abs(value - expected[i]);
        assert.ok(diff <= tolerance,
            `${label}[${i}]: got ${value}, expected ${expected[i]} (diff ${diff} > ${tolerance})`);
    });
}

test('exports calibrate and calibrateFromPoints', () => {
    assert.equal(typeof addon.calibrate, 'function');
    assert.equal(typeof addon.calibrateFromPoints, 'function');
});

test('calibrateFromPoints returns the same shape as 0.2.1', () => {
    const result = addon.calibrateFromPoints(pointsInput());
    assert.deepEqual(Object.keys(result).sort(), ['camera', 'renderer', 'success']);
    assert.equal(result.success, true);
    for (const part of ['camera', 'renderer']) {
        assert.deepEqual(Object.keys(result[part]).sort(), ['modelview', 'projection']);
    }
});

test('calibrateFromPoints matches the 0.2.1 values', () => {
    const result = addon.calibrateFromPoints(pointsInput());
    for (const part of ['camera', 'renderer']) {
        // Projection only depends on the inputs: must match to float precision.
        assertMatrixClose(result[part].projection, reference[part].projection, 1e-6, `${part}.projection`);
        // Modelview comes from solvePnP; allow for OpenCV 3 -> 4 numeric drift:
        // rotation entries within 1e-3, translation (mm) within 0.05 mm.
        const mv = result[part].modelview;
        const ref = reference[part].modelview;
        assertMatrixClose(mv.slice(0, 12).concat([0, 0, 0, 1]), ref.slice(0, 12).concat([0, 0, 0, 1]), 1e-3, `${part}.modelview rotation`);
        assertMatrixClose(mv.slice(12).concat(new Array(12).fill(0)), ref.slice(12).concat(new Array(12).fill(0)), 0.05, `${part}.modelview translation`);
    }
});

test('calibrateFromPoints does not modify its argument', () => {
    const input = pointsInput();
    addon.calibrateFromPoints(input);
    assert.deepEqual(input, pointsInput());
});

test('calibrate returns {success: false} when the file is missing', () => {
    assert.deepEqual(addon.calibrate(fileInput('/nonexistent/calib.png')), { success: false });
});

test('calibrate on the test image returns a well-formed result', () => {
    // 0.2.1 + OpenCV 3.2 did not find the chessboard in this image ({success: false});
    // newer OpenCV versions may. Either way the shape must be the old one.
    const result = addon.calibrate(fileInput(path.join(__dirname, '..', 'testData', 'calib.png')));
    assert.equal(typeof result.success, 'boolean');
    if (result.success) {
        for (const part of ['camera', 'renderer']) {
            assert.equal(result[part].projection.length, 16);
            assert.equal(result[part].modelview.length, 16);
        }
    } else {
        assert.deepEqual(result, { success: false });
    }
});

test('invalid input throws a JS error instead of aborting the process', () => {
    // 0.2.1 crashed Node with an uncaught cv::Exception here.
    assert.throws(() => addon.calibrateFromPoints({}));
    assert.throws(() => addon.calibrateFromPoints({ imagePoints: [1, 2, 3, 4], objectPoints: [1, 2] }));
});
