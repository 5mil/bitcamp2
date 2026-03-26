# Bitcamp v1 – Minimal PoW/PoS Blockchain

A complete, single-node in-process blockchain implementation with SHA256T Proof-of-Work,
staking committee selection, and gas metering.

## Features

- **SHA256T PoW**: Triple SHA256 hashing with difficulty retargeting
- **Staking Committee**: Weighted random selection of validators per block
- **Gas Metering**: Per-tx gas accounting with refunds
- **Difficulty Adjustment**: 30-second target, 20-block windows, 4× clamp
- **ncurses UI**: Real-time hashrate, height, and chain status
- **Pure C**: Portable POSIX implementation, no dependencies except ncurses and pthreads

## Building

**Requirements:**
- GCC or Clang
- libncurses5-dev (or ncurses-devel)
- Linux/POSIX

**Build:**
```bash
make
```

**Clean:**
```bash
make clean
```

## Running

```bash
./bitcamp
```

## Controls

- **m** – Start/stop mining
- **v** – Verify entire chain (full validation run)
- **q** – Quit

## Architecture

### Core Components

1. **bitcamp.h / bitcamp_core.c** – Chain state, validation, difficulty, gas, staking
2. **sha256.c/h, sha256t.c/h** – Cryptographic hash functions
3. **platform.c/h** – POSIX threading, time, logging
4. **miner.c** – In-process mining loop
5. **ui.c** – ncurses status display and main loop

### Design Choices

- **Shared validation library**: Node and miner both call the same `bc_validate_block()` and `bc_apply_block()` functions to prevent divergent rules.
- **Atomic snapshots**: UI reads immutable snapshots of chain state, avoiding stale data.
- **Committee per block**: Previous block hash seeds deterministic weighted selection of 3 stakers.
- **Test-mode fallback**: Blocks without committee approvals are accepted but marked as unstaked, keeping the chain from dead-ending during development.
- **Minimal gas model**: Intrinsic cost + per-byte cost, with miner fee collection.

## Transactions

Three transaction types:

1. **BC_TX_TRANSFER** (type 0): Simple fund transfer
2. **BC_TX_STAKE** (type 1): Lock funds for 10 blocks, enter staking committee pool
3. **BC_TX_UNSTAKE** (type 2): Unlock staked funds

## Difficulty

- **Target spacing**: 30 seconds
- **Adjustment window**: 20 blocks
- **Max adjust factor**: 4×
- **Clamp**: Difficulty cannot jump/drop more than 4× per retarget

## Account Model

Each account has:
- Balance
- Staked amount (locked for voting)
- Stake lock height (must wait before unstaking)
- Nonce (tx counter)

Genesis pre-funds 5 accounts with 1M balance and 50k staked each.

## Performance

On Pi 3B or similar:
- ~0.5–1.5 MH/s (varies by architecture)
- Full chain verification under 1s for typical blockcounts

## Porting

To adapt for **Moto Play 2024 with Linux/Android**:
1. Replace `platform.c` pthread calls with equivalent in your target OS.
2. Keep `bitcamp_core.c` and all consensus logic unchanged.
3. UI can switch from ncurses to a simple text loop or custom graphics layer.

The consensus layer is purely computational and OS-agnostic.

## License

Educational reference implementation. Use freely for learning.
