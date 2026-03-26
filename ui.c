#include "bitcamp.h"
#include "platform.h"
#include <ncurses.h>
#include <stdatomic.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

bc_chain g_chain;
atomic_int g_mining = 0;
atomic_uint_fast64_t g_hashes = 0;

void miner_start(uint32_t miner_idx);

static void draw_status(void) {
    bc_stats_snapshot snap = bc_get_snapshot(&g_chain);

    uint64_t hashes = atomic_load(&g_hashes);
    static uint64_t last_hashes = 0;
    static uint64_t last_ts = 0;

    uint64_t now = platform_now_ms();
    double hps = 0.0;
    if (last_ts != 0) {
        double dt = (double)(now - last_ts) / 1000.0;
        uint64_t dh = hashes - last_hashes;
        if (dt > 0.0) hps = (double)dh / dt;
    }
    last_ts = now;
    last_hashes = hashes;

    mvprintw(0, 0, "Bitcamp v1 - Height: %u  Timestamp: %u  Bits: 0x%08x",
             snap.height, snap.timestamp, snap.bits);
    mvprintw(1, 0, "Mining: %s  Hashrate: %.1f H/s  Total hashes: %llu",
             atomic_load(&g_mining) ? "ON " : "OFF",
             hps, (unsigned long long)hashes);
    mvprintw(2, 0, "Tip hash prefix: %02x%02x%02x%02x",
             snap.tip_hash_prefix[0], snap.tip_hash_prefix[1],
             snap.tip_hash_prefix[2], snap.tip_hash_prefix[3]);

    mvprintw(4, 0, "Committee: %u members  Approvals: %u",
             snap.committee_sz, snap.approvals);

    mvprintw(6, 0, "Controls: m=toggle mining, v=verify chain, q=quit");
}

static void handle_key(int ch) {
    if (ch == 'm') {
        if (!atomic_load(&g_mining)) {
            miner_start(1);
        } else {
            atomic_store(&g_mining, 0);
        }
    } else if (ch == 'v') {
        char err[256];
        if (bc_verify_chain(&g_chain, err, sizeof(err))) {
            mvprintw(8, 0, "Chain verification: PASS                                    ");
        } else {
            mvprintw(8, 0, "Chain verification: FAIL - %s", err);
        }
    }
}

int main(void) {
    bc_init_chain(&g_chain);
    uint32_t now = (uint32_t)(platform_now_ms() / 1000);
    bc_add_genesis(&g_chain, now);

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    int running = 1;
    while (running) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q') {
                running = 0;
            } else {
                handle_key(ch);
            }
        }

        erase();
        draw_status();
        refresh();

        platform_sleep_ms(200);
    }

    endwin();
    atomic_store(&g_mining, 0);
    platform_sleep_ms(500);
    return 0;
}
