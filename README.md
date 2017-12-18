
## Julian Easly's CS 633 Final Project

## To Compile:

(on Unix-like system)
mkdir build; cd build; cmake ..; make

(on MS Windows)
same but cmake is not installed by default so you will have to download it here
https://cmake.org/download/

## To Run:

Type "./triangulate -m monotone -s 10 ../polygons/simple1.poly"

This command triangulates "simple1.poly" and output an SVG file called "monotone_simple1.svg"

The flag "-s" scales the polygon 10 times so it is easier to see.

The flag "-m" determines which method you will use to triangulate.

There are four options: 

1. monotone
2. earclip_fan
3. earclip_onion
4. optimal

## Input File Format:

The format of the poly file can be found in PLY_FORMAT.txt

If you are interested in getting more polygons, here is a link
http://masc.cs.gmu.edu/wiki/uploads/Dude2D/polydata.zip