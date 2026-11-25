#include "SimulationController.hpp"
#include "raylib.h"

SimulationController::SimulationController(SimulationModel &model, IsometricView &view)
    : model(model), view(view) {}

void SimulationController::handleInput()
{
    Vector2 mousePos = GetMousePosition();
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_H))
    {
        view.toggleHelp();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (CheckCollisionPointRec(mousePos, view.getHelpButtonBounds()))
        {
            view.toggleHelp();
            return;
        }

        Rectangle slider = view.getElasticitySliderBounds();
        Rectangle sliderHitArea = {slider.x - 10.0f, slider.y - 12.0f, slider.width + 20.0f, slider.height + 24.0f};

        if (CheckCollisionPointRec(mousePos, sliderHitArea))
        {
            draggingSlider = true;
            model.setElasticity(view.valueFromSliderMouseX(mousePos.x));
        }
        else
        {
            panningView = true;
            lastMousePos = mousePos;
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (draggingSlider)
        {
            model.setElasticity(view.valueFromSliderMouseX(mousePos.x));
        }
        else if (panningView)
        {
            Vector2 delta = {mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y};
            view.panBy(delta);
            lastMousePos = mousePos;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        draggingSlider = false;
        panningView = false;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        view.zoomBy(wheel * 0.15f);
    }

    // WASD e Frecce: coordinano la visuale E inclinano/muovono le sfere lungo il telo
    float tiltX = 0.0f;
    float tiltZ = 0.0f;
    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
    float rotSpeed = 2.4f * dt;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        tiltX -= 1.0f;
        yawDelta -= rotSpeed;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        tiltX += 1.0f;
        yawDelta += rotSpeed;
    }
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        tiltZ -= 1.0f;
        pitchDelta += rotSpeed * 0.85f;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        tiltZ += 1.0f;
        pitchDelta -= rotSpeed * 0.85f;
    }

    if (tiltX != 0.0f || tiltZ != 0.0f)
    {
        model.tiltGravity(tiltX, tiltZ, dt);
    }
    if (yawDelta != 0.0f || pitchDelta != 0.0f)
    {
        view.rotateBy(yawDelta, pitchDelta);
    }

    if (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_ENTER))
    {
        float randX = (float)GetRandomValue(-110, 110);
        float randZ = (float)GetRandomValue(-110, 110);
        model.spawnSphere(randX, 240.0f, randZ);
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        model.togglePause();
    }

    if (IsKeyPressed(KEY_R))
    {
        model.reset();
        view.resetCamera();
    }

    if (IsKeyPressed(KEY_G))
    {
        view.toggleGrid();
    }
}

void SimulationController::update(float dt)
{
    handleInput();
    model.update(dt);
}
