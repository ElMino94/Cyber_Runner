# Minimum features
- GPU driven culling
- Raytraced shadows
- CSM
- Simple post processing
- Skybox rendering
- IBL
- C++ scripting (xmake compilation)
- Physics (jolt)
- Audio (miniaudio)
- Editor/serialization/scenes blablabla (ImGui)
- Actor model for entities
- GLTF model loading and image loading with STB
- Simple asset export pipeline (BC7, compressed mesh format)
- Simple in-game UI

## Would be good to have

- Animations
- LOD selection
- Mesh shaders
- SSR
- RTAO
- AngelScript support

## TODO

- PassIO, editor renders to viewport, runtime renders to backbuffer
- World system to handle transitions
- World serialization/deserialization (JSON)
- Simple HDR forward pass + tonemap via PassIO + debug renderer
- Cube renderer with entities from the world
- Asset system (ref counting/caching)
- Input system (keyboard, mouse, gamepad)
- Audio system (audio source, listener, simple playback)
- Rendering 3D models (static GLTF)
- Physics system (implement different types of colliders)
- Scripting system (how tho)
- Project system (simple)
- Vulkan TLAS/BLAS/AS Build
- GPU driven frustum cull
- RT shadows
- CSM
- Shadow atlas
- IBL
- PostFX (DOF, Bloom, TAA)
- Skybox
- Batched 2D sprite renderer
