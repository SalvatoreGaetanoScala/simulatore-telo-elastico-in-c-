#pragma once
#include "../model/SimulationModel.hpp"
#include "../view/IsometricView.hpp"
#include "raylib.h"

class SimulationController
{
public:
    SimulationController(SimulationModel &model, IsometricView &view);

    void handleInput();
    void update(float dt);

private:
    SimulationModel &model;
    IsometricView &view;

    bool draggingSlider = false;
    bool panningView = false;
    Vector2 lastMousePos = {0.0f, 0.0f};
};
