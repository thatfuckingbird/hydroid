#!/bin/sh

find . -iname *.h -o -iname *.cpp | xargs clang-format -style=file -i
#Need to wait for Qt 5.15.1 because qmlformat is too buggy: find . -iname *.qml | xargs qmlformat -V -i
