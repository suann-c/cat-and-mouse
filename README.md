# Game Information
(Note: fill in this portion with information about your game.)

Title: Cat and Mouse

Author: Suann Chi (suannc)

Design Document: Cat and Mouse	URL: http://graphics.cs.cmu.edu/courses/15-466-f18/game2-designs/suannc/

Screen Shot:

![Screen Shot](screenshot.png)

How To Play:

The game starts off with Tom having a random amount of objects to throw down (control by mouse click) at Jerry. Tom can choose where to drop the objects. 
The object will disappear once it hits the floor or it hits Jerry. Jerry attempts to survive by going left or right (control by left and right arrow key) to 
dodge the object coming down at him. Tom wins when he hits Jerry. Jerry wins if he manages to dodge all of Tom's objects.

Changes From The Design Document:

The orignal design has Tom throwing various shape of objects to Jerry, I simplified the thrown object to just one shape.
I spent most of my time trying to figure out server client connections. 

Good / Bad / Ugly Code:

Good: Logical separation of what each client does and what the server does. Unfortunately the server was not able to actually receive data from client and I ran out of time to debug it. Finished writing the logic for win/loss conditions in game and in theory it should work if the server was able to receive info.
Bad: I had the jerry client send to the server if jerry was still in the game, someone could easily change the source code to cheat. 


# Using This Base Code

Before you dive into the code, it helps to understand the overall structure of this repository.
- Files you should read and/or edit:
    - ```main.cpp``` creates the game window and contains the main loop. You should read through this file to understand what it's doing, but you shouldn't need to change things (other than window title, size, and maybe the initial Mode).
    - ```server.cpp``` creates a basic server.
    - ```GameMode.*pp``` declaration+definition for the GameMode, a basic scene-based game mode.
    - ```meshes/export-meshes.py``` exports meshes from a .blend file into a format usable by our game runtime.
    - ```meshes/export-walkmeshes.py``` exports meshes from a given layer of a .blend file into a format usable by the WalkMeshes loading code.
    - ```meshes/export-scene.py``` exports the transform hierarchy of a blender scene to a file.
	- ```Connection.*pp``` networking code.
    - ```Jamfile``` responsible for telling FTJam how to build the project. If you add any additional .cpp files or want to change the name of your runtime executable you will need to modify this.
    - ```.gitignore``` ignores the ```objs/``` directory and the generated executable file. You will need to change it if your executable name changes. (If you find yourself changing it to ignore, e.g., your editor's swap files you should probably, instead be investigating making this change in the global git configuration.)
- Files you should read the header for (and use):
	- ```Sound.*pp``` spatial sound code.
    - ```WalkMesh.*pp``` code to load and walk on walkmeshes.
    - ```MenuMode.hpp``` presents a menu with configurable choices. Can optionally display another mode in the background.
    - ```Scene.hpp``` scene graph implementation, including loading code.
    - ```Mode.hpp``` base class for modes (things that recieve events and draw).
    - ```Load.hpp``` asset loading system. Very useful for OpenGL assets.
    - ```MeshBuffer.hpp``` code to load mesh data in a variety of formats (and create vertex array objects to bind it to program attributes).
    - ```data_path.hpp``` contains a helper function that allows you to specify paths relative to the executable (instead of the current working directory). Very useful when loading assets.
    - ```draw_text.hpp``` draws text (limited to capital letters + *) to the screen.
    - ```compile_program.hpp``` compiles OpenGL shader programs.
    - ```load_save_png.hpp``` load and save PNG images.
- Files you probably don't need to read or edit:
    - ```GL.hpp``` includes OpenGL prototypes without the namespace pollution of (e.g.) SDL's OpenGL header. It makes use of ```glcorearb.h``` and ```gl_shims.*pp``` to make this happen.
    - ```make-gl-shims.py``` does what it says on the tin. Included in case you are curious. You won't need to run it.
    - ```read_chunk.hpp``` contains a function that reads a vector of structures prefixed by a magic number. It's surprising how many simple file formats you can create that only require such a function to access.

## Asset Build Instructions

The ```meshes/export-meshes.py``` script can write mesh data including a variety of attributes (e.g., *p*ositions, *n*ormals, *c*olors, *t*excoords) from a selected layer of a blend file:

```
blender --background --python meshes/export-meshes.py -- meshes/crates.blend:1 dist/crates.pnc
```

The ```meshes/export-scene.py``` script can write the transformation hierarchy of the scene from a selected layer of a blend file, and includes references to meshes (by name):

```
blender --background --python meshes/export-scene.py -- meshes/crates.blend:1 dist/crates.scene
```

The ```meshes/export-walkmeshes.py``` script can writes vertices, normals, and triangle indicies of all meshes on a selected layer of a .blend file:

```
blender --background --python meshes/export-walkmeshes.py -- meshes/crates.blend:3 dist/crates.walkmesh
```

There is a Makefile in the ```meshes``` directory with some example commands of this sort in it as well.

## Runtime Build Instructions

The runtime code has been set up to be built with [FT Jam](https://www.freetype.org/jam/).

### Getting Jam

For more information on Jam, see the [Jam Documentation](https://www.perforce.com/documentation/jam-documentation) page at Perforce, which includes both reference documentation and a getting started guide.

On unixish OSs, Jam is available from your package manager:
```
	brew install ftjam #on OSX
	apt get ftjam #on Debian-ish Linux
```

On Windows, you can get a binary [from sourceforge](https://sourceforge.net/projects/freetype/files/ftjam/2.5.2/ftjam-2.5.2-win32.zip/download),
and put it somewhere in your `%PATH%`.
(Possibly: also set the `JAM_TOOLSET` variable to `VISUALC`.)

### Libraries

This code uses the [libSDL](https://www.libsdl.org/) library to create an OpenGL context, and the [glm](https://glm.g-truc.net) library for OpenGL-friendly matrix/vector types.
On MacOS and Linux, the code should work out-of-the-box if if you have these installed through your package manager.

If you are compiling on Windows or don't want to install these libraries globally there are pre-built library packages available in the
[kit-libs-linux](https://github.com/ixchow/kit-libs-linux),
[kit-libs-osx](https://github.com/ixchow/kit-libs-osx),
and [kit-libs-win](https://github.com/ixchow/kit-libs-win) repositories.
Simply clone into a subfolder and the build should work.

### Building

Open a terminal (or ```x64 Native Tools Command Prompt for VS 2017``` on Windows), change to the directory containing this code, and type:

```
jam
```

That's it. You can use ```jam -jN``` to run ```N``` parallel jobs if you'd like; ```jam -q``` to instruct jam to quit after the first error; ```jam -dx``` to show commands being executed; or ```jam main.o``` to build a specific file (in this case, main.cpp).  ```jam -h``` will print help on additional options.
