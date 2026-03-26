#ifndef BITCAMP_H
#define BITCAMP_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#define BC_MAX_ACCOUNTS      1024
#define BC_MAX_BLOCKS        10000
#define BC_MAX_TX_PER_BL     256
#define BC_COMMITTEE_SIZE    3
#define BC_COMMITTEE_QUORUM  2

#define BC_TARGET_SPACING      30      /* seconds */
#define BC_ADJUST_WINDOW       20      /* blocks */
#define BC_MAX_ADJUST_FACTOR   4.0

#define BC_BLOCK_GAS_LIMIT     100000
#define BC_TX_GAS_BASE         21000
#define BC_TX_GAS_PER_BYTE     10

typedef enum {
    BC_TX_TRANSFER = 0,
    BC_TX_STAKE    = 1,
    BC_TX_UNSTAKE  = 2
} bc_tx_type;

typedef struct {
    uint64_t balance;
    uint64_t staked;
    uint32_t stake_lock_height;
    uint32_t nonce;
} bc_account;

typedef struct {
    uint8_t  type;
    uint32_t from;
    uint32_t to;
    uint64_t amount;
    uint64_t gas_limit;
    uint64_t gas_price;
    uint32_t nonce;
} bc_tx;

typedef struct {
    uint32_t version;
    uint32_t height;
    uint8_t  prev_hash[32];
    uint32_t timestamp;
    uint32_t bits;
    uint32_t nonce;
    uint32_t miner;
    uint32_t tx_count;
    bc_tx    txs[BC_MAX_TX_PER_BL];

    uint8_t  merkle_root[32];
    uint8_t  hash[32];

    uint32_t committee[BC_COMMITTEE_SIZE];
    uint32_t stake_approvals;
} bc_block;

typedef struct {
    bc_block  blocks[BC_MAX_BLOCKS];
    size_t    count;
    bc_account accounts[BC_MAX_ACCOUNTS];
} bc_chain;

/* Core functions */
void bc_init_chain(bc_chain *c);
int  bc_add_genesis(bc_chain *c, uint32_t timestamp);

void bc_calc_merkle_root(const bc_block *b, uint8_t out[32]);
void bc_serialize_header(const bc_block *b, uint8_t out[80]);
void bc_calc_block_hash(bc_block *b);

void bc_bits_to_target(uint32_t bits, uint8_t out[32]);
uint32_t bc_target_to_bits(const uint8_t target[32]);
int  bc_hash_meets_target(const uint8_t hash[32], const uint8_t target[32]);

void bc_retarget(const bc_chain *c, bc_block *new_block, uint32_t now_ts);

/* Committee selection */
void bc_select_committee(const bc_chain *c, bc_block *b);

/* Validation and application */
int bc_validate_tx(const bc_chain *c, const bc_block *b,
                   const bc_tx *tx, char *err, size_t errlen);
int bc_apply_tx_with_gas(bc_chain *c, bc_block *b,
                         const bc_tx *tx, uint64_t *gas_used,
                         char *err, size_t errlen);

int bc_validate_block(const bc_chain *c, const bc_block *b,
                      char *err, size_t errlen);
int bc_apply_block(bc_chain *c, const bc_block *b, char *err, size_t errlen);

/* Verification helper */
int bc_verify_chain(const bc_chain *c, char *err, size_t errlen);

/* Stats snapshot */
typedef struct {
    uint32_t height;
    uint32_t timestamp;
    uint8_t  tip_hash_prefix[4];
    uint32_t bits;
    uint32_t committee_sz;
    uint32_t approvals;
} bc_stats_snapshot;

bc_stats_snapshot bc_get_snapshot(const bc_chain *c);

#endif
