roller coaster

by XU WU


keyboard callback:   ’s’ : automatically save 300 images

Use the “splines/rollerCoaster.sp” to generate the frames.

Level completed approaches:

Level 1(spline) : I firstly used the brute force method with step (u value)increase by 0.001, for every spline , for control points, get the control matrix and caculate each point and then connect to next point with increased u. 

Level 2(ground) : read the texture pics and then assign texture names to them for ground and future sky and trail use. Then load the ground texture with its texture name.

Level 3(sky) : At the beginning of the code, initial a constant variable CUBE_SIZE, then create a sky cube use the textures loaded in level 2, the dimension is from -100 to 100 in each axis(x, y and z). 

Level 4(ride): At the beginning of the code, initial some variables for the ride processing. For every spline, for every control point, using current u vaule to caculate the current position, tangent vector, normal and binormal vectors and normalize them. Then use these unit vectors to decide current eye position, center position, camera up postion. At last, increase u value by 0.06（speed related value), recaculate the position and vectors......skip to next control points...skip to next spline(if exists)
The up vector I use is the normal vector of each point. Add the eye position, I use the current position(caculated by u) to add 0.03*(unit normal vector), which can make the view direction better!

Level 5(rail cross-section): According to result of level 1, for every consecutive points p0 and p1, I assume two width and one height, then use the p0 and p1's tangent, normal, binormal vectors to caculate v0,v1.....,v7 and k0,k1.....,k7 as left and right rails' vertices. Then use these vertices to draw two retangle cube as left and right rail. Increase u and draw next cubes just like the level's loop.


