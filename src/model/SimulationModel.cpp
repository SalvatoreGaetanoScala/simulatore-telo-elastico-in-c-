#include "SimulationModel.hpp"
#include "raymath.h"
#include <algorithm>
#include <cmath>

SimulationModel::SimulationModel(int gridWidth, int gridDepth, float spacing)
    : width(gridWidth), depth(gridDepth), spacing(spacing) {
    setElasticity(0.5f);
    initCloth();
}

void SimulationModel::setElasticity(float value) {
    elasticity = std::clamp(value, 0.0f, 1.0f);
    stiffness = 0.45f - (elasticity * 0.35f);
}

void SimulationModel::tiltGravity(float tiltX, float tiltZ, float dt) {
    float speed = 460.0f * dt;
    gravityVec.x = std::clamp(gravityVec.x + tiltX * speed, -maxHorizontalTilt, maxHorizontalTilt);
    gravityVec.z = std::clamp(gravityVec.z + tiltZ * speed, -maxHorizontalTilt, maxHorizontalTilt);
}

void SimulationModel::initCloth() {
    nodes.clear();
    nodes.resize(width * depth);
    float offsetX = (width - 1) * spacing * 0.5f;
    float offsetZ = (depth - 1) * spacing * 0.5f;
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int idx = z * width + x;
            Vector3 p = { x * spacing - offsetX, 0.0f, z * spacing - offsetZ };
            nodes[idx].pos = p;
            nodes[idx].oldPos = p;
            nodes[idx].restPos = p;
            if (x == 0 || x == width - 1 || z == 0 || z == depth - 1) {
                nodes[idx].pinned = true;
            }
        }
    }
}

void SimulationModel::reset() {
    spheres.clear();
    gravityVec = { 0.0f, -450.0f, 0.0f };
    initCloth();
}

void SimulationModel::spawnSphere(float x, float y, float z) {
    Color palette[] = {
        { 100, 220, 255, 255 },
        { 100, 255, 140, 255 },
        { 255, 100, 120, 255 },
        { 255, 220, 90,  255 }
    };
    Color chosen = palette[GetRandomValue(0, 3)];
    float r = (float)GetRandomValue(16, 22);
    float maxRangeX = (width - 5) * spacing * 0.5f;
    float maxRangeZ = (depth - 5) * spacing * 0.5f;
    float clampedX = std::clamp(x, -maxRangeX, maxRangeX);
    float clampedZ = std::clamp(z, -maxRangeZ, maxRangeZ);
    spheres.push_back({ {clampedX, y, clampedZ}, {0.0f, 0.0f, 0.0f}, r, r * 0.5f, chosen });
}

float SimulationModel::sampleClothHeight(float x, float z) const {
    float offsetX = (width - 1) * spacing * 0.5f;
    float offsetZ = (depth - 1) * spacing * 0.5f;
    float gx = (x + offsetX) / spacing;
    float gz = (z + offsetZ) / spacing;
    int x0 = std::clamp((int)std::floor(gx), 0, width - 2);
    int z0 = std::clamp((int)std::floor(gz), 0, depth - 2);
    int x1 = x0 + 1;
    int z1 = z0 + 1;
    float tx = std::clamp(gx - x0, 0.0f, 1.0f);
    float tz = std::clamp(gz - z0, 0.0f, 1.0f);
    float y00 = nodes[z0 * width + x0].pos.y;
    float y10 = nodes[z0 * width + x1].pos.y;
    float y01 = nodes[z1 * width + x0].pos.y;
    float y11 = nodes[z1 * width + x1].pos.y;
    float top = y00 * (1.0f - tx) + y10 * tx;
    float bottom = y01 * (1.0f - tx) + y11 * tx;
    return top * (1.0f - tz) + bottom * tz;
}

void SimulationModel::update(float dt) {
    if (paused) return;
    float restoreFactor = 0.028f - (elasticity * 0.022f);
    for (auto& node : nodes) {
        if (node.pinned) continue;
        Vector3 vel = Vector3Scale(Vector3Subtract(node.pos, node.oldPos), damping);
        node.oldPos = node.pos;
        Vector3 targetRest = {
            node.restPos.x + (gravityVec.x / maxHorizontalTilt) * (10.0f + elasticity * 18.0f),
            node.restPos.y,
            node.restPos.z + (gravityVec.z / maxHorizontalTilt) * (10.0f + elasticity * 18.0f)
        };
        Vector3 pullBack = Vector3Scale(Vector3Subtract(targetRest, node.pos), restoreFactor);
        node.pos = Vector3Add(node.pos, vel);
        node.pos = Vector3Add(node.pos, pullBack);
    }
    for (auto& s : spheres) {
        s.vel.x += gravityVec.x * dt;
        s.vel.y += gravityVec.y * dt;
        s.vel.z += gravityVec.z * dt;
        s.pos = Vector3Add(s.pos, Vector3Scale(s.vel, dt));
        float limitX = (width - 3) * spacing * 0.5f - s.radius;
        float limitZ = (depth - 3) * spacing * 0.5f - s.radius;
        if (s.pos.x < -limitX) { s.pos.x = -limitX; s.vel.x *= -0.45f; }
        if (s.pos.x >  limitX) { s.pos.x =  limitX; s.vel.x *= -0.45f; }
        if (s.pos.z < -limitZ) { s.pos.z = -limitZ; s.vel.z *= -0.45f; }
        if (s.pos.z >  limitZ) { s.pos.z =  limitZ; s.vel.z *= -0.45f; }
    }
    const int subSteps = 8;
    for (int iter = 0; iter < subSteps; ++iter) {
        solveConstraints();
        handleCollisions();
    }
}

