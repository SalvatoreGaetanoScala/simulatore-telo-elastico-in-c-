#include "IsometricView.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

IsometricView::IsometricView(int screenWidth, int screenHeight)
    : screenW(screenWidth), screenH(screenHeight)
{
    resetCamera();
}

void IsometricView::resetCamera()
{
    panOffset = {0.0f, 0.0f};
    yaw = 0.785398f;
    pitch = 0.615479f;
    zoom = 1.0f;
}

void IsometricView::panBy(Vector2 delta)
{
    panOffset.x += delta.x;
    panOffset.y += delta.y;
}

void IsometricView::rotateBy(float deltaYawRad, float deltaPitchRad)
{
    yaw += deltaYawRad;
    pitch += deltaPitchRad;
}

void IsometricView::zoomBy(float deltaZoom)
{
    zoom = std::clamp(zoom + deltaZoom, 0.25f, 4.0f);
}

Rectangle IsometricView::getHelpButtonBounds() const
{
    float w = 156.0f;
    float h = 40.0f;
    return {(screenW - w) * 0.5f, 18.0f, w, h};
}

Rectangle IsometricView::getElasticitySliderBounds() const
{
    return {36.0f, 186.0f, 248.0f, 14.0f};
}

float IsometricView::valueFromSliderMouseX(float mouseX) const
{
    Rectangle bar = getElasticitySliderBounds();
    float t = (mouseX - bar.x) / bar.width;
    return std::clamp(t, 0.0f, 1.0f);
}

float IsometricView::depthKey(Vector3 pos) const
{
    float cosY = std::cos(yaw);
    float sinY = std::sin(yaw);
    float cosP = std::cos(pitch);
    float sinP = std::sin(pitch);

    float rx = pos.x * cosY - pos.z * sinY;
    float rz = pos.x * sinY + pos.z * cosY;
    return rz * cosP + pos.y * sinP + rx * 0.001f;
}

Vector2 IsometricView::worldToScreen(Vector3 pos) const
{
    float cosY = std::cos(yaw);
    float sinY = std::sin(yaw);
    float cosP = std::cos(pitch);
    float sinP = std::sin(pitch);

    float rx = pos.x * cosY - pos.z * sinY;
    float rz = pos.x * sinY + pos.z * cosY;

    float screenX = rx * zoom;
    float screenY = (rz * sinP - pos.y * cosP) * zoom;

    return {
        screenW * 0.5f + panOffset.x + screenX,
        screenH * 0.55f + panOffset.y + screenY};
}

void IsometricView::render(const SimulationModel &model)
{
    BeginDrawing();
    ClearBackground({10, 12, 24, 255});

    drawCloth(model);
    drawSpheres(model);
    drawUI(model);

    if (showHelp)
    {
        drawHelpModal();
    }

    EndDrawing();
}

void IsometricView::drawCloth(const SimulationModel &model) const
{
    const auto &nodes = model.getNodes();
    int w = model.getGridWidth();
    int d = model.getGridDepth();

    int stress = (int)(model.getElasticity() * 90.0f);
    Color quadColor = {
        (unsigned char)(20 + stress),
        (unsigned char)(60 + stress / 2),
        (unsigned char)(140 + stress / 2),
        170};
    Color lineCol = {100, 180, 255, 110};

    for (int z = 0; z < d - 1; ++z)
    {
        for (int x = 0; x < w - 1; ++x)
        {
            int i0 = z * w + x;
            int i1 = z * w + (x + 1);
            int i2 = (z + 1) * w + (x + 1);
            int i3 = (z + 1) * w + x;

            Vector2 p0 = worldToScreen(nodes[i0].pos);
            Vector2 p1 = worldToScreen(nodes[i1].pos);
            Vector2 p2 = worldToScreen(nodes[i2].pos);
            Vector2 p3 = worldToScreen(nodes[i3].pos);

            DrawTriangle(p0, p3, p2, quadColor);
            DrawTriangle(p0, p2, p1, quadColor);

            if (showGrid)
            {
                DrawLineV(p0, p1, lineCol);
                DrawLineV(p0, p3, lineCol);
            }
        }
    }
}

