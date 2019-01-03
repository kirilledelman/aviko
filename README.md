# aviko

## Building on Raspberry Pi

First, update, and install required libraries:
```
sudo apt update
sudo apt install libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev libsdl2-mixer-2.0-0 libsdl2-mixer-dev libsdl2-net-2.0-0 libsdl2-net-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev libmozjs-24-0 libmozjs-24-dev
```
Then, fetch current Aviko repository:
```
git https://github.com/kirilledelman/aviko.git aviko
cd aviko
```
Fetch required SDL_gpu library
```
git clone https://github.com/grimfang4/sdl-gpu.git sdl-gpu
cd sdl-gpu
```
Open SDL_gpu's make file:
```
nano CMakeLists.txt
```
Press Ctrl+W to invoke search, and type `USE_BRO`, press Enter, you'll see this line:
```
add_definitions(-DSDL_GPU_USE_BROADCOM_RASPBERRYPI_WORKAROUND)
```
Add `#` in front of it, so it's commented out:
```
# add_definitions(-DSDL_GPU_USE_BROADCOM_RASPBERRYPI_WORKAROUND)
```
Now, Ctrl+W again, and type `1.X`, then Enter. You'll see a block that looks like this:
```
option(SDL_gpu_DISABLE_OPENGL_1 "Disable OpenGL 1.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_2 "Disable OpenGL 2.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_3 "Disable OpenGL 3.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_4 "Disable OpenGL 4.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_1 "Disable OpenGLES 1.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_2 "Disable OpenGLES 2.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_3 "Disable OpenGLES 3.X renderer" OFF)
```
Change it, so only "Disable OpenGLES 2.X renderer" is OFF, and all others are ON:
```
option(SDL_gpu_DISABLE_OPENGL_1 "Disable OpenGL 1.X renderer" ON)
option(SDL_gpu_DISABLE_OPENGL_2 "Disable OpenGL 2.X renderer" ON)
option(SDL_gpu_DISABLE_OPENGL_3 "Disable OpenGL 3.X renderer" ON)
option(SDL_gpu_DISABLE_OPENGL_4 "Disable OpenGL 4.X renderer" ON)
option(SDL_gpu_DISABLE_GLES_1 "Disable OpenGLES 1.X renderer" ON)
option(SDL_gpu_DISABLE_GLES_2 "Disable OpenGLES 2.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_3 "Disable OpenGLES 3.X renderer" ON)
```
Press Ctrl+O to save changes, and press Enter at file name prompt. Then press Ctrl+X to exit the text editor. 

Now, we'll build, install, and remove SDL_gpu directory (you can ignore build warnings):
```
cmake -G "Unix Makefiles"
make
sudo make install
cd ..
rm -rf sdl-gpu
```
Now, let's build and install Aviko:
```
mkdir obj
make
sudo make install
```

