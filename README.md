# FreezeRay
FreezeRay is a physically-based ray tracer. It is intended to provide an easy way to comprae the performance of different rendering equation integrators, such as path traching and bidirectional path tracing. It is my final capstone project for the [Rutgers Honors College](https://honorscollege.rutgers.edu/academics/curriculum/capstone-project). Much of the code architecture is based off of the textbook [Physically Based Rendering](https://www.pbr-book.org/3ed-2018/contents) by Pharr, Jakob, and Humphreys, as well as the accompanying [source code](https://github.com/mmp/pbrt-v3/tree/master). 

FreezeRay is intended to be as simple as possible while still providing enough features to render photorealistic scenes. It supports:
- Multiple physically-based BSDF models
- Multiple physically-based lights
- Multiple material models (aggregates of BSDFS)
- Path tracing with multiple importance sampling
- Bidirectional path tracing with multiple importance sampling
- Metropolis light transport
- Loading and rendering of scenes from `.obj` files
- A flexible code architecture, allowing easy addition of BSDFs/lights/materials/integrators

See `writeup.pdf` for a full description of FreezeRay and it's development process.

## Results
![San Miguel Render](renders/san_miguel_reference.png)
![Sponza Render](renders/sponza_reference.png)
![Cornell Box Render](renders/cornell_box_reference.png)

## Building
FreezeRay uses `cmake` as its build system. To build the project, run the following commands from the project root:
```bash
mkdir build
cd build
cmake ..
```
This will generate platform-specific build files. On Unix, this is likely a Makefile, so simply run `make` to generate the executable. On Window, this is likely a Visual Studio solution, so simply open the `.sln` file and build from VS.

The generated executable takes a single parameter: a path to an output file, where the final render will be stored. It also takes an optional 2nd parameter: a path to an image against which to compute MSE. A preview of the render will also be shown as it is rendering. Modify `src/main.cpp` to choose which scene gets rendered. You may need to download some assets yourself, as they are too big to store on GitHub, they can be found [here](https://casual-effects.com/data/).
