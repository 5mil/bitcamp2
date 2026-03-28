#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include "raylib.h"
#include "bitcamp.h"
#include "platform.h"

// Silence unused-parameter warnings inside raygui
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

// ---- Global blockchain + miner state (definitions) ----
// miner.c declares these as extern; we define them here.
bc_chain g_chain;
atomic_int g_mining = 0;
atomic_uint_fast64_t g_hashes = 0;

// From miner.c
extern void miner_start(uint32_t miner_idx);

// Local GUI state for hashrate calculation
static uint64_t last_time = 0;
static uint64_t last_hashes = 0;
static double current_hashrate = 0.0;

// ---- Core GUI loop ----
static void run_gui(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Bitcamp v1 - Node Dashboard");

    GuiLoadStyleDefault();
    Font font = GetFontDefault();
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    SetTargetFPS(60);

    last_time = platform_now_ms();

    while (!WindowShouldClose()) {
        // Update hashrate once per second
        uint64_t now = platform_now_ms();
        if (now - last_time >= 1000) {
            uint64_t h = atomic_load(&g_hashes);
            current_hashrate = (double)(h - last_hashes) / ((now - last_time) / 1000.0);
            last_hashes = h;
            last_time = now;
        }

        // Snapshot chain stats
        bc_stats_snapshot snap = bc_get_snapshot(&g_chain);
        bool is_mining = atomic_load(&g_mining) != 0;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Header
        DrawRectangle(0, 0, screenWidth, 80, DARKGRAY);
        DrawText("BITCAMP BLOCKCHAIN", 30, 25, 30, WHITE);

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
                miner_start(0);  // local miner index 0
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

// ---- Program entry point(s) ----

// Standard C entry (used on Linux, and we call it from WinMain on Windows)
int main(void) {
    // Initialize chain and create genesis block
    bc_init_chain(&g_chain);
    uint32_t ts = (uint32_t)(platform_now_ms() / 1000);
    bc_add_genesis(&g_chain, ts);

    run_gui();
    return 0;
}

#ifdef _WIN32
// Windows GUI entry point (satisfies the crtexewin/WinMain requirement)
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return main();
}
#endif


