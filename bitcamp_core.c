#include "bitcamp.h"
#include "sha256t.h"
#include "sha256.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Helper: big-endian 256-bit compare */
static int cmp256(const uint8_t a[32], const uint8_t b[32]) {
    for (int i = 0; i < 32; i++) {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    return 0;
}

/* Simple hash-based committee selection using SHA256 */
static uint32_t select_one_staker(const bc_chain *c, const uint8_t *seed, size_t seed_len, int idx) {
    uint8_t buf[96];
    memcpy(buf, seed, seed_len);
    uint32_t idx_be = (uint32_t)idx;
    idx_be = ((idx_be & 0xff000000) >> 24) | ((idx_be & 0x00ff0000) >> 8) |
             ((idx_be & 0x0000ff00) << 8) | ((idx_be & 0x000000ff) << 24);
    memcpy(buf + seed_len, &idx_be, 4);

    uint8_t hash[32];
    sha256(buf, seed_len + 4, hash);

    uint32_t total_stake = 0;
    for (size_t i = 0; i < BC_MAX_ACCOUNTS; i++) {
        total_stake += c->accounts[i].staked;
    }
    if (total_stake == 0) return 0;

    uint32_t val = (uint32_t)(hash[0]) << 24 |
                   (uint32_t)(hash[1]) << 16 |
                   (uint32_t)(hash[2]) << 8 |
                   (uint32_t)(hash[3]);

    uint32_t offset = val % total_stake;
    uint32_t accum = 0;
    for (size_t i = 0; i < BC_MAX_ACCOUNTS; i++) {
        accum += c->accounts[i].staked;
        if (accum > offset) return (uint32_t)i;
    }
    return 0;
}

void bc_select_committee(const bc_chain *c, bc_block *b) {
    memset(b->committee, 0, sizeof(b->committee));

    if (b->height == 0) return;

    const bc_block *prev = &c->blocks[b->height - 1];
    for (uint32_t i = 0; i < BC_COMMITTEE_SIZE; i++) {
        uint32_t staker = select_one_staker(c, prev->hash, 32, i);
        b->committee[i] = staker;
    }
    b->stake_approvals = 0;
}

void bc_init_chain(bc_chain *c) {
    memset(c, 0, sizeof(*c));
}

int bc_add_genesis(bc_chain *c, uint32_t timestamp) {
    if (c->count != 0) return 0;
    bc_block *g = &c->blocks[0];
    memset(g, 0, sizeof(*g));
    g->version = 1;
    g->height = 0;
    g->timestamp = timestamp;

    uint8_t target[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    g->bits = bc_target_to_bits(target);

    bc_calc_merkle_root(g, g->merkle_root);
    bc_calc_block_hash(g);
    c->count = 1;

    /* Pre-fund test accounts with initial stake */
    for (int i = 0; i < 5 && i < BC_MAX_ACCOUNTS; i++) {
        c->accounts[i].balance = 1000000;
        c->accounts[i].staked = 50000;
        c->accounts[i].nonce = 0;
    }

    return 1;
}

void bc_calc_merkle_root(const bc_block *b, uint8_t out[32]) {
    uint8_t buf[4096];
    size_t pos = 0;
    for (uint32_t i = 0; i < b->tx_count && pos + sizeof(bc_tx) <= sizeof(buf); i++) {
        memcpy(buf + pos, &b->txs[i], sizeof(bc_tx));
        pos += sizeof(bc_tx);
    }
    if (pos == 0) {
        memset(out, 0, 32);
    } else {
        sha256(buf, pos, out);
    }
}

void bc_serialize_header(const bc_block *b, uint8_t out[80]) {
    memset(out, 0, 80);
    uint32_t version_le = b->version;
    uint32_t height_le = b->height;
    uint32_t timestamp_le = b->timestamp;
    uint32_t bits_le = b->bits;

    memcpy(out + 0,  &version_le,   4);
    memcpy(out + 4,  &height_le,    4);
    memcpy(out + 8,  b->prev_hash,  32);
    memcpy(out + 40, b->merkle_root,32);
    memcpy(out + 72, &timestamp_le, 4);
    memcpy(out + 76, &bits_le,      4);
}

void bc_calc_block_hash(bc_block *b) {
    uint8_t header[80];
    bc_serialize_header(b, header);
    memcpy(header + 76, &b->nonce, 4);
    sha256t_hash(header, 80, b->hash);
}

void bc_bits_to_target(uint32_t bits, uint8_t out[32]) {
    memset(out, 0, 32);
    uint32_t exp = bits >> 24;
    uint32_t mant = bits & 0x007fffffU;
    if (exp <= 3) {
        mant >>= 8*(3-exp);
        out[0] = (mant >> 16) & 0xff;
        out[1] = (mant >> 8) & 0xff;
        out[2] = mant & 0xff;
    } else {
        int i = exp - 3;
        if (i + 2 < 32) {
            out[i+0] = (mant >> 16) & 0xff;
            out[i+1] = (mant >> 8) & 0xff;
            out[i+2] = mant & 0xff;
        }
    }
}

uint32_t bc_target_to_bits(const uint8_t target[32]) {
    int i = 0;
    while (i < 32 && target[i] == 0) i++;
    uint32_t exp = (i < 32) ? (32 - i + 3) : 0;
    uint32_t mant = 0;
    if (i < 32) {
        mant |= (uint32_t)target[i] << 16;
        if (i+1 < 32) mant |= (uint32_t)target[i+1] << 8;
        if (i+2 < 32) mant |= (uint32_t)target[i+2];
    }
    return (exp << 24) | mant;
}

int bc_hash_meets_target(const uint8_t hash[32], const uint8_t target[32]) {
    return cmp256(hash, target) <= 0;
}

void bc_retarget(const bc_chain *c, bc_block *new_block, uint32_t now_ts) {
    if (c->count < BC_ADJUST_WINDOW) {
        new_block->bits = c->blocks[c->count - 1].bits;
        return;
    }
    const bc_block *last  = &c->blocks[c->count - 1];
    const bc_block *first = &c->blocks[c->count - BC_ADJUST_WINDOW];

    uint32_t actual = last->timestamp - first->timestamp;
    uint32_t expected = BC_TARGET_SPACING * BC_ADJUST_WINDOW;

    if (actual < expected / (uint32_t)BC_MAX_ADJUST_FACTOR)
        actual = expected / (uint32_t)BC_MAX_ADJUST_FACTOR;
    if (actual > expected * (uint32_t)BC_MAX_ADJUST_FACTOR)
        actual = expected * (uint32_t)BC_MAX_ADJUST_FACTOR;

    double factor = (double)actual / (double)expected;

    uint8_t old_target[32], new_target[32];
    bc_bits_to_target(last->bits, old_target);

    double leading = (double)old_target[0] * 65536.0 +
                     (double)old_target[1] * 256.0 +
                     (double)old_target[2];
    leading *= factor;
    if (leading > 0xffffff) leading = 0xffffff;
    if (leading < 1.0) leading = 1.0;

    uint32_t mant = (uint32_t)leading;
    memset(new_target, 0, 32);
    new_target[0] = (mant >> 16) & 0xff;
    new_target[1] = (mant >> 8) & 0xff;
    new_target[2] = mant & 0xff;

    new_block->bits = bc_target_to_bits(new_target);
    (void)now_ts;
}

static int ensure_account_index(uint32_t idx) {
    return idx < BC_MAX_ACCOUNTS;
}

int bc_validate_tx(const bc_chain *c, const bc_block *b,
                   const bc_tx *tx, char *err, size_t errlen) {
    (void)b;
    if (!ensure_account_index(tx->from)) {
        snprintf(err, errlen, "bad from");
        return 0;
    }
    if (!ensure_account_index(tx->to)) {
        snprintf(err, errlen, "bad to");
        return 0;
    }
    const bc_account *from = &c->accounts[tx->from];
    if (tx->nonce != from->nonce) {
        snprintf(err, errlen, "bad nonce");
        return 0;
    }
    if (tx->gas_limit * tx->gas_price > from->balance) {
        snprintf(err, errlen, "insufficient balance for gas");
        return 0;
    }
    if (tx->type == BC_TX_TRANSFER && tx->amount > from->balance) {
        snprintf(err, errlen, "insufficient balance for value");
        return 0;
    }
    return 1;
}

int bc_apply_tx_with_gas(bc_chain *c, bc_block *b,
                         const bc_tx *tx, uint64_t *gas_used,
                         char *err, size_t errlen) {
    bc_account *from = &c->accounts[tx->from];
    bc_account *to   = &c->accounts[tx->to];
    bc_account *miner= &c->accounts[b->miner];

    uint64_t intrinsic = BC_TX_GAS_BASE;
    intrinsic += BC_TX_GAS_PER_BYTE * sizeof(*tx);
    if (tx->gas_limit < intrinsic) {
        snprintf(err, errlen, "gas limit too low");
        return 0;
    }

    uint64_t max_fee = tx->gas_limit * tx->gas_price;
    if (from->balance < max_fee) {
        snprintf(err, errlen, "cannot prepay gas");
        return 0;
    }

    from->balance -= max_fee;

    switch (tx->type) {
    case BC_TX_TRANSFER:
        if (from->balance < tx->amount) {
            snprintf(err, errlen, "insufficient funds");
            from->balance += max_fee;
            return 0;
        }
        from->balance -= tx->amount;
        to->balance   += tx->amount;
        break;
    case BC_TX_STAKE:
        if (from->balance < tx->amount) {
            snprintf(err, errlen, "insufficient for stake");
            from->balance += max_fee;
            return 0;
        }
        from->balance -= tx->amount;
        from->staked  += tx->amount;
        from->stake_lock_height = b->height + 10;
        break;
    case BC_TX_UNSTAKE:
        if (from->staked < tx->amount ||
            b->height < from->stake_lock_height) {
            snprintf(err, errlen, "cannot unstake yet");
            from->balance += max_fee;
            return 0;
        }
        from->staked  -= tx->amount;
        from->balance += tx->amount;
        break;
    default:
        snprintf(err, errlen, "unknown tx type");
        from->balance += max_fee;
        return 0;
    }

    *gas_used = intrinsic;
    uint64_t fee_used = (*gas_used) * tx->gas_price;
    if (fee_used > max_fee) fee_used = max_fee;

    uint64_t refund = max_fee - fee_used;
    from->balance += refund;
    miner->balance += fee_used;

    ((bc_account*)from)->nonce++;

    return 1;
}

int bc_validate_block(const bc_chain *c, const bc_block *b,
                      char *err, size_t errlen) {
    if (c->count == 0 && b->height != 0) {
        snprintf(err, errlen, "non-genesis with empty chain");
        return 0;
    }
    if (b->height != c->count) {
        snprintf(err, errlen, "height mismatch");
        return 0;
    }
    if (b->height > 0) {
        const bc_block *prev = &c->blocks[b->height - 1];
        if (memcmp(b->prev_hash, prev->hash, 32) != 0) {
            snprintf(err, errlen, "prev hash mismatch");
            return 0;
        }
        if (b->timestamp <= prev->timestamp) {
            snprintf(err, errlen, "non-monotonic timestamp");
            return 0;
        }
    }

    uint8_t target[32];
    bc_bits_to_target(b->bits, target);

    if (!bc_hash_meets_target(b->hash, target)) {
        snprintf(err, errlen, "hash above target");
        return 0;
    }

    uint64_t block_gas_used = 0;
    for (uint32_t i = 0; i < b->tx_count; i++) {
        const bc_tx *tx = &b->txs[i];
        if (!bc_validate_tx(c, b, tx, err, errlen)) return 0;
        block_gas_used += BC_TX_GAS_BASE;
        if (block_gas_used > BC_BLOCK_GAS_LIMIT) {
            snprintf(err, errlen, "block gas limit exceeded");
            return 0;
        }
    }

    return 1;
}

int bc_apply_block(bc_chain *c, const bc_block *b_in, char *err, size_t errlen) {
    if (c->count >= BC_MAX_BLOCKS) {
        snprintf(err, errlen, "chain full");
        return 0;
    }

    bc_block b = *b_in;
    if (!bc_validate_block(c, &b, err, errlen)) return 0;

    bc_chain tmp = *c;
    uint64_t total_gas = 0;
    for (uint32_t i = 0; i < b.tx_count; i++) {
        uint64_t gu = 0;
        if (!bc_apply_tx_with_gas(&tmp, &b, &b.txs[i], &gu, err, errlen))
            return 0;
        total_gas += gu;
    }

    tmp.blocks[tmp.count] = b;
    tmp.count++;
    *c = tmp;
    (void)total_gas;
    return 1;
}

int bc_verify_chain(const bc_chain *c, char *err, size_t errlen) {
    bc_chain scratch;
    bc_init_chain(&scratch);

    for (size_t i = 0; i < c->count; i++) {
        if (i == 0) {
            scratch.blocks[0] = c->blocks[0];
            scratch.count = 1;
            continue;
        }
        if (!bc_apply_block(&scratch, &c->blocks[i], err, errlen))
            return 0;
    }
    return 1;
}

bc_stats_snapshot bc_get_snapshot(const bc_chain *c) {
    bc_stats_snapshot s;
    if (c->count == 0) {
        memset(&s, 0, sizeof(s));
        return s;
    }

    const bc_block *tip = &c->blocks[c->count - 1];
    s.height = tip->height;
    s.timestamp = tip->timestamp;
    memcpy(s.tip_hash_prefix, tip->hash, 4);
    s.bits = tip->bits;
    s.committee_sz = BC_COMMITTEE_SIZE;
    s.approvals = tip->stake_approvals;

    return s;
}
