# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

This is a small C++ project with no build system (no Makefile, CMake, etc.). The `.vscode/tasks.json` task invokes `gcc` on a single active file, which will not link the SHA256 implementation — use `g++` with both sources:

```bash
g++ -g -o out blockchain_cplusplus.cpp sha256.cpp
./out
```

Running `./out` writes/appends blockchain state to `blockchain_data.txt` in the working directory.

There are no tests, lint config, or CI.

## Architecture

Two translation units:

- `sha256.{h,cpp}` — standalone SHA256 implementation (Stephan Brumme, public). Used only via `SHA256 sha256; sha256(str)`.
- `blockchain_cplusplus.cpp` — single-file blockchain: `TransactionData`, `Block`, `Blockchain`, and `main`.

### Chain / mining model (non-obvious)

The `Blockchain` ctor calls `createGenesisBlock()`, which pushes an *empty* block at index 0 that is treated as the "current open block" for incoming transactions — `submitTransaction` appends to `chain.back()`, not to a separate mempool. This means the latest block in `chain` is always the mutable pending block, and `getLatestBlock()` returns a pending block until mining completes.

`mineCurrentBlock()`:
1. Appends a `NETWORK -> miner` reward tx of `1.0` to the pending `chain.back()`.
2. PoW-iterates `nonce` until `hash` starts with `difficulty` zeros.
3. Calls `updateBalances()` on the just-mined block.
4. Pushes a **new empty block** onto `chain` as the next pending block, then calls `adjustDifficulty()`.

Consequence: after mining, `chain.back()` is empty again. `main()` currently calls `getLatestBlock()` *before* mining and then `addToChain(blk)` *after* — this re-adds the mined block a second time and double-applies balances. This is a known in-progress issue (see recent commit messages); don't treat `main()` as a correct usage example.

### Difficulty

`difficulty` starts at 4. `adjustDifficulty()` runs after every mine once `chain.size() > DIFFICULTY_ADJUSTMENT_INTERVAL` (5 blocks). It compares actual vs. expected elapsed time (`TARGET_BLOCK_TIME * DIFFICULTY_ADJUSTMENT_INTERVAL` = 50s) with hysteresis: bumps up if actual < expected/2, down if actual > expected*2, floored at 1.

### Balances

`balances` is a plain `unordered_map<string, double>` updated only inside `updateBalances()`, called from `mineCurrentBlock` and `addToChain`. `submitTransaction` validates funds as `balances[sender] - pending >= amount + TRANSACTION_FEE`, where `pending` is the sum of already-submitted-but-unmined txs from the same sender in `chain.back()`. Note: `TRANSACTION_FEE` is checked against but never actually deducted or paid to the miner.

### Hashing

`calculateHash` concatenates `index + previousHash + transactions + timestamp + nonce` (no difficulty, no hash) and SHA256s it. `transaction_to_str` uses the format `sender->recipient:amount;` (e.g. `Alice->Bob:10.000000;`). Changing this format invalidates all previously mined hashes.

### Persistence

`saveToFile` opens in **append** mode and dumps the entire chain each call, so `blockchain_data.txt` accumulates the chain across runs rather than being a snapshot. There is no loader — the file is write-only from the program's perspective.

## Status / direction

Per recent commit messages ("fix transaction logic ready to start working backend"), the project is moving toward a client/server split. The comment at `mineCurrentBlock`'s declaration ("this needs to go to client, not blockchain") flags that mining and transaction submission are expected to separate from the chain/ledger object.
