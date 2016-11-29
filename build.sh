#!/bin/sh

node-gyp configure
node-gyp build

node src/test.js
#cp -R ../simpliCalib ../../meteor/.meteor/local/build/programs/server/node_modules/

echo "done."
