FunnelVision
============
A rendering technique to simulate portal-like visual effects, built on the OpenGL and VTK frameworks.
Continuation of my final project from CIS 441 "Intro Comp. Graphics" w/ Prof. Hank Childs.

Masado Ishii  
v0.1 2016-12-26  
v0.2 2017-08-05

----


Building and Running:
---------------------
The FunnelVision application depends on  

* VTK (>= v6.3.0)
* OpenGL (>= v3.0)
* the GLM headers (>= v0.9.7.2)

Other versions may work but are not guaranteed to work.

To build from source using CMake, run the following from the project root directory:

    funnel-vision$ mkdir build
    funnel-vision$ cd build
    funnel-vision$ cmake ..

Then to run, type `./funnelvision` .


Attributions:
-------------
* Interactor and mapper derived from examples by Hank Childs.
* glm usage derived from the glm docs at <glm.g-truc.net>.
* Other source code used as provided by Hank Childs, presumably derivative
    of Kitware coders K.Martin, W.Schroeder, and B.Lorensen.
