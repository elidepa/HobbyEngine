# HobbyEngine

The aim of this project is to create a simple educational OpenGL-based game engine and to learn graphics and game engine programming. My focus has been on extensibility and making the source code easy to understand. The project is very much in a work-in-progress -stage and is constantly evolving.

## Usage

### Precompiled binary

The precompiled binaries for Windows can be found in this [repository's releases page](https://github.com/elidepa/HobbyEngine/releases). The zip-file includes the compiled binary as well as libraries required to run the program.

### Compiling

This repository contains project files for Visual Studio 2019, but the code should be cross-platform compatible, but this has not been tested.

#### Dependencies

The following libraries are required to compile the code:

| Library   | Version         | Download                                         |
|-----------|-----------------|--------------------------------------------------|
| FreeType  | 2.10            | https://www.freetype.org/download.html           |
| GLAD      | OpenGL 3.3 core | https://glad.dav1d.de/                           |
| GLFW      | 3.3             | https://www.glfw.org/download.html               |
| GLM       | 0.9.9           | https://github.com/g-truc/glm/tags               |
| Lua       | 5.3.5           | https://www.lua.org/download.html                |
| sol       | 3.0             | https://github.com/ThePhD/sol2/releases          |
| stb_image | 2.23            | https://github.com/nothings/stb                  |
| TinyXML-2 | 7.0             | https://github.com/leethomason/tinyxml2/releases |

If using the included VC++ solution, the project configuration should be updated to point to these libraries and header files.

## Features

### OpenGL rendering

The OpenGL-renderer supports the following features:

- Basic Phong lighting model with ambient, diffuse and specular lighting
- Environment mapping and reflection maps for different materials
- Skyboxes
- Normal mapping
- Shadow mapping
- Instanced rendering of particle systems
- Text rendering with fonts loaded by FreeType
- FPS-style camera

### Component-based game objects

The game object architecture is meant to be as easily extensible as possible. Game objects are created through a factory which makes it easy to implement and register new component types. The game objects are configured with XML, which makes it straightforward to implement new features. This also would make it easier to implement an editor for the engine.

Currently the following components are supported by the engine:

- Lua component, which enables scripting game object behaviour
- Particle system component
- Render component, which encapsulates all the data needed for the renderer to render a particular game object, including its model's vertices, material details and normal maps
- Transform component, which contains the game object's position and rotation

### Resource loading

The resource loader is also made with extensibility in mind, and it's pretty straightforward to implement loaders for new types of resources. Currently the loader supports loading fonts, images, Lua scripts, .obj models and basic text files.

## Next steps

These are some of the possible next steps for the project:

- Basic physics system
  - AABB collision detection
  - Gravity
  - Spatial partitioning of game objects
- Scene level scripting and interaction between game objects
- More robust Lua API
- Messaging system for system and game events
  - Change of game state
  - Loading new scenes
  - System level events (e.g. exit signal, change of resolution etc.)
- Performance optimizations
  - Memory usage optimizations, for example a custom memory allocator
  - Currently there is some memory leakage, which should be fixed

## Coding conventions

This list includes some conventions used throughout the source code. It can be expanded or updated at any time if need rises.

### Naming

Use camelCase for everything. Class and type names are capitalised, variables and member functions start with lowercase. Private class member variables are prefixed with a "m_". Macros should be named all caps with an underscore between words.

### Member variables

If both read _and_ write access is required from outside the class, the member should be public to indicate this. Otherwise all members should be private, read access can be provided via a getter function, which is named identically to the member excluding the "m_"-prefix.

### Pointers

Prioritize using C++ refs and C++11 smart pointers over raw pointers whenever possible.

## Sources

The following major sources have been used in coding this engine:

- https://learnopengl.com/ OpenGL tutorial by Joey de Vries
- http://www.opengl-tutorial.org/
- Nystrom, B. (2014). _Game Programming Patterns_. https://gameprogrammingpatterns.com/
- McShaffry M. and Graham D. (2012). _Game Coding Complete_.
