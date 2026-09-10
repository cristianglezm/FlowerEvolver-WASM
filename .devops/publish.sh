#!/bin/bash

set -euo pipefail

GH_TOKEN=$1

cd dist

echo "@cristianglezm:registry=https://registry.npmjs.org" > .npmrc
npm publish --access public

sleep 10s

echo "@cristianglezm:registry=https://npm.pkg.github.com" > .npmrc
echo "//npm.pkg.github.com/:_authToken=\${GH_TOKEN}" >> .npmrc
npm publish --access public

cd ..
