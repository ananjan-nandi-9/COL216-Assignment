#!/bin/bash

	dos2unix *.txt
	g++ -o A5 A5.cpp
        ./A5 "$@" > output.txt # N M <ROW> <COLUMN> <files>
        cat output.txt
