# Personal-Graphics-Framework
## Overview
+ Implemented Features:
  + Deferred Shading
  + BRDF-Phong Lighting
  + Shadow Map
  + Spatial Data Structure
    + Octree
    + BSP-tree
  + Collision Dectection
    + GJK Algorithm
  + Animation & Modeling
    + Quaternion
    + Skeletal Animation
    + Interpolating Spline
    + Motion along a path
  + Inverse Kinematic
    + Cyclic Descent Coordinate (CCD)
    + Constraints
  + Physic simulation
    + Spring-Mass-Damper System
    + Runge-kutta 4th order
  + UI
    + ImGui
    + ImPlot
## How to use framework
+ There are two types of camera:
  + First type: holding left click to rotate camera around center, holding right click to move camera in XY-plane
  + Second type: using WASD + holding left click to move around
  + Use "TAB" key to switch between two types
+ There are two types of animation:
  + Animation1: walking
  + Animation2: running
+ Space curve graph that allows user to edit the space curve at run-time:
  + Use mouse scroll to zoom in/out
  + Hover any control points, hold left-click and drag the point to desired location
  + Space curve will be updated at run-time reflects the changes in the graph
+ End Effector can be moved by using ARROW KEYS