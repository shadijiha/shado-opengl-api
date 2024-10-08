# Shado Game engine (scroll for images)

This is simplified version of unity game engine written from scratch in C++.
This engine supports:

- Adding, moving and deleting entities (with a nice editor)
- Attaching components to entities (with entt)
- Adding physics components to entities (with box2D)
- Adding different cameras
- Scripting using C# with an API similar to unity

This engine was built from scratch and uses the following the following libraries and technologies:

- GLFW and GLEW
- OpenGL with GLSL 4.1 shaders
- box2d for physics
- entt for entity component system
- ImGui and ImGuizmo for UI
- spdlog for debug and logging
- glm for math
- mono as a bridge between C# and C++
- yaml Cpp for serialization

## Screenshots

Here's a demo of the physics system
![Physics system demo](https://github.com/shadijiha/shado-opengl-api/blob/main/shado-editor/assets/screenshots/runtime.gif?raw=true)

Here's a code snippet from the engine code (on the left in C++) and scripting code (on the right in C#)
![Code demo](https://github.com/shadijiha/shado-opengl-api/blob/main/shado-editor/assets/screenshots/code.jpg?raw=true)

## How to build and run?

- Clone the main branch recursively <code>git clone --recursive https://github.com/shadijiha/shado-opengl-api.git</code>
- Run setup.bat
- Open the solution file, switch to release mode
- Build and run and enjoy!
