#include "raylib.h"
#include "raymath.h"
#include "ui.h"
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void InitData();
void UpdateLoop();

// --- WEB BUILD ---
//emcmake cmake -DPLATFORM=Web -S . -B build
// ---           ---

Image GenHeightmap(int width, int height, float a, int mint, int maxt) {
    int dataSize =width * height;
    Color* data = MemAlloc(sizeof(Color) * width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x >= mint && x <= maxt) {
                float fx = (2.0 * PI * x) / (float)width;
                float fy = (2.0 * PI * y) / (float)height;
                int c = fy + sinf(fx) * cosf(2 * a * fy) * 255;
                
                //int c = (sinf(2 * fy) * sinf(fx)) * 255;
                data[width * y + x] = (Color){ c, c, c, 255 };
            } 
            else {
                data[width * y + x] = (Color){ 0,0,0,225 };
            }
        }
    }
   
    Image img = {
        .data = data,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    return img;
}

Model NewModel(Texture2D *texture, float a, int mint, int maxt) {
    Image img = GenHeightmap(50, 50, a, mint, maxt);

    Mesh nmesh = GenMeshHeightmap(img, (Vector3) { 32, 16, 32 });

    Model newModel = LoadModelFromMesh(nmesh);
    newModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

    UnloadImage(img);

    return newModel;
}

void CameraControl(Camera* cam, float* orbit, float* zoom) {
    if (IsKeyDown(KEY_RIGHT)) {
        *orbit -= 0.02;
    }
    else if (IsKeyDown(KEY_LEFT)) {
        *orbit += 0.02;
    }
    (*cam).position.x = *zoom * cosf(*orbit);
    (*cam).position.z = *zoom * sinf(*orbit);

    *zoom -= 2*GetMouseWheelMove();
}

Model* mainModel;
Model model;
Image gradientImg;
Camera camera;
Texture2D texture, equationT;
Vector3 mapPosition;
float a = 0.5;
float t = 0;
float tdir = 1;
float timer = 0;
float orbit = 0;
float zoom = 50;
bool animate = false;

const int screenWidth = 600;
const int screenHeight = 600;

int main(void) {
    
    InitData();
    

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateLoop, 0, 1);
#else
    while (!WindowShouldClose()) {
        UpdateLoop();
    }
#endif

    // Unload Resources
    UnloadTexture(texture);
    UnloadTexture(equationT);
    UnloadModel(*mainModel);
    UnloadFont(font);

    CloseWindow();

    return 0;
}

void UpdateLoop() {
    UpdateCamera(&camera);

    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawModel(*mainModel, mapPosition, 1.0f, SKYBLUE);
    DrawModelWires(*mainModel, mapPosition, 1.0f, (Color) { 255, 255, 255, 50 });

    DrawGrid(32, 1.0f);

    // Coordinate Guide
    DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 32, 0, 0 }, RED);
    DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 0, 32, 0 }, GREEN);
    DrawLine3D((Vector3) { 0, 0, 0 }, (Vector3) { 0, 0, 32 }, BLUE);

    EndMode3D();

    DrawTextEx(font, "x", (Vector2) { 510, 5 }, 24, 2, RED);
    DrawTextEx(font, "t", (Vector2) { 535, 5 }, 24, 2, GREEN);
    DrawTextEx(font, "z", (Vector2) { 560, 5 }, 24, 2, BLUE);

    CameraControl(&camera, &orbit, &zoom);

    DrawTextEx(font, "Animate", (Vector2) { 5, 5 }, 24, 2, BLACK);
    bool button = uiButton(animate ? "Stop" : "Start", (Rectangle) { 5, 25, 100, 25 }, DARKBLUE, SKYBLUE, BLUE);
    if (button) {
        if (animate) {
            UnloadModel(*mainModel);
            model = NewModel(&texture, a, 0, 50);
            mainModel = &model;
        }
        animate = !animate;
    }

    if (uiCounter("alpha", &a, 0.1, (Vector2) { 150, 15 }) && !animate) {
        UnloadModel(*mainModel);
        model = NewModel(&texture, a, 0, 50);
        mainModel = &model;
    }

    DrawRectangle(300 - equationT.width / 2 - 12, 600 - equationT.height - 12, equationT.width + 24, equationT.height + 24, BLUE);
    DrawTexture(equationT, 300 - equationT.width / 2, 600 - equationT.height, WHITE);

    if (animate) {
        timer += GetFrameTime();
        if (timer > 0.05f) {
            timer = 0;
            t += tdir;
            if (t >= 50 || t <= 0) {
                tdir = -tdir;
            }

            UnloadModel(*mainModel);
            model = NewModel(&texture, a, t, t + 5);
            mainModel = &model;

        }
    }

    //DrawTexture(texture, screenWidth - texture.width - 20, 20, RED);
    //DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);

    EndDrawing();
}


void InitData() {
    // Window and Camera Setup
    InitWindow(screenWidth, screenHeight, "Differential Equation Viewer");
    camera = (Camera){ { 36.0f, 36.0f, 36.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
    camera.target.y = 8;
    SetCameraMode(camera, CAMERA_PERSPECTIVE);
    SetTargetFPS(60);

    // Generate Gradient Texture for Graph
    gradientImg = GenImageGradientV(200, 200, RED, BLUE);
    SetWindowIcon(gradientImg);
    texture = LoadTextureFromImage(gradientImg);
    UnloadImage(gradientImg);

    Image equationImage = LoadImage("resources/eq.png");
    equationT = LoadTextureFromImage(equationImage);
    UnloadImage(equationImage);

    mapPosition = (Vector3){ -16.0f, 0.0f, -16.0f };

    model = NewModel(&texture, a, 0, 50);
    mainModel = &model;

    loadFonts();
}