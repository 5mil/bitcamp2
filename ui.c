#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include "raylib.h"
#include "bitcamp.h"
#include "platform.h"

// Raygui implementation in this translation unit
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

// Global blockchain + miner state
bc_chain g_chain;
atomic_int g_mining = 0;
atomic_uint_fast64_t g_hashes = 0;

// Miner entry (implemented in miner.c)
extern void miner_start(uint32_t miner_idx);

// GUI-local state
static uint64_t last_time_ms = 0;
static uint64_t last_hashes = 0;
static double current_hashrate = 0.0;

// Helper to compute hashrate once per second
static void UpdateHashrate(void) {
    uint64_t now = platform_now_ms();
    if (now - last_time_ms >= 1000) {
        uint64_t h = atomic_load(&g_hashes);
        double dt = (double)(now - last_time_ms) / 1000.0;
        if (dt > 0.0) {
            current_hashrate = (double)(h - last_hashes) / dt;
        }
        last_hashes = h;
        last_time_ms = now;
    }
}

static void RunGUI(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Bitcamp Blockchain Node");
    SetTargetFPS(60);

    GuiLoadStyleDefault();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    last_time_ms = platform_now_ms();

    while (!WindowShouldClose()) {
        UpdateHashrate();
        bc_stats_snapshot snap = bc_get_snapshot(&g_chain);
        bool is_mining = (atomic_load(&g_mining) != 0);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Header
        DrawRectangle(0, 0, screenWidth, 80, DARKGRAY);
        DrawText("BITCAMP NODE DASHBOARD", 30, 25, 30, WHITE);

        // Miner controls
        GuiGroupBox((Rectangle){ 30, 110, 350, 200 }, "Miner Controls");

        if (is_mining) {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(MAROON));
            if (GuiButton((Rectangle){ 60, 150, 290, 50 }, "STOP MINING")) {
                atomic_store(&g_mining, 0);
            }
        } else {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(DARKGREEN));
            if (GuiButton((Rectangle){ 60, 150, 290, 50 }, "START MINING")) {
                miner_start(0); // local miner index 0
            }
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));

        DrawText("Current Hashrate:", 60, 230, 20, DARKGRAY);
        DrawText(TextFormat("%.2f H/s", current_hashrate),
                 60, 260, 30, is_mining ? DARKGREEN : GRAY);

        // Network stats
        GuiGroupBox((Rectangle){ 420, 110, 350, 200 }, "Network Stats");

        DrawText("Block Height:", 450, 150, 20, DARKGRAY);
        DrawText(TextFormat("%u", snap.height), 450, 180, 40, BLACK);

        DrawText("Committee Approvals:", 450, 240, 20, DARKGRAY);
        DrawText(TextFormat("%u / %u", snap.approvals, snap.committee_sz),
                 450, 270, 30, BLUE);

        DrawText("Press ESC or close window to exit.", 30, 560, 15, GRAY);

        EndDrawing();
    }

    atomic_store(&g_mining, 0);
    CloseWindow();
}

// Program entry point: single main, no WinMain, no windows.h
int main(void) {
    // Initialize blockchain and add genesis block
    bc_init_chain(&g_chain);
    uint32_t ts = (uint32_t)(platform_now_ms() / 1000);
    bc_add_genesis(&g_chain, ts);

    RunGUI();
    return 0;
}
