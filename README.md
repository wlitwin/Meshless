Description
===================

Straight forward implementation of the paper "Meshless Deformation Based on Shape Matching" 

http://dl.acm.org/citation.cfm?id=1073216

Building and Usage
==================

####Dependencies

This code has the following dependencies (other versions may work):

- [dlib](http://dlib.net/) 18.5
- [GLM](http://glm.g-truc.net/0.9.5/index.html) 0.9.5
- [GLFW](http://www.glfw.org/) 2.7
- [GLEW](http://glew.sourceforge.net/) Supporting OpenGL 3.3
- OpenGL 3.3

####Building

To build run `make` which should produce an executable `meshless`.

To build with slow motion turned on run `make slowmo`.

####Usage

Run `./meshless [obj_file]`. The OBJ file is optional and will run with the `sphere.obj` by default.

When the program is running `h` will print the controls to the console.

Performance is surprisingly good, running 100,000+ particles on an older system. Although, larger numbers of particles may require the `SIM_DT` to be changed in `src/main.cpp`.

**Note about regular simulation:** With high beta and low alpha values and large forces the mesh may turn inside out. To correct inversion throw the mesh again softer, this is a side effect of how the particle system is implemented.

**Note about slow motion:** The paper suggests a fix for variable time steps that can make the simulation more unstable, so try to avoid a combination of high beta and low alpha values.

Visuals
=======

Models created using Blender. I suggest loading more exciting models than these.

Slow motion enabled beta = 0.8, alpha = 0.2, cube.
[Example 1](https://vimeo.com/85008766)

Slow motion enabled beta = 0.8, alpha = 0.2, sphere.
[Example 2](https://vimeo.com/85009231)
