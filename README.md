# Deference
Quick showcase of D3D12 and DXR. Implements a hybrid-rendering pipeline which integrates rasterization and raytracing.

## Prereqs
- DirectX 12 runtime support
- Raytracing-supported GPU (Tested with NVIDIA RTX 2060 Ti)

## Build and Run
The project can be built using CMake(3.18 or greater). Currently, x64-debug is the only stable and tested configuration. 
Run the executable binary located within the binary directory(out/build/x64-debug by default).

## Controls
- WASD: move camera
- Tab: toggle camera look with mouse
- Esc: Quit application

## Hybrid rendering with a frame graph
The frame graph currently uses a rasterized geometry pass for filling in position, normal, and albedo information. Subsequent passes will use this information for lighting/shadows. This hybrid approach not only performs better in real-time compared to path tracing, but it's also easier to implement as uploading vertex, index, and material information to the raytracing shaders for a raytraced geometry pass relies on additional setup.

## Screenshots 
Normal map support  
![normal_mapping](https://github.com/swaggimus-grime/Deference/blob/657c017c2ac29cd386dcd2c3f7f336db578b5cb7/screenshots/normal_mapping.png)

Ambient occlusion
![ao](https://github.com/swaggimus-grime/Deference/blob/657c017c2ac29cd386dcd2c3f7f336db578b5cb7/screenshots/ao.png)

Albedo channel
![albedo](https://github.com/swaggimus-grime/Deference/blob/657c017c2ac29cd386dcd2c3f7f336db578b5cb7/screenshots/albedo.png)

Floating UI
![UI](https://github.com/swaggimus-grime/Deference/blob/657c017c2ac29cd386dcd2c3f7f336db578b5cb7/screenshots/ui.png)

Hybrid output
![Hybrid output](https://github.com/swaggimus-grime/Deference/blob/657c017c2ac29cd386dcd2c3f7f336db578b5cb7/screenshots/hybrid.png)
