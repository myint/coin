/* Example demonstrating shadow rendering with Coin3D.
 *
 * This example shows how to use SoShadowGroup to create dynamic shadows
 * from a rotating cube onto a ground plane, lit by a directional light.
 *
 * Note: This example uses GLFW and does not require SoGUI libraries.
 */

#include <Inventor/SoDB.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoRotor.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/sensors/SoTimerSensor.h>

#include <cmath>

// Shadow support
#include <Inventor/annex/FXViz/nodes/SoShadowGroup.h>
#include <Inventor/annex/FXViz/nodes/SoShadowStyle.h>
#include <Inventor/annex/FXViz/nodes/SoShadowDirectionalLight.h>

#include <cstdlib>
#include <cstdio>
#include <functional>

#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

// ----------------------------------------------------------------------

GLFWwindow* window;
SoSceneManager* sceneManager;
SoCamera* camera;
SoRotor* rotor;
SoShadowGroup* shadowGroup;

// Frame rate tracking
double lastTime = 0.0;
int frameCount = 0;
double fps = 0.0;

// Mouse controls
bool mouseLeftDown = false;
bool mouseRightDown = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
float cameraDistance = 100.0f;
float cameraRotationY = 0.0f;  // Azimuth
float cameraRotationX = 0.6f;  // Elevation
SbVec3f cameraTarget(0.0f, 0.0f, 0.0f);

// ----------------------------------------------------------------------

// Update camera position based on spherical coordinates
void updateCamera()
{
    if (!camera) return;

    // Calculate camera position using spherical coordinates
    float x = cameraTarget[0] + cameraDistance * cosf(cameraRotationX) * sinf(cameraRotationY);
    float y = cameraTarget[1] + cameraDistance * sinf(cameraRotationX);
    float z = cameraTarget[2] + cameraDistance * cosf(cameraRotationX) * cosf(cameraRotationY);

    camera->position = SbVec3f(x, y, z);

    // Calculate orientation to look at target
    SbVec3f direction = cameraTarget - SbVec3f(x, y, z);
    direction.normalize();

    SbVec3f up(0, 1, 0);
    SbVec3f right = direction.cross(up);
    right.normalize();
    up = right.cross(direction);
    up.normalize();

    SbMatrix m = SbMatrix::identity();
    m[0][0] = right[0];
    m[0][1] = right[1];
    m[0][2] = right[2];
    m[1][0] = up[0];
    m[1][1] = up[1];
    m[1][2] = up[2];
    m[2][0] = -direction[0];
    m[2][1] = -direction[1];
    m[2][2] = -direction[2];

    SbRotation rotation;
    rotation.setValue(m);
    camera->orientation = rotation;

    sceneManager->scheduleRedraw();
}

// Mouse button callback
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseLeftDown = (action == GLFW_PRESS);
        if (mouseLeftDown) {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        mouseRightDown = (action == GLFW_PRESS);
        if (mouseRightDown) {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
    }
}

// Mouse motion callback
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    double dx = xpos - lastMouseX;
    double dy = ypos - lastMouseY;

    if (mouseLeftDown) {
        // Rotate camera
        cameraRotationY += dx * 0.005f;
        cameraRotationX -= dy * 0.005f;

        // Clamp elevation to avoid flipping
        const float maxElevation = 1.5f;
        const float minElevation = -1.5f;
        if (cameraRotationX > maxElevation) cameraRotationX = maxElevation;
        if (cameraRotationX < minElevation) cameraRotationX = minElevation;

        updateCamera();
    }
    else if (mouseRightDown) {
        // Pan camera (move target)
        SbVec3f right, up;
        camera->orientation.getValue().multVec(SbVec3f(1, 0, 0), right);
        camera->orientation.getValue().multVec(SbVec3f(0, 1, 0), up);

        float panSpeed = cameraDistance * 0.001f;
        cameraTarget += right * (float)(-dx * panSpeed);
        cameraTarget += up * (float)(dy * panSpeed);

        updateCamera();
    }

    lastMouseX = xpos;
    lastMouseY = ypos;
}

// Mouse scroll callback for zoom
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraDistance *= (1.0f - yoffset * 0.1f);

    // Clamp distance
    if (cameraDistance < 5.0f) cameraDistance = 5.0f;
    if (cameraDistance > 500.0f) cameraDistance = 500.0f;

    updateCamera();
}

// ----------------------------------------------------------------------

// Redraw on scenegraph changes.
void redrawCallback(void * user, SoSceneManager * manager)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  sceneManager->render();
  glfwSwapBuffers(window);
}

// Redraw on expose events.
void exposeCallback(void)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  sceneManager->render();
  glfwSwapBuffers(window);
}

