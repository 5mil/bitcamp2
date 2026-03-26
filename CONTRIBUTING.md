# Contributing to Bitcamp

We welcome contributions! Here's how:

## Setup

```bash
git clone https://github.com/yourusername/bitcamp.git
cd bitcamp
make clean && make
./bitcamp
```

## Making Changes

1. Create a branch: `git checkout -b feature/my-feature`
2. Edit code
3. Test: `make clean && make && ./bitcamp`
4. Commit with clear messages
5. Push and open a PR

## Code Style

- C11, POSIX-compliant
- 4-space indentation
- No trailing whitespace
- Keep functions under 100 lines when possible
- Document public APIs in headers

## Testing

Before submitting:
1. Build without warnings: `make`
2. Run chain verification: Press `v` in the UI
3. Test on at least one platform (Linux, macOS, or Windows)

## Areas for Contribution

- [ ] Peer networking (P2P sync)
- [ ] Persistent state (RocksDB integration)
- [ ] Key management & signing
- [ ] Web dashboard (WebSocket + React)
- [ ] Mobile port (Android NDK)
- [ ] Documentation improvements
- [ ] Performance optimizations

## Questions?

Open an issue or discussion. We're here to help!
