#!/bin/sh

for tst in `ls *.expect`
do
  echo "Running $tst ..."
  expect -f $tst
done
