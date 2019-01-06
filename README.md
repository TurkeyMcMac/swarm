# Swarm
This is something I made when testing my "Just Wheels Bouncing" library. I know
the code is messy and all, but it looks cool.

## Behavior
The circles accelerate toward each other upon touching, the same amount
regardless of mass. There is no conservation of energy or momentum. A small
force of friction acts on all circles. The universe is toroidal.

On the large scale, circles form dense clumps which build up rotational speed.
These clumps are shortly ripped apart by their own spinning.

## Compiling
You need SDL2 and my JWB library to run this. You will have to manually put a
shared library called "libjwb.so" into the root directory before `make`ing. I
have only compiled this on Ubuntu.
