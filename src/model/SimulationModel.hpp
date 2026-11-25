#pragma once
#include "raylib.h"
#include <vector>

struct ClothNode {
    Vector3 pos;
    Vector3 oldPos;
    Vector3 restPos;
    bool pinned = false;
};

struct Sphere {
    Vector3 pos;
    Vector3 vel;
    float radius;
    float mass;
    Color color;
};

class SimulationModel {
public:
    SimulationModel(int gridWidth, int gridDepth, float spacing);

    void update(float dt);
    void spawnSphere(float x, float y, float z);
    void reset();

    const std::vector<ClothNode>& getNodes() const { return nodes; }
    const std::vector<Sphere>& getSpheres() const { return spheres; }
    int getGridWidth() const { return width; }
    int getGridDepth() const { return depth; }
    bool isPaused() const { return paused; }
    void togglePause() { paused = !paused; }

    float getElasticity() const { return elasticity; }
    void setElasticity(float value);

    Vector3 getGravity() const { return gravityVec; }
    void tiltGravity(float tiltX, float tiltZ, float dt);

    float stiffness = 0.25f;
    float damping = 0.985f;

private:
    int width;
    int depth;
    float spacing;
    bool paused = false;
    float elasticity = 0.5f;

    Vector3 gravityVec = { 0.0f, -450.0f, 0.0f };
    float maxHorizontalTilt = 360.0f;

    std::vector<ClothNode> nodes;
    std::vector<Sphere> spheres;

    void initCloth();
    void solveConstraints();
    void handleCollisions();
    float sampleClothHeight(float x, float z) const;
};
