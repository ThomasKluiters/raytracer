# raytracer

## To Do

These tasks have not yet been started:
* Implement openGL debug ()
* Implement depth of field
* Implement motion blur
* Implement gaussian blur
* Implement motion picture

## In Progress

These tasks are in progress:
* SIMD ray-triangle intersection (G & C)
* Implement distributed raytracing (G & C)
* Implement refraction (R)
* Implement reflection (R)
* Implement textures (E)
* Implement openGL debug (M)

## Done

These tasks are done:

* Implement soft shadows
* Implement shadows
* Ray-triangle insersection based on Plücker coordinates  (G & C)
* Simple AABB acceleration structure (L & T)
* Implement advanced acceleration structure (L & T)
* Multi-threading of the main raytracing loop (G & C)
* Multi-threading of pre computing often used triangle related values (G & C)
* Implement simple ray tracing (G, C & R)

# How to run

Client application:
```
application [Headless Mode, t for true, f for false] [Server IP] [Server Port]
```
_Windows users: If the application does not compile, add wsock32.lib as an additional dependency: Debug > Raytracing Properties > Linker > Input > Additional Dependencies, add wsock32.lib to the list (full path not required)._

Server application:
```
application <Server Port> <Number of Clients> <Number of blocks in X direction> <Number of blocks in Y direction> <Width> <Height>
```
