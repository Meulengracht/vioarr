# vioarr
Open source window manager with complimentary apps. The window manager provides a window manager protocol that anyone would be able to implement, and
it allows for a very flexible system. Vioarr comes with a C++ application framework, which can be useful as a quick start for new applications. The window
manager is still in its early stages, and there still needs to be support for opengl/vulkan context creation in its applications for drawing, right now
drawing is manually done on a framebuffer. 

Vioarr's protocol is defined in the protocol folder, and uses Gracht for code generation, and as it's communications library.

Vioarr is the default window manager in Vali/MollenOS, and consists of the following components

- Asgaard
- Vioarr (core)

Asgaard provides a default client implementation framework that apps can quickly be built from.

## Applications that comes with this window manager
- Terminal (Alumni)
- Launcher (Heimdall)

More applications are planned, and a much better application environment is also in the works.

## Build Instructions

To build vioarr, GLAD must be installed as a python3 module

```
pip3 install --upgrade git+https://github.com/meulengracht/glad.git#egg=glad
```
