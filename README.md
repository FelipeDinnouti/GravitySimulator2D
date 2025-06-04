## Building the project
Preferably run it on linux (works best)

In the root directory run this command to create and setup the build directory (only need to run it once)
```bash
cmake -B build
```

And everytie time you need to rebuild: 
```bash
cmake --build build
```
And it creates an executable on the `/build` folder.

## Using the project
Move the mouse around when locked to move the camera.

H - Toggles control help menu 
F - Toggle mouse lock
D - (hold) Create and "throw" an actor
G - Fullscreen (may not work correctly in some systems)
Tab - Change the actor on which the camera focuses
S - Toggle camera focus mode.

Unlock the mouse and move the sliders to change the creating actor size and gravitational constant.
