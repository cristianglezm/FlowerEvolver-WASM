#!/bin/bash

set -e

NPM_TOKEN=$1
GH_TOKEN=$2

cd dist
echo "@cristianglezm:registry:https://registry.npmjs.org" > .npmrc
echo "//registry.npmjs.org/:_authToken=${NPM_TOKEN}" >> .npmrc
npm publish --access public
sleep 10s
echo "@cristianglezm:registry:https://npm.pkg.github.com" > .npmrc
echo "//npm.pkg.github.com/:_authToken=${GH_TOKEN}" >> .npmrc
npm publish --access public
echo "--" > .npmrc
cd ..
