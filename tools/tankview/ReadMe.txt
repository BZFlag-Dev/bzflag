========================================================================
    tankview Project Overview
========================================================================

tankview is a very simple app. It will show you, graphicaly what a tank
model will look like. Simply build it, drop it into a folder with a data
dir, and run. The applicaiton will then load the 5 obj files from the
data dir for the standard high res tank;

 * data/geometry/tank/std/body.obj
 * data/geometry/tank/std/turret.obj
 * data/geometry/tank/std/barrel.obj
 * data/geometry/tank/std/rcasing.obj
 * data/geometry/tank/std/lcasing.obj

These 5 models will be displaed in the same postions that they would be
in game, and they will be textured with the standard tank texture. This
can let you see how your model will look in game.

you can move the view arround by tapping the arrow keys. Use alt and ctl
to do various pan and rotate movements.

The gray grid is the ground. The 3 colored lines in the middle is the
tank's origin. The light blue box is the bounding box of the standard
tank.

*TODO*
 * Keys to change the team texture
 * code to draw the standard tank parts if an obj is missing
 * option to spin the tank.
 * linux build system

*dependencies

all the cpp and h files in /tools/tankview/
/src/3d/Model.cxx
/include/Model.h
/src/common/TextUtils.h
/include/TextUtils.h

sdl, and sdl_image
openGL, and glu



