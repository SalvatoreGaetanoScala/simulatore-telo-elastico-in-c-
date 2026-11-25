#pragma once
#include "../model/SimulationModel.hpp"
#include "raylib.h"

class IsometricView
{
public:
    IsometricView(int screenWidth, int screenHeight);

    void render(const SimulationModel &model);
    void toggleGrid() { showGrid = !showGrid; }
    void toggleHelp() { showHelp = !showHelp; }
    bool isHelpVisible() const { return showHelp; }

    void panBy(Vector2 delta);
    void rotateBy(float deltaYawRad, float deltaPitchRad);
    void zoomBy(float deltaZoom);
    void resetCamera();

    Rectangle getHelpButtonBounds() const;
    Rectangle getElasticitySliderBounds() const;
    float valueFromSliderMouseX(float mouseX) const;

private:
    int screenW;
    int screenH;
    bool showGrid = true;
    bool showHelp = false;

    Vector2 panOffset = {0.0f, 0.0f};
    float yaw = 0.785398f;
    float pitch = 0.615479f;
    float zoom = 1.0f;

    Vector2 worldToScreen(Vector3 pos) const;
    float depthKey(Vector3 pos) const;

    void drawFF7Window(Rectangle bounds, const char *title = nullptr) const;
    void drawCloth(const SimulationModel &model) const;
    void drawSpheres(const SimulationModel &model) const;
    void drawUI(const SimulationModel &model) const;
    void drawHelpModal() const;
};
