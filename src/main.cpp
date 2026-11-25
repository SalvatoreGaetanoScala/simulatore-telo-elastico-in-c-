#include "raylib.h"
#include "model/SimulationModel.hpp"
#include "view/IsometricView.hpp"
#include "controller/SimulationController.hpp"

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Simulazione telo esastico");
    SetTargetFPS(60);

    // Griglia 34x34 con spaziatura 10.5f: maglia fitta che avvolge le sfere senza farle passare attraverso
    SimulationModel model(34, 34, 10.5f);
    IsometricView view(screenWidth, screenHeight);
    SimulationController controller(model, view);

    model.spawnSphere(0.0f, 220.0f, 0.0f);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.033f)
            dt = 0.033f;

        controller.update(dt);
        view.render(model);
    }

    CloseWindow();
    return 0;
}
