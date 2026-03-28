#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" // Gives you buttons, progress bars, panels, etc.

void start_gui() {
    // Open a real 800x600 window
    InitWindow(800, 600, "Bitcamp Blockchain Node");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // Draw a Title
        DrawText("Bitcamp Node Dashboard", 30, 30, 30, DARKGRAY);

        // Draw a Button. If clicked, toggle the miner!
        if (GuiButton((Rectangle){ 30, 100, 200, 40 }, "TOGGLE MINING")) {
            // Call your miner thread function here
            miner_toggle(); 
        }

        // Draw Live Stats
        DrawText(TextFormat("Hashrate: %.2f MH/s", get_hashrate()), 30, 170, 20, MAROON);
        DrawText(TextFormat("Block Height: %d", get_block_height()), 30, 210, 20, DARKGREEN);

        EndDrawing();
    }

    CloseWindow();
}
