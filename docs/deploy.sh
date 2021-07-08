#!/usr/bin/env sh

# abort on errors
set -e

# build
npm run docs:build

# navigate into the build output directory
cd src/.vuepress/dist

# if you are deploying to a custom domain
# echo 'www.example.com' > CNAME

git init
git add -A
git commit -m 'deploy'

# if you are deploying to https://<USERNAME>.github.io
# git push -f git@github.com:<USERNAME>/<USERNAME>.github.io.git master

ssh -vT git@github.com

# if you are deploying to https://<USERNAME>.github.io/<REPO>
git push -f git@github.com:toor1245/cpu_features.git master:gh-pages

cd -