#include "bitcamp.h"
#include "sha256t.h"
#include "platform.h"
#include <string.h>
#include <stdatomic.h>
#include <stdint.h>

extern bc_chain g_chain;
extern atomic_int g_mining;
extern atomic_uint_fast64_t g_hashes;

static void make_candidate(bc_block *b, uint32_t miner_idx) {
    if (g_chain.count == 0) return;
    const bc_block *tip = &g_chain.blocks[g_chain.count - 1];
    memset(b, 0, sizeof(*b));
    b->version = 1;
    b->height  = tip->height + 1;
    memcpy(b->prev_hash, tip->hash, 32);
    b->timestamp = (uint32_t)(platform_now_ms() / 1000);
    b->miner = miner_idx;
    b->tx_count = 0;
    bc_retarget(&g_chain, b, b->timestamp);
    bc_select_committee(&g_chain, b);
}

static void *miner_thread(void *arg) {
    uint32_t miner_idx = (uint32_t)(uintptr_t)arg;
    uint8_t target[32], header[80], hash[32];
    char err[128];

    while (atomic_load(&g_mining)) {
        bc_block b;
        make_candidate(&b, miner_idx);
        bc_bits_to_target(b.bits, target);

        bc_calc_merkle_root(&b, b.merkle_root);
        bc_serialize_header(&b, header);

        for (uint32_t nonce = 0; atomic_load(&g_mining); nonce++) {
            uint32_t nonce_le = nonce;
            memcpy(header + 76, &nonce_le, 4);
            sha256t_hash(header, 80, hash);
            memcpy(b.hash, hash, 32);
            atomic_fetch_add(&g_hashes, 1);

            if (bc_hash_meets_target(hash, target)) {
                b.nonce = nonce;
                if (bc_apply_block(&g_chain, &b, err, sizeof(err))) {
                    platform_log("[miner] found block %u", b.height);
                } else {
                    platform_log("[miner] rejected: %s", err);
                }
                break;
            }
        }

        platform_sleep_ms(10);
    }
    return NULL;
}

void miner_start(uint32_t miner_idx) {
    static pthread_t handle;
    atomic_store(&g_mining, 1);
    platform_thread_start((void**)&handle, miner_thread, (void*)(uintptr_t)miner_idx);
}
