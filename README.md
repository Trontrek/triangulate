
# CS 633 Triangulation Assignment
## Due: September 28, 2017 by 11:59pm.

## How to submit: Submit all source code (no binary) in a file named

               cs633_pa1_[your last name].zip to Blackboard

### Not TODO:

You Cannot
1. use external code
2. Change the provided code (unless there is a bug)

### TODO:

In this assignment, you will implement the triangulation algorithm described
in Chapter 3 of the CGAA book. Here is a more detailed list of your tasks

1. Make sure that you understand the algorithm and the skeleton code, esp.
    classes in polygon.h

2. Implement the triangulate function in triangulation.cpp

3. intersection.h should contain some helpful functions.

4. Vertices in the polygon are ordered counter clockwise. Triangles
    created by your function should also be ordered counter clockwise.

5. There will be 20% bonus of you can make the algorithm work for
    polygon with holes.


## To compile:

(on Unix-like system)
mkdir build; cd build; cmake ..; make

(on MS Windows)
same but cmake is not installed by default so you will have to download it here
https://cmake.org/download/

## To run:

Type "triangulate -s 10 polygons/simple1.poly"

This command triangulates "simple1.poly" and output an SVG file called output.svg
The flag "-s" scales the polygon 10 times so it is easier to see.

There are several other input files in the polygons directory.

Use them to test your code.

The format of the poly file can be found in PLY_FORMAT.txt

If you are interested in getting more polygons, here is a link
http://masc.cs.gmu.edu/wiki/uploads/Dude2D/polydata.zip

## Bug report:

Let me know if there are problems/issues.

jmlien@cs.gmu.edu
