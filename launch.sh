#!/bin/sh
rm -rf *.png *.pov ./a.out 

echo "compilation";
g++-4.8 main2.cpp -std=gnu++11 -fopenmp -lGLEW -lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
echo "execution";
./a.out 

#echo "rendu";
#for i in `ls *.pov`; do
	#povray -D -V $i
#done
