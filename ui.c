#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include "raylib.h"
#include "bitcamp.h"
#include "platform.h" // For platform_now_ms()

// Temporarily disable the unused-parameter warning for raygui
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

// --- References to the real variables in your project ---
extern bc_chain g_chain;
extern atomic_int g_mining;
extern atomic_uint_fast64_t g_hashes;

// The function in your miner.c
extern void miner_start(uint32_t miner_idx); 

// --- Local state for GUI ---
static uint64_t last_time = 0;
static uint64_t last_hashes = 0;
static double current_hashrate = 0.0;

void start_gui() {
    // 1. Initialize the Window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Bitcamp v1 - Node Dashboard");

    // Load a slightly larger font for UI elements
    GuiLoadStyleDefault();
    Font font = GetFontDefault();
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    SetTargetFPS(60);
    
    last_time = platform_now_ms();

    // 2. Main GUI Loop
    while (!WindowShouldClose()) {
        
        // Calculate real hashrate
        uint64_t now = platform_now_ms();
        if (now - last_time >= 1000) {
            uint64_t h = atomic_load(&g_hashes);
            current_hashrate = (double)(h - last_hashes) / ((now - last_time) / 1000.0);
            last_hashes = h;
            last_time = now;
        }

        // Get safe snapshot of the blockchain
        bc_stats_snapshot snap = bc_get_snapshot(&g_chain);
        bool is_mining = atomic_load(&g_mining) != 0;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Header
        DrawRectangle(0, 0, screenWidth, 80, DARKGRAY);
        DrawText("BITCAMP BLOCKCHAIN", 30, 25, 30, WHITE);

        // --- Mining Controls Panel ---
        GuiGroupBox((Rectangle){ 30, 110, 350, 200 }, "Miner Controls");

        // Toggle Button (Changes color based on state)
        if (is_mining) {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(MAROON));
            if (GuiButton((Rectangle){ 60, 150, 290, 50 }, "STOP MINING")) {
                atomic_store(&g_mining, 0); // Stops the miner loop
            }
        } else {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(DARKGREEN));
            if (GuiButton((Rectangle){ 60, 150, 290, 50 }, "START MINING")) {
                // Assuming miner index 0 for the local node
                miner_start(0); 
            }
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY)); // reset

        // Draw Hashrate
        DrawText("Current Hashrate:", 60, 230, 20, DARKGRAY);
        DrawText(TextFormat("%.2f H/s", current_hashrate), 60, 260, 30, is_mining ? DARKGREEN : GRAY);

        // --- Blockchain Stats Panel ---
        GuiGroupBox((Rectangle){ 420, 110, 350, 200 }, "Network Stats");

        DrawText("Block Height:", 450, 150, 20, DARKGRAY);
        DrawText(TextFormat("%d", snap.height), 450, 180, 40, BLACK);

        DrawText("Committee Approvals:", 450, 240, 20, DARKGRAY);
        DrawText(TextFormat("%d / %d", snap.approvals, snap.committee_sz), 450, 270, 30, BLUE);

        // Footer
        DrawText("Press ESC or close window to exit.", 30, 560, 15, GRAY);

        EndDrawing();
    }

    // 3. Clean up
    // Stop mining on exit
    atomic_store(&g_mining, 0); 
    CloseWindow();
}

