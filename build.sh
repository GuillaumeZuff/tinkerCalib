#!/bin/sh

node-gyp configure
node-gyp build
#cp -R ../simpliCalib ../../meteor/.meteor/local/build/programs/server/node_modules/

echo "done."
