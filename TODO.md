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

### Friday
- Change camera to main camera on play
- Asset system (ref counting/caching)
- 3D audio + audio components
- Textures
- Rendering 3D models (static GLTF) + mesh component
- Simple HDR forward pass + tonemap via PassIO + debug renderer
- Physics system (implement different types of colliders)

### Saturday
- Microfacet BRDF
- Skybox
- IBL
- CSM
- TAA
- Bloom
- Runtime renders to backbuffer

## Sunday
- Material asset
- Asset compression and packing on export
- Project system

## After
- Vulkan and Metal TLAS/BLAS/AS Build
- RT shadows
- GPU driven frustum cull
- Shadow atlas
- DOF
- Batched 2D sprite renderer
- Text renderer
