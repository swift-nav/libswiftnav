#!/usr/bin/env bash
if [ "$TRAVIS_PULL_REQUEST" == "false" ] ; then
  # Not in a pull request, so compare against parent commit
  base_commit="HEAD^"
  echo "Running clang-format against parent commit $(git rev-parse $base_commit)"
else
  git remote set-branches origin $TRAVIS_BRANCH && git fetch
  base_commit="origin/$TRAVIS_BRANCH"
  echo "Running clang-format against branch $base_commit, with hash $(git rev-parse $base_commit)"
fi
output="$(git-clang-format-4.0 --binary clang-format-4.0 --commit $base_commit --diff)"
if [ "$output" == "no modified files to format" ] || [ "$output" == "clang-format did not modify any files" ] ; then
  echo "clang-format passed."
  exit 0
else
  echo "clang-format failed:"
  echo "$output"
  exit 1
fi