void SimulationModel::solveConstraints() {
    auto satisfy = [&](int idx1, int idx2) {
        ClothNode& n1 = nodes[idx1];
        ClothNode& n2 = nodes[idx2];
        Vector3 delta = Vector3Subtract(n2.pos, n1.pos);
        float dist = Vector3Length(delta);
        if (dist < 0.0001f) return;
        float restDist = Vector3Distance(n1.restPos, n2.restPos);
        float diff = (dist - restDist) / dist;
        Vector3 offset = Vector3Scale(delta, diff * 0.5f * stiffness);
        if (!n1.pinned) n1.pos = Vector3Add(n1.pos, offset);
        if (!n2.pinned) n2.pos = Vector3Subtract(n2.pos, offset);
    };
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int idx = z * width + x;
            if (x < width - 1) satisfy(idx, idx + 1);
            if (z < depth - 1) satisfy(idx, idx + width);
            if (x < width - 1 && z < depth - 1) {
                satisfy(idx, idx + width + 1);
                satisfy(idx + 1, idx + width);
            }
        }
    }
}

void SimulationModel::handleCollisions() {
    float clothYield = 0.45f + (elasticity * 0.45f);
    float spherePush = 1.0f - clothYield;
    for (size_t i = 0; i < spheres.size(); ++i) {
        for (size_t j = i + 1; j < spheres.size(); ++j) {
            Vector3 diff = Vector3Subtract(spheres[j].pos, spheres[i].pos);
            float dist = Vector3Length(diff);
            float minDist = spheres[i].radius + spheres[j].radius;
            if (dist < minDist && dist > 0.0001f) {
                Vector3 n = Vector3Scale(diff, 1.0f / dist);
                float overlap = (minDist - dist) * 0.5f;
                spheres[i].pos = Vector3Subtract(spheres[i].pos, Vector3Scale(n, overlap));
                spheres[j].pos = Vector3Add(spheres[j].pos, Vector3Scale(n, overlap));
            }
        }
    }
    for (auto& s : spheres) {
        float effectiveRadius = s.radius + 2.5f;
        for (auto& node : nodes) {
            Vector3 diff = Vector3Subtract(node.pos, s.pos);
            float dist = Vector3Length(diff);
            if (dist < effectiveRadius && dist > 0.0001f) {
                Vector3 normal = Vector3Scale(diff, 1.0f / dist);
                float penetration = effectiveRadius - dist;
                if (!node.pinned) {
                    node.pos = Vector3Add(node.pos, Vector3Scale(normal, penetration * clothYield));
                }
                s.pos = Vector3Subtract(s.pos, Vector3Scale(normal, penetration * spherePush));
            }
        }
        float clothY = sampleClothHeight(s.pos.x, s.pos.z);
        float minAllowedCenterY = clothY + s.radius * 0.92f;
        if (s.pos.y < minAllowedCenterY) {
            float correction = minAllowedCenterY - s.pos.y;
            s.pos.y = minAllowedCenterY;
            for (auto& node : nodes) {
                if (node.pinned) continue;
                float dx = node.pos.x - s.pos.x;
                float dz = node.pos.z - s.pos.z;
                float horizDistSq = dx * dx + dz * dz;
                float influenceR = s.radius * 1.6f;
                if (horizDistSq < influenceR * influenceR) {
                    float weight = 1.0f - (std::sqrt(horizDistSq) / influenceR);
                    node.pos.y -= correction * weight * (0.25f + elasticity * 0.55f);
                }
            }
            if (s.vel.y < 0.0f) {
                float bounce = 0.55f - (elasticity * 0.25f);
                s.vel.y = -s.vel.y * std::max(0.15f, bounce);
                s.vel.x *= 0.96f;
                s.vel.z *= 0.96f;
            }
        }
    }
}
