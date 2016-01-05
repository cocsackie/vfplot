#!/bin/sh
#
# refresh kdtree.* from git master

NAME="kdtree"
URL="https://raw.githubusercontent.com/jtsiomb/$NAME/master"

for ext in c h
do
    FILE="$NAME.$ext"
    echo -n "$FILE .."
    wget -q "$URL/$FILE" -O "$FILE"
    echo ". done"
done