void IsometricView::drawSpheres(const SimulationModel &model) const
{
    std::vector<Sphere> sortedSpheres = model.getSpheres();
    std::sort(sortedSpheres.begin(), sortedSpheres.end(), [this](const Sphere &a, const Sphere &b)
              { return depthKey(a.pos) < depthKey(b.pos); });

    for (const auto &s : sortedSpheres)
    {
        Vector2 screenPos = worldToScreen(s.pos);
        float scaledRadius = s.radius * zoom;

        float shadowY = std::min(0.0f, s.pos.y - s.radius * 0.85f);
        Vector2 shadowPos = worldToScreen({s.pos.x, shadowY, s.pos.z});
        float heightAboveCloth = std::max(0.0f, s.pos.y - s.radius - shadowY);
        float shadowScale = std::clamp(1.0f - (heightAboveCloth / 320.0f), 0.25f, 1.0f) * zoom;
        DrawEllipse((int)shadowPos.x, (int)shadowPos.y, s.radius * shadowScale, s.radius * 0.5f * shadowScale, {0, 0, 0, 110});

        DrawCircleGradient((int)screenPos.x, (int)screenPos.y, scaledRadius, s.color, {15, 25, 60, 255});
        DrawCircle((int)(screenPos.x - scaledRadius * 0.3f), (int)(screenPos.y - scaledRadius * 0.3f), scaledRadius * 0.25f, {255, 255, 255, 200});
        DrawCircleLines((int)screenPos.x, (int)screenPos.y, scaledRadius, {220, 240, 255, 180});
    }
}

void IsometricView::drawFF7Window(Rectangle bounds, const char *title) const
{
    float roundness = 0.16f;
    int segments = 12;

    Rectangle shadow = {bounds.x + 4, bounds.y + 4, bounds.width, bounds.height};
    DrawRectangleRounded(shadow, roundness, segments, {0, 0, 0, 150});

    DrawRectangleRounded(bounds, roundness, segments, {0, 20, 110, 235});
    Rectangle innerGradient = {bounds.x + 2, bounds.y + 2, bounds.width - 4, bounds.height * 0.5f};
    DrawRectangleRounded(innerGradient, roundness, segments, {25, 85, 195, 120});

    DrawRectangleRoundedLines(bounds, roundness, segments, 3.0f, {60, 70, 90, 255});
    Rectangle innerBorder = {bounds.x + 2, bounds.y + 2, bounds.width - 4, bounds.height - 4};
    DrawRectangleRoundedLines(innerBorder, roundness, segments, 2.0f, {235, 240, 255, 255});

    if (title != nullptr)
    {
        DrawText(title, (int)bounds.x + 18, (int)bounds.y + 12, 18, {120, 230, 255, 255});
    }
}

