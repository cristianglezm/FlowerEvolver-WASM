#!/bin/bash

GITUSER=$1
EMAIL=$2

git config user.name $GITUSER && git config user.email $EMAIL
git checkout --orphan gh-pages
git --work-tree dist add --all
git --work-tree dist commit -m"gh-pages"
echo "pushing to gh-pages..."
git push origin HEAD:gh-pages --force
echo "deployment done"
