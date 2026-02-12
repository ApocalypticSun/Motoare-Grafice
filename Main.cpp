#include "Main.h"
#include "ObjectLoader.h"
#include <algorithm>
#include <cmath>

static constexpr unsigned int kWindowW = 800;
static constexpr unsigned int kWindowH = 800;

static constexpr float kFovDeg = 45.0f;
static constexpr float kNearPlane = 0.1f;
static constexpr float kFarPlane = 50.0f;

// UI placement in front of camera
static constexpr float kUiDist = 1.0f;
static constexpr float kUiSide = 0.2f;
static constexpr float kUiDown = -0.2f;

// Light movement
static constexpr float kLightStep = 1.0f;
static constexpr float kLightLimit = 5.0f;


// Ray vs AABB (slab method)
static bool RayAABB(const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const glm::vec3& boxCenter,
    const glm::vec3& boxHalfSize,
    float& hitDistance)
{
    const glm::vec3 invDir = 1.0f / rayDirection;
    const glm::vec3 boxMin = boxCenter - boxHalfSize;
    const glm::vec3 boxMax = boxCenter + boxHalfSize;

    const glm::vec3 tNear = (boxMin - rayOrigin) * invDir;
    const glm::vec3 tFar = (boxMax - rayOrigin) * invDir;

    const glm::vec3 tMin = glm::min(tNear, tFar);
    const glm::vec3 tMax = glm::max(tNear, tFar);

    const float entryDistance = std::max({ tMin.x, tMin.y, tMin.z });
    const float exitDistance = std::min({ tMax.x, tMax.y, tMax.z });

    if (exitDistance < 0.0f || entryDistance > exitDistance)
        return false;

    hitDistance = (entryDistance >= 0.0f) ? entryDistance : exitDistance;
    return true;
}

static glm::vec3 MouseRayDirection(GLFWwindow* window,
    const Camera& camera,
    int screenWidth,
    int screenHeight)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    const float ndcX = 2.0f * float(mouseX) / float(screenWidth) - 1.0f;
    const float ndcY = 1.0f - 2.0f * float(mouseY) / float(screenHeight);

    const glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up);
    const glm::mat4 proj = glm::perspective(glm::radians(kFovDeg), float(screenWidth) / float(screenHeight), kNearPlane, kFarPlane);

    glm::vec4 rayEye = glm::inverse(proj) * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    return glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
}

static bool IsObjectHitByMouse(GLFWwindow* window,
    const Camera& camera,
    MeshSystem& mesh,
    const char* objectName,
    int screenWidth,
    int screenHeight)
{
    SceneObject* object = mesh.FindObject(objectName);
    if (!object) return false;

    const glm::vec3 rayOrigin = camera.Position;
    const glm::vec3 rayDir = MouseRayDirection(window, camera, screenWidth, screenHeight);

    float hitDistance = 0.0f;
    return RayAABB(rayOrigin, rayDir, object->pos, 0.5f * object->scale, hitDistance);
}

static inline float WrapStep(float v, float step, float limit)
{
    v += step;
    if (v > limit)  v = -limit;
    if (v < -limit) v = limit;
    return v;
}

GLFWwindow* InitWindow(unsigned int w, unsigned int h, const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);

    return window;
}

void RegisterDefaultMeshes(MeshSystem& mesh)
{
    mesh.AddPrimitiveMesh("circle", gfx::ShapeType::Circle);
    mesh.AddPrimitiveMesh("triangle", gfx::ShapeType::Triangle);
    mesh.AddPrimitiveMesh("square", gfx::ShapeType::Square);
    mesh.AddPrimitiveMesh("cube", gfx::ShapeType::Cube);
}

void RegisterDefaultTextures(MeshSystem& mesh, Shader& shader)
{
    mesh.AddTexture("anime", "poza.jpg");
    mesh.AddTexture("brick", "brick.jpg");
    mesh.AddTexture("metal", "metal.jpg");

    shader.Activate();
    glUniform1i(glGetUniformLocation(shader.ID, "tex0"), 0);
}

