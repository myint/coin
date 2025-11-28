# Coin3D Shadow Rendering Example

This example demonstrates dynamic shadow rendering using Coin3D's FXViz shadow nodes.

## Overview

The shadow example showcases:
- Real-time shadow casting and receiving using `SoShadowGroup`
- Directional light shadows with `SoShadowDirectionalLight`
- Multiple shadow-casting objects in the same scene
- Animated geometry with dynamic shadow updates
- GLFW-based rendering without requiring SoQt/SoWin GUI libraries
- Real-time FPS monitoring for performance analysis

## Scene Description

The scene consists of:
1. **Ground plane**: A large flat gray cube that receives shadows
2. **Rotating blue cube**: Animated cube that casts and receives shadows
3. **Static red cube**: Fixed cube positioned to the side
4. **Directional light**: Shadow-casting light source
5. **Ambient light**: Additional non-shadow-casting light for visibility

## Building

The shadow example is built as part of the Coin3D examples when `COIN_BUILD_EXAMPLES=ON`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCOIN_BUILD_EXAMPLES=ON \
  -DCMAKE_PREFIX_PATH="/opt/local/libexec/boost/1.76;/opt/local"
cmake --build build --target Shadows --config Release
```

### Dependencies

- **Coin3D**: The main library (built automatically)
- **GLFW 3.3+**: Cross-platform window and OpenGL context management
- **Boost**: C++ header-only libraries
- **OpenGL 2.1+**: Graphics API

## Running

```bash
./build/bin/Shadows
```

On macOS with X11, you may need to set the DISPLAY variable:
```bash
DISPLAY=:0 ./build/bin/Shadows
```

## Controls

- **SPACE**: Toggle rotation animation on/off
- **ESC**: Exit the application

## Performance

The example includes real-time FPS monitoring that prints to stdout every second:
```
Shadow Example Controls:
  SPACE - Toggle rotation
  ESC   - Exit
FPS: 24.9
FPS: 25.0
FPS: 24.9
```

This FPS output is useful for:
- Performance benchmarking
- Optimization testing
- Comparing different shadow quality settings
- Profiling shadow rendering overhead

## Code Structure

### Main Components

**Scene Setup** (`createScene()`):
- Creates camera with positioned viewpoint
- Initializes `SoShadowGroup` for shadow management
- Adds shadow-casting directional light
- Constructs ground plane and cubes with appropriate materials
- Configures shadow styles for each object

**Rendering Loop**:
- GLFW event polling
- Coin sensor queue processing (for animation)
- Scene rendering via `SoSceneManager`
- FPS calculation and display

**Shadow Configuration**:
```cpp
SoShadowGroup * shadowGroup = new SoShadowGroup;
shadowGroup->quality = 1.0f;        // High quality per-pixel lighting
shadowGroup->precision = 1.0f;      // High precision shadows
shadowGroup->intensity = 0.7f;      // Shadow darkness (0=none, 1=black)
shadowGroup->isActive = TRUE;
```

**Shadow Style Options**:
- `SoShadowStyle::CASTS_SHADOW_AND_SHADOWED`: Object casts and receives shadows
- `SoShadowStyle::CASTS_SHADOW`: Only casts shadows
- `SoShadowStyle::SHADOWED`: Only receives shadows
- `SoShadowStyle::NO_SHADOWING`: No shadow interaction

### Key Classes Used

- `SoShadowGroup`: Root node for shadow rendering subsystem
- `SoShadowDirectionalLight`: Light source that casts shadows
- `SoShadowStyle`: Controls shadow casting/receiving per object
- `SoSceneManager`: Manages scene graph rendering
- `SoRotor`: Provides automatic rotation animation
- `SoPerspectiveCamera`: 3D perspective camera
- `SoCube`, `SoMaterial`, `SoTransform`: Basic scene elements

## Technical Details

### Shadow Implementation

Coin3D's shadow system uses shadow mapping techniques:
1. Renders scene from light's perspective to create shadow map
2. Projects shadow map onto scene during final rendering
3. Uses depth comparison to determine shadowed areas

### OpenGL Context Management

**Important**: Shadow classes must be initialized after a valid OpenGL context exists:

```cpp
// Create GLFW window and make context current first
glfwMakeContextCurrent(window);

// Then initialize shadow support
SoShadowGroup::initClass();
SoShadowStyle::initClass();
SoShadowDirectionalLight::initClass();
```

Initializing shadow classes before the OpenGL context can cause crashes on some platforms (particularly macOS).

### Platform Notes

**macOS**: Uses Core Graphics Layer (CGL) for OpenGL context. The example avoids calling `SoShadowGroup::isSupported()` as it may create problematic off-screen contexts on some macOS versions.

**Linux**: Should work with both GLX (X11) and EGL contexts.

**Windows**: Uses WGL for OpenGL context management.

## Customization

### Adjusting Shadow Quality

Modify shadow group parameters for different quality/performance tradeoffs:

```cpp
shadowGroup->quality = 0.5f;      // Lower for better performance
shadowGroup->precision = 0.5f;    // Lower for softer shadows
shadowGroup->intensity = 0.5f;    // Lighter shadows
shadowGroup->threshold = 0.3f;    // Adjust shadow bias
```

### Adding More Objects

To add shadow-casting objects:

```cpp
SoSeparator * objSep = new SoSeparator;
shadowGroup->addChild(objSep);

// Configure shadow behavior
SoShadowStyle * objStyle = new SoShadowStyle;
objStyle->style = SoShadowStyle::CASTS_SHADOW_AND_SHADOWED;
objSep->addChild(objStyle);

// Add material and geometry
SoMaterial * objMat = new SoMaterial;
objMat->diffuseColor = SbColor(1.0f, 0.5f, 0.0f);
objSep->addChild(objMat);

SoSphere * sphere = new SoSphere;
objSep->addChild(sphere);
```

### Changing Light Direction

Modify the shadow light's direction vector:

```cpp
light->direction = SbVec3f(1, -1, 0);  // Shadow falls to the right
light->direction = SbVec3f(0, -1, 0);  // Shadow directly below objects
```

## Troubleshooting

**Shadows not visible**:
- Ensure your GPU supports the required OpenGL extensions
- Check that `SoShadowGroup::isActive` is TRUE
- Verify shadow intensity is > 0

**Low FPS**:
- Reduce `shadowGroup->quality` setting
- Decrease shadow map resolution
- Simplify scene geometry

**Crashes on startup**:
- Verify OpenGL context is created before initializing shadow classes
- Check that GLFW and OpenGL dependencies are properly installed
- Try removing the `SoShadowGroup::isSupported()` check if present

## References

- [Coin3D FXViz Documentation](https://github.com/coin3d/coin/wiki)
- [Open Inventor Mentor](http://www-evasion.imag.fr/Membres/Francois.Faure/doc/inventorMentor/sgi_html/)
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- Shadow Mapping technique: [Learn OpenGL - Shadow Mapping](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)

## License

This example is part of Coin3D and is released under the BSD License.
Copyright Kongsberg Oil & Gas Technologies AS.