// Reconfigure on changes to window dimensions.
void framebufferSizeCallback(GLFWwindow* window, int w, int h)
{
  sceneManager->setWindowSize(SbVec2s(w, h));
  sceneManager->scheduleRedraw();
}

// Process the internal Coin queues when idle.
void idleCallback(void)
{
  SoDB::getSensorManager()->processTimerQueue();
  SoDB::getSensorManager()->processDelayQueue(TRUE);
}

// Update and display FPS
void updateFPS(void)
{
  double currentTime = glfwGetTime();
  frameCount++;

  // Update FPS every second
  if (currentTime - lastTime >= 1.0) {
    fps = frameCount / (currentTime - lastTime);
    printf("FPS: %.1f\n", fps);
    fflush(stdout);
    frameCount = 0;
    lastTime = currentTime;
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS) {
    switch(key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
      case GLFW_KEY_SPACE:
        // Toggle rotation
        rotor->on = !rotor->on.getValue();
        printf("Rotation: %s\n", rotor->on.getValue() ? "ON" : "OFF");
        break;
      case GLFW_KEY_C:
        // Toggle shadow caching
        shadowGroup->shadowCachingEnabled = !shadowGroup->shadowCachingEnabled.getValue();
        printf("Shadow caching: %s\n", shadowGroup->shadowCachingEnabled.getValue() ? "ON" : "OFF");
        break;
    }
  }
}

// ----------------------------------------------------------------------

// Create a high-polygon terrain mesh
SoSeparator* createTerrain(int gridSize, float size, float heightScale)
{
    SoSeparator* terrainSep = new SoSeparator;

    // Shape hints for proper lighting
    SoShapeHints* hints = new SoShapeHints;
    hints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    hints->shapeType = SoShapeHints::SOLID;
    hints->creaseAngle = 0.8f;
    terrainSep->addChild(hints);

    // Material
    SoMaterial* mat = new SoMaterial;
    mat->diffuseColor = SbColor(0.4f, 0.6f, 0.3f);  // Green-ish terrain
    mat->specularColor = SbColor(0.1f, 0.1f, 0.1f);
    mat->shininess = 0.1f;
    terrainSep->addChild(mat);

    // Generate terrain vertices with height variation
    int numVertices = gridSize * gridSize;
    SoCoordinate3* coords = new SoCoordinate3;
    coords->point.setNum(numVertices);
    SbVec3f* vertices = coords->point.startEditing();

    float step = size / (gridSize - 1);
    float halfSize = size / 2.0f;

    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int idx = z * gridSize + x;
            float xPos = x * step - halfSize;
            float zPos = z * step - halfSize;

            // Generate height using multiple sine waves for interesting terrain
            float height = 0.0f;
            height += sinf(xPos * 0.5f) * cosf(zPos * 0.5f) * heightScale;
            height += sinf(xPos * 1.2f + 1.0f) * cosf(zPos * 0.8f) * heightScale * 0.5f;
            height += sinf(xPos * 2.5f) * sinf(zPos * 2.5f) * heightScale * 0.25f;

            vertices[idx] = SbVec3f(xPos, height - 3.0f, zPos);
        }
    }
    coords->point.finishEditing();
    terrainSep->addChild(coords);

    // Generate normals (per-vertex)
    SoNormal* normals = new SoNormal;
    normals->vector.setNum(numVertices);
    SbVec3f* norms = normals->vector.startEditing();

    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int idx = z * gridSize + x;

            // Calculate normal from neighboring vertices
            SbVec3f center = vertices[idx];
            SbVec3f normal(0, 0, 0);

            // Get neighboring vertices (with boundary checks)
            SbVec3f left = (x > 0) ? vertices[idx - 1] : center;
            SbVec3f right = (x < gridSize - 1) ? vertices[idx + 1] : center;
            SbVec3f up = (z > 0) ? vertices[idx - gridSize] : center;
            SbVec3f down = (z < gridSize - 1) ? vertices[idx + gridSize] : center;

            // Cross products to get normal
            SbVec3f dx = right - left;
            SbVec3f dz = down - up;
            normal = dz.cross(dx);
            normal.normalize();

            norms[idx] = normal;
        }
    }
    normals->vector.finishEditing();
    terrainSep->addChild(normals);

    SoNormalBinding* normalBinding = new SoNormalBinding;
    normalBinding->value = SoNormalBinding::PER_VERTEX_INDEXED;
    terrainSep->addChild(normalBinding);

    // Generate face indices
    int numQuads = (gridSize - 1) * (gridSize - 1);
    int numIndices = numQuads * 5;  // 4 vertices + -1 terminator per quad

    SoIndexedFaceSet* faceSet = new SoIndexedFaceSet;
    faceSet->coordIndex.setNum(numIndices);
    int32_t* indices = faceSet->coordIndex.startEditing();

    int i = 0;
    for (int z = 0; z < gridSize - 1; z++) {
        for (int x = 0; x < gridSize - 1; x++) {
            int topLeft = z * gridSize + x;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + gridSize;
            int bottomRight = bottomLeft + 1;

            indices[i++] = topLeft;
            indices[i++] = bottomLeft;
            indices[i++] = bottomRight;
            indices[i++] = topRight;
            indices[i++] = -1;  // End of face
        }
    }
    faceSet->coordIndex.finishEditing();
    terrainSep->addChild(faceSet);

    return terrainSep;
}

