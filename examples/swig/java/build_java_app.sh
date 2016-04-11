#!/bin/bash

# Java app
javac *.java
java -Djava.library.path=`pwd` app

exit 0