void IsometricView::drawUI(const SimulationModel &model) const
{
    Rectangle helpBtn = getHelpButtonBounds();
    drawFF7Window(helpBtn, nullptr);
    const char *helpText = showHelp ? "[H] CHIUDI HELP" : "[H] HELP";
    int textW = MeasureText(helpText, 17);
    DrawText(helpText, (int)(helpBtn.x + (helpBtn.width - textW) * 0.5f), (int)(helpBtn.y + 11), 17, {120, 230, 255, 255});

    drawFF7Window({20, 20, 290, 224}, "STATUS SIMULAZIONE");

    char buf[96];
    snprintf(buf, sizeof(buf), "Sfere Attive: %zu", model.getSpheres().size());
    DrawText(buf, 38, 48, 16, WHITE);

    snprintf(buf, sizeof(buf), "Stato: %s", model.isPaused() ? "IN PAUSA" : "ATTIVO");
    DrawText(buf, 38, 68, 16, model.isPaused() ? YELLOW : GREEN);

    snprintf(buf, sizeof(buf), "Cam Yaw/Pitch: %.0f / %.0f", yaw * RAD2DEG, pitch * RAD2DEG);
    DrawText(buf, 38, 88, 14, {180, 220, 255, 255});

    snprintf(buf, sizeof(buf), "Pan: %.0f, %.0f | Zoom: %.1fx", panOffset.x, panOffset.y, zoom);
    DrawText(buf, 38, 106, 14, LIGHTGRAY);

    float elast = model.getElasticity();
    snprintf(buf, sizeof(buf), "Elasticita' (Cedimento): %.2f", elast);
    DrawText(buf, 38, 130, 16, {120, 230, 255, 255});
    DrawText("Rigido", 38, 152, 13, LIGHTGRAY);
    DrawText("Default", 142, 152, 13, YELLOW);
    DrawText("Cedevole", 232, 152, 13, LIGHTGRAY);

    Rectangle bar = getElasticitySliderBounds();
    DrawRectangleRounded(bar, 0.5f, 8, {10, 20, 55, 255});
    DrawRectangleRoundedLines(bar, 0.5f, 8, 2.0f, {200, 220, 255, 255});

    float centerX = bar.x + bar.width * 0.5f;
    DrawLine((int)centerX, (int)bar.y - 3, (int)centerX, (int)(bar.y + bar.height + 3), YELLOW);

    float knobX = bar.x + elast * bar.width;
    float knobY = bar.y + bar.height * 0.5f;
    DrawCircle((int)knobX, (int)knobY, 9.0f, {120, 230, 255, 255});
    DrawCircleLines((int)knobX, (int)knobY, 9.0f, WHITE);

    float panelW = 390.0f;
    float panelH = 128.0f;
    float panelX = (float)screenW - panelW - 20.0f;
    float panelY = (float)screenH - panelH - 20.0f;
    drawFF7Window({panelX, panelY, panelW, panelH}, "COMANDI RAPIDI");
    DrawText("[WASD / Frecce] Visuale + Movimento Sfere", (int)panelX + 18, (int)panelY + 42, 16, WHITE);
    DrawText("[F / INVIO] Lancia Sfera", (int)panelX + 18, (int)panelY + 64, 16, WHITE);
    DrawText("[Click SX + Trascina] Sposta  |  [Rotella] Zoom", (int)panelX + 18, (int)panelY + 86, 15, {180, 220, 255, 255});
    DrawText("[H] Help   [SPAZIO] Pausa   [R] Reset", (int)panelX + 18, (int)panelY + 106, 15, LIGHTGRAY);
}

void IsometricView::drawHelpModal() const
{
    float w = 700.0f;
    float h = 385.0f;
    if (w > screenW - 40.0f)
        w = screenW - 40.0f;
    if (h > screenH - 40.0f)
        h = screenH - 40.0f;

    Rectangle box = {(screenW - w) * 0.5f, (screenH - h) * 0.5f, w, h};

    DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, 140});
    drawFF7Window(box, "GUIDA COMANDI - HELP");

    const char *lines[] = {
        "WASD / Frecce        : Coordina la visuale E inclina/muove le sfere lungo il telo",
        "F / INVIO            : Fa cadere una nuova sfera sul telo elastico",
        "Click SX + Trascina  : Sposta la visuale sullo schermo",
        "Rotella Mouse        : Zoom avanti / indietro",
        "Barra Elasticita'    : Centro (0.50) = default | Destra = affonda di piu'",
        "SPAZIO / R / G       : Pausa/Riprendi  |  Reset  |  Mostra/Nascondi griglia",
        "H                    : Chiude questa finestra di aiuto"};

    int startX = (int)box.x + 26;
    int startY = (int)box.y + 52;
    int lineSpacing = 40;
    int fontSize = 16;

    for (int i = 0; i < 7; ++i)
    {
        Color col = (i == 6) ? Color{120, 230, 255, 255} : WHITE;
        DrawText(lines[i], startX, startY + i * lineSpacing, fontSize, col);
    }
}