SoSeparator* createScene();

std::function<void()> loop;
void main_loop() { loop(); }

int main(void)
{
    glfwSetErrorCallback([](int error, const char* description) {
      fprintf(stderr, "Error: %s\n", description);
    });

    SoDB::init();

    if (!glfwInit())
        return EXIT_FAILURE;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(800, 600, "Coin3D Shadow Example", NULL, NULL);
    if (!window) {
      glfwTerminate();
      return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwMakeContextCurrent(window);

    // Initialize shadow support (after OpenGL context is created)
    SoShadowGroup::initClass();
    SoShadowStyle::initClass();
    SoShadowDirectionalLight::initClass();

    SoSeparator* root = createScene();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSizeCallback(window, width, height);

    printf("Shadow Example Controls:\n");
    printf("  Left Mouse  - Rotate camera\n");
    printf("  Right Mouse - Pan camera\n");
    printf("  Scroll      - Zoom in/out\n");
    printf("  SPACE       - Toggle rotation\n");
    printf("  C           - Toggle shadow caching\n");
    printf("  ESC         - Exit\n");
    printf("\nShadow caching: ON\n");

    // Initialize FPS tracking
    lastTime = glfwGetTime();

    // Initialize camera position
    updateCamera();

    loop = [&] {
        glfwPollEvents();
        idleCallback();
        exposeCallback();
        updateFPS();
    };

    while (!glfwWindowShouldClose(window))
        main_loop();

    root->unref();
    delete sceneManager;

    glfwTerminate();
    return 0;
}

// ----------------------------------------------------------------------

SoSeparator* createScene()
{
    auto root = new SoSeparator;
    root->ref();

    // Camera setup - pulled back to see the larger terrain
    SoPerspectiveCamera * perspectiveCamera = new SoPerspectiveCamera;
    perspectiveCamera->position = SbVec3f(40, 40, 70);
    perspectiveCamera->orientation = SbRotation(SbVec3f(-1, 1, 0), 0.6f);
    perspectiveCamera->nearDistance = 1.0f;
    perspectiveCamera->farDistance = 1000.0f;
    camera = perspectiveCamera;
    root->addChild(perspectiveCamera);

    // Create shadow group - this enables shadow rendering
    shadowGroup = new SoShadowGroup;
    shadowGroup->quality = 1.0f;        // High quality for per-pixel lighting
    shadowGroup->precision = 1.0f;      // High precision shadows
    shadowGroup->intensity = 0.7f;      // Shadow darkness
    shadowGroup->isActive = TRUE;
    shadowGroup->shadowCachingEnabled = TRUE;  // Enable shadow map caching
    root->addChild(shadowGroup);

    // Add a directional light that casts shadows
    SoShadowDirectionalLight * light = new SoShadowDirectionalLight;
    light->direction = SbVec3f(1, -1, -1);
    light->intensity = 0.9f;
    light->color = SbColor(1, 1, 1);
    shadowGroup->addChild(light);

    // Add ambient light for better visibility
    SoDirectionalLight * ambientLight = new SoDirectionalLight;
    ambientLight->direction = SbVec3f(0, 0, -1);
    ambientLight->intensity = 0.3f;
    shadowGroup->addChild(ambientLight);

    // --- Create high-polygon terrain (shadow receiver) ---
    // 2100x2100 grid = 4,410,000 vertices, ~8,820,000 triangles
    const int TERRAIN_GRID_SIZE = 2100;
    const float TERRAIN_SIZE = 70.0f;
    const float TERRAIN_HEIGHT = 2.5f;

    SoSeparator * terrainSep = new SoSeparator;
    shadowGroup->addChild(terrainSep);

    SoShadowStyle * terrainStyle = new SoShadowStyle;
    terrainStyle->style = SoShadowStyle::CASTS_SHADOW_AND_SHADOWED;
    terrainSep->addChild(terrainStyle);

    SoSeparator * terrain = createTerrain(TERRAIN_GRID_SIZE, TERRAIN_SIZE, TERRAIN_HEIGHT);
    terrainSep->addChild(terrain);

    int terrainPolys = (TERRAIN_GRID_SIZE - 1) * (TERRAIN_GRID_SIZE - 1) * 2;
    printf("Terrain: %dx%d grid = %d polygons\n", TERRAIN_GRID_SIZE, TERRAIN_GRID_SIZE, terrainPolys);

    // --- Create rotating cube (shadow caster) ---
    SoSeparator * cubeSep = new SoSeparator;
    shadowGroup->addChild(cubeSep);

    // Cube casts shadows and receives them
    SoShadowStyle * cubeStyle = new SoShadowStyle;
    cubeStyle->style = SoShadowStyle::CASTS_SHADOW_AND_SHADOWED;
    cubeSep->addChild(cubeStyle);

    SoMaterial * cubeMat = new SoMaterial;
    cubeMat->diffuseColor = SbColor(0.2f, 0.6f, 0.9f);
    cubeMat->specularColor = SbColor(0.8f, 0.8f, 0.8f);
    cubeMat->shininess = 0.8f;
    cubeSep->addChild(cubeMat);

    SoTranslation * cubePos = new SoTranslation;
    cubePos->translation = SbVec3f(0, 1, 0);
    cubeSep->addChild(cubePos);

    // Add rotation animation
    rotor = new SoRotor;
    rotor->rotation = SbRotation(SbVec3f(0, 1, 0), 0);
    rotor->speed = 0.3f;
    rotor->on = TRUE;
    cubeSep->addChild(rotor);

    SoCube * cube = new SoCube;
    cube->width = 2.0f;
    cube->height = 2.0f;
    cube->depth = 2.0f;
    cubeSep->addChild(cube);

    // --- Create multiple high-polygon spheres ---
    // High complexity spheres for stress testing
    const int NUM_SPHERES = 81;  // 9x9 grid
    const float SPHERE_COMPLEXITY = 1.0f;  // Maximum tessellation

    SoSeparator * spheresSep = new SoSeparator;
    shadowGroup->addChild(spheresSep);

    SoShadowStyle * spheresStyle = new SoShadowStyle;
    spheresStyle->style = SoShadowStyle::CASTS_SHADOW_AND_SHADOWED;
    spheresSep->addChild(spheresStyle);

    // High complexity for all spheres
    SoComplexity * complexity = new SoComplexity;
    complexity->value = SPHERE_COMPLEXITY;
    spheresSep->addChild(complexity);

    // Create a grid of spheres (9x9 = 81 spheres)
    const int SPHERE_GRID = 9;
    int sphereCount = 0;
    for (int row = 0; row < SPHERE_GRID; row++) {
        for (int col = 0; col < SPHERE_GRID; col++) {
            SoSeparator * sphereSep = new SoSeparator;
            spheresSep->addChild(sphereSep);

            // Varying colors
            SoMaterial * sphereMat = new SoMaterial;
            float r = 0.3f + (row * 0.08f);
            float g = 0.3f + (col * 0.08f);
            float b = 0.9f - ((row + col) * 0.05f);
            sphereMat->diffuseColor = SbColor(r, g, b);
            sphereMat->specularColor = SbColor(0.8f, 0.8f, 0.8f);
            sphereMat->shininess = 0.8f;
            sphereSep->addChild(sphereMat);

            SoTransform * sphereTransform = new SoTransform;
            float x = (col - (SPHERE_GRID-1)/2.0f) * 2.0f;
            float z = (row - (SPHERE_GRID-1)/2.0f) * 2.0f;
            float y = 0.5f + sinf(x * 0.5f) * cosf(z * 0.5f) * 0.5f;
            sphereTransform->translation = SbVec3f(x, y, z);
            sphereSep->addChild(sphereTransform);

            SoSphere * sphere = new SoSphere;
            sphere->radius = 0.5f;
            sphereSep->addChild(sphere);

            sphereCount++;
        }
    }

    // Estimate sphere polygon count (complexity 1.0 = ~2000 tris per sphere)
    int spherePolys = sphereCount * 2000;
    printf("Spheres: %d spheres at complexity %.1f = ~%d polygons\n",
           sphereCount, SPHERE_COMPLEXITY, spherePolys);

    printf("Total: ~%d polygons\n", terrainPolys + spherePolys);
    printf("----------------------------------------\n");

    // Setup scene manager
    sceneManager = new SoSceneManager;
    sceneManager->setRenderCallback(redrawCallback, (void *)1);
    sceneManager->setBackgroundColor(SbColor(0.2f, 0.3f, 0.4f));
    sceneManager->activate();
    sceneManager->setSceneGraph(root);

    return root;
}
