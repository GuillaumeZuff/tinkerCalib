// Prints OpenCV build flags for binding.gyp, via pkg-config.
//   node utils/find-opencv.js --include   include dirs (space separated)
//   node utils/find-opencv.js --libs      linker flags
// Tries opencv4, then opencv5, then the legacy "opencv" package name.
const { execFileSync } = require('child_process');

const mode = process.argv[2];
const names = ['opencv4', 'opencv5', 'opencv'];
const pkg = names.find((name) => {
    try {
        execFileSync('pkg-config', ['--exists', name]);
        return true;
    } catch (e) {
        return false;
    }
});
if (!pkg) {
    console.error('tinkercalib: OpenCV not found with pkg-config (tried ' + names.join(', ') +
        '). Install it, e.g. "sudo apt-get install libopencv-dev" or "brew install opencv".');
    process.exit(1);
}
const run = (flag) => execFileSync('pkg-config', [flag, pkg], { encoding: 'utf8' }).trim();
if (mode === '--include') {
    console.log(run('--cflags-only-I').split(/\s+/).filter(Boolean).map((f) => f.replace(/^-I/, '')).join(' '));
} else if (mode === '--libs') {
    console.log(run('--libs'));
} else {
    console.error('usage: node utils/find-opencv.js --include|--libs');
    process.exit(1);
}
