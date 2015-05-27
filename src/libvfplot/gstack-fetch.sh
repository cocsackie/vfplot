#!/bin/sh
#
# refresh gstack.* from git master

NAME=gstack
URL="https://raw.githubusercontent.com/jjgreen/$NAME/master"

for ext in c h
do
    FILE="$NAME.$ext"
    echo -n "$FILE .."
    wget -q $URL/$FILE -O $FILE
    echo ". done"
done
