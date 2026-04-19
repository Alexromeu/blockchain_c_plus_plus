# blockchain_c_plus_plus  
**Work in Progress**

A minimal, educational blockchain implementation written in modern C++.  
The goal is clarity: a small, readable codebase that demonstrates the core mechanics behind blockchains without networking, wallets, or distributed consensus.

---

## Status
This project is **actively being developed**.  
Features, structure, and documentation will continue to evolve as the implementation grows.

---

## Features
- **Block structure**
  - Index  
  - Timestamp  
  - Previous block hash  
  - SHA‑256 hash  
  - Nonce (Proof‑of‑Work)  
  - Vector of transactions  

- **Transaction model**
  - Sender  
  - Recipient  
  - Amount  

- **Blockchain**
  - Genesis block creation  
  - Mining with adjustable difficulty  
  - Pending transaction pool  
  - Transaction batching per block  
  - Chain validation  

- **Proof‑of‑Work**
  - Leading‑zero difficulty target  
  - Nonce incrementation loop  
  - Deterministic hashing  

---

## Project Structure
    src
    blockchain.cpp
    blockchain.h
    block.cpp
    block.h
    sha256.cpp
    sha256.h
    main.cpp

    
- `Block` — represents a single block  
- `Blockchain` — manages the chain, mining, and pending transactions  
- `sha256.*` — hashing implementation  
- `main.cpp` — example usage and basic testing  

---

## How It Works

### 1. Add transactions
Transactions are added to a pending pool:

```cpp
pendingTransactions.push_back({ "alice", "bob", 50.0 });
2. Mine a block

Mining selects a batch of pending transactions, performs PoW, and appends the block:
cpp

blockchain.mineBlock();

3. Hashing

Each block hash is derived from:

    index

    timestamp

    previousHash

    serialized transactions

    nonce

4. Validation

The chain is valid if:

    each block’s hash is correct

    each previousHash matches the block before it

    PoW difficulty is satisfied

Build & Run
Requirements

    C++17 or newer

    No external dependencies

Compile
bash

g++ -std=c++17 -O2 src/*.cpp -o blockchain

Run
bash

./blockchain

Example Output
Code

Mining block...
Block mined: 0000a3f9...
Adding block to chain...
Chain is valid: true

Roadmap

Planned additions:

    Mining reward transactions

    Difficulty retargeting

    Persistent storage

    Merkle trees

    Digital signatures (ECDSA)

    P2P networking (Asio)

    Configurable block size

    Serialization formats (JSON / binary)

Purpose

This project is intentionally minimal.
It serves as a foundation for learning and experimentation with blockchain internals, without the complexity of full cryptocurrency systems.
License

MIT License.
Code


---

If you want, I can also generate:

- a **badge‑heavy GitHub version**  
- a **diagram version** (ASCII or SVG)  
- a **more technical/academic version**  
- or a **shorter, portfolio‑friendly version**

Just tell me the style you want next.