void SpawnDefaultObjects(MeshSystem& mesh)
{
    mesh.AddObjectInstance({ "Circle1",   "circle",   "anime", "default", {0,0,0},     {1,1,1},             Motion::RotateX, 60.0f });
    mesh.AddObjectInstance({ "Triangle1", "triangle", "brick", "default", {1.5f,0,0},  {0.8f,0.8f,0.8f},    Motion::BobY,    0.0f, 0.5f, 2.0f });
    mesh.AddObjectInstance({ "Square1",   "square",   "metal", "default", {-1.5f,0,0}, {0.9f,0.9f,0.9f},    Motion::RotateY, 30.0f });
    mesh.AddObjectInstance({ "Cube1",     "cube",     "brick", "default", {3.0f,0,0},  {0.7f,0.7f,0.7f},    Motion::RotateXY, 90.0f });
}

static void SpawnImportedObject(MeshSystem& mesh)
{
    CpuMeshData imported = ObjectLoader::LoadOBJ("models/Testing1.obj");
    mesh.AddMesh("testing", imported);
    mesh.AddObjectInstance({ "Testing1", "testing", "brick", "default", {0,0,10.0f}, {1,1,1}, Motion::RotateXY, 90.0f });
}

static void SpawnLamp(MeshSystem& mesh, const glm::vec3& lightPos)
{
    mesh.AddObjectInstance({ "Lamp", "cube", "brick", "object", lightPos, {0.2f,0.2f,0.2f}, Motion::None, 0.0f });
}

static void SpawnUiButtons(MeshSystem& mesh)
{
    mesh.AddObjectInstance({ "BtnLeft",  "cube", "anime", "default", {0,0,0}, {0.18f,0.18f,0.18f}, Motion::None, 0.0f });
    mesh.AddObjectInstance({ "BtnRight", "cube", "anime", "default", {0,0,0}, {0.18f,0.18f,0.18f}, Motion::None, 0.0f });
}

static void UpdateUiButtonPositions(MeshSystem& mesh, const Camera& camera)
{
    const glm::vec3 forward = glm::normalize(camera.Orientation);
    const glm::vec3 right = glm::normalize(glm::cross(forward, camera.Up));
    const glm::vec3 up = glm::normalize(camera.Up);

    if (SceneObject* bL = mesh.FindObject("BtnLeft"))
        bL->pos = camera.Position + forward * kUiDist - right * kUiSide + up * kUiDown;

    if (SceneObject* bR = mesh.FindObject("BtnRight"))
        bR->pos = camera.Position + forward * kUiDist + right * kUiSide + up * kUiDown;
}

static bool ConsumeRmbEdge(GLFWwindow* window, bool& wasRmbDown)
{
    const bool isRmbDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    const bool edge = isRmbDown && !wasRmbDown;
    wasRmbDown = isRmbDown;
    return edge;
}

// ------------------------ Main ------------------------

int main()
{
    GLFWwindow* window = InitWindow(kWindowW, kWindowH, "TestOpenGL");
    if (!window) return -1;

    Shader defaultShader("Default.vert", "Default.frag");
    Shader objectShader("Object.vert", "Object.frag");
    Camera camera(kWindowW, kWindowH, glm::vec3(0, 0, 2));

    MeshSystem mesh;
    RegisterDefaultMeshes(mesh);
    RegisterDefaultTextures(mesh, defaultShader);
    SpawnDefaultObjects(mesh);

    mesh.RegisterShaderProgram("default", defaultShader);
    mesh.RegisterShaderProgram("object", objectShader);

    SpawnImportedObject(mesh);

    glm::vec4 lightColor(1, 1, 1, 1);
    glm::vec3 lightPos(0.5f, 0.5f, 0.5f);
    mesh.SetLightParams(lightColor, lightPos);

    SpawnLamp(mesh, lightPos);
    SpawnUiButtons(mesh);

    bool wasRmbDown = false;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);

        UpdateUiButtonPositions(mesh, camera);

        if (ConsumeRmbEdge(window, wasRmbDown))
        {
            if (IsObjectHitByMouse(window, camera, mesh, "BtnLeft", int(kWindowW), int(kWindowH)))
                lightPos.x = WrapStep(lightPos.x, -kLightStep, kLightLimit);

            if (IsObjectHitByMouse(window, camera, mesh, "BtnRight", int(kWindowW), int(kWindowH)))
                lightPos.x = WrapStep(lightPos.x, kLightStep, kLightLimit);
        }

        mesh.SetLightParams(lightColor, lightPos);
        if (SceneObject* lamp = mesh.FindObject("Lamp"))
            lamp->pos = lightPos;

        mesh.Render(camera, (float)glfwGetTime());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    mesh.Shutdown();
    defaultShader.Delete();
    objectShader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
