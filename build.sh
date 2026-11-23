#!/bin/sh
set -e
# Builds the addon (node-gyp via npm install) and runs the tests.
npm install
npm test
