#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <unordered_map>
#include <fstream>
#include "sha256.h"

const int DIFFICULTY_ADJUSTMENT_INTERVAL = 5; // 5 blocks
const int TRANSACTION_FEE = 1; 
const int TARGET_BLOCK_TIME = 10; // seconds
const size_t MAX_TX_PER_BLOCK = 1000;


struct TransactionData {
    std::string sender;
    std::string recipient;
    double amount;
};

std::string transaction_to_str(const std::vector<TransactionData> &transactions);

class Block {
public:
    int index;
    std::string previousHash;
    std::vector<TransactionData> data;
    std::time_t timestamp;
    std::string hash;
    int difficulty;
    int nonce;
};

class Blockchain {
public:
    Blockchain();
    void createGenesisBlock();
    void mineCurrentBlock(const std::string& minerAddress = "NETWORK");//this needs to go to client, not blockchain
    void addToChain(const Block& newBlock);
    std::vector<Block> getChain() const { return chain; };
    double getBalance(const std::string& address);
    Block getLatestBlock() const { return chain.back(); };
    std::unordered_map<std::string, double> getBalances() const { return balances; };
    bool submitTransaction(const TransactionData &tx);//set tx into block
    void printChain() const;
    void printBlock(const Block& block) const;
    void saveToFile(const std::string& filename) const;
    std::string getDifficultyString() const { return std::string(difficulty, '0'); }
    bool verifyMinedBlock(const Block& block);
   
private:
    std::vector<Block> chain;
    int difficulty = 4; 
    std::unordered_map<std::string, double> balances; 
    void adjustDifficulty();
    std::string calculateHash(const Block& block);
    void updateBalances(const Block &block);
    double calculatePendingSpent(const std::string& sender) const;
};


Blockchain::Blockchain() {
    createGenesisBlock();
}

void Blockchain::createGenesisBlock() {
    Block genesisBlock;
    genesisBlock.index = 0;
    genesisBlock.previousHash = "0";
    genesisBlock.data = {};
    genesisBlock.timestamp = std::time(0);
    genesisBlock.hash = this->calculateHash(genesisBlock);
    genesisBlock.nonce = 0;
    genesisBlock.difficulty = difficulty;
    chain.push_back(genesisBlock);
}

void Blockchain::mineCurrentBlock(const std::string& minerAddress) {
    Block& current = chain.back();

    TransactionData rewardTx = {"NETWORK", minerAddress, 1.0}; // reward
    current.data.push_back(rewardTx);

    current.nonce = 0;    
    current.hash = calculateHash(current);

    while (current.hash.substr(0, difficulty) != std::string(difficulty, '0')) {
        current.nonce++;
        current.hash = calculateHash(current);
    }

    std::cout << "Mined block " << current.index << " with hash: " << current.hash << "\n";    
    updateBalances(current);

    Block newBlock;
    newBlock.index = chain.size();
    newBlock.previousHash = current.hash;
    newBlock.timestamp = std::time(nullptr);
    newBlock.nonce = 0;
    newBlock.difficulty = difficulty;
    newBlock.hash = calculateHash(newBlock);

    chain.push_back(newBlock);
    adjustDifficulty();
}

std::string Blockchain::calculateHash(const Block& block) {
    SHA256 sha256;
    std::string hashInput =
        std::to_string(block.index) +
        block.previousHash +
        transaction_to_str(block.data) +
        std::to_string(block.timestamp) +
        std::to_string(block.nonce);

    return sha256(hashInput);
}


void Blockchain::adjustDifficulty() {
    int size = this->getChain().size();
    if (size < DIFFICULTY_ADJUSTMENT_INTERVAL + 1) return;

    Block& lastBlock = chain.back();
    Block& prevAdjustBlock = chain[size - 1 - DIFFICULTY_ADJUSTMENT_INTERVAL];

    int actualTime = lastBlock.timestamp - prevAdjustBlock.timestamp;
    int expectedTime = TARGET_BLOCK_TIME * DIFFICULTY_ADJUSTMENT_INTERVAL;

    if (actualTime < expectedTime / 2) {
        difficulty++; // blocks too fast → increase difficulty
    } 
    else if (actualTime > expectedTime * 2) {
        difficulty--; // blocks too slow → decrease difficulty
        if (difficulty < 1) difficulty = 1;
    }

    std::cout << "Adjusted difficulty to: " << difficulty << "\n";
}

void Blockchain::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << "\n";
        return;
    }       

    for (const auto& block : chain) {
        file << block.index << ","
             << block.previousHash << ","
             << block.timestamp << ","
             << block.hash << ","
             << block.nonce << ",";

        for (const auto& tx : block.data) {
            file << tx.sender << "->" << tx.recipient << ":" << tx.amount << ";";
        }
        file << "\n\n";
    }
    file.close();
    std::cout << "Blockchain saved to " << filename << "\n";
}

void Blockchain::printBlock(const Block& block) const {
    std::cout << "-----------------------------\n";
    std::cout << "Block Index: " << block.index << "\n";
    std::cout << "Timestamp:   " << block.timestamp << "\n";
    std::cout << "Prev Hash:   " << block.previousHash << "\n";
    std::cout << "Hash:        " << block.hash << "\n";
    std::cout << "Nonce:       " << block.nonce << "\n";
    std::cout << "Transactions:\n";

    for (const auto& tx : block.data) {
        std::cout << "  " << tx.sender
                  << " -> " << tx.recipient
                  << " : " << tx.amount << "\n";
    }

    std::cout << "-----------------------------\n\n";
}

void Blockchain::printChain() const {
    std::cout << "\n=== BLOCKCHAIN START ===\n\n";

    for (const auto& block : chain) {
        printBlock(block);
    }

    std::cout << "=== BLOCKCHAIN END ===\n\n";
}



std::string transaction_to_str(const std::vector<TransactionData> &transactions) {
    std::string transactionString;
    for (const auto& transaction : transactions) {
        transactionString += transaction.sender + "->" + transaction.recipient + ":" + std::to_string(transaction.amount) + ";";
    }
    return transactionString;
//trasaction example: "Alice->Bob:10;Bob->Charlie:5;"
}


double Blockchain::getBalance(const std::string& address) {
    auto it = balances.find(address);
    if (it == balances.end()) {
        return 0.0; 
    }
    return it->second;
}

void Blockchain::updateBalances(const Block &block) {
    for (const auto& tx : block.data) {

        // NETWORK creates coins, so don't subtract
        if (tx.sender != "NETWORK") {
            balances[tx.sender] -= tx.amount;
        }

        balances[tx.recipient] += tx.amount;
    }
}

void Blockchain::addToChain(const Block& newBlock) {
    chain.push_back(newBlock);
    updateBalances(newBlock);
}

bool Blockchain::submitTransaction(const TransactionData& tx) {
    if (chain.empty()) {
        createGenesisBlock();
    }

    double pending = calculatePendingSpent(tx.sender);

    if (balances[tx.sender] - pending < tx.amount + TRANSACTION_FEE) {
        std::cerr << "Transaction rejected: insufficient funds for " << tx.sender << "\n";
        return false;
    }

    Block& current = chain.back();

    if (current.data.size() >= MAX_TX_PER_BLOCK) {
        std::cerr << "Transaction rejected: current block is full\n";
        return false;
    }

    current.data.push_back(tx);

    std::cout << "Transaction submitted: "
              << tx.sender << " -> " << tx.recipient
              << " : " << tx.amount << "\n";

    return true;
}

bool Blockchain::verifyMinedBlock(const Block& block) {
    std::string hash = calculateHash(block);
    return (hash == block.hash) && (hash.substr(0, difficulty) == std::string(difficulty, '0'));
}


double Blockchain::calculatePendingSpent(const std::string& sender) const {
    const Block& current = chain.back();
    double spent = 0.0;

    for (const auto& tx : current.data) {
        if (tx.sender == sender)
            spent += tx.amount;
    }

    return spent;
}

int main() {
    Blockchain myBlockchain;

    TransactionData transaction1 = {"Alice", "Bob", 10.0};
    TransactionData transaction2 = {"Bob", "Charlie", 5.0};

    if (!myBlockchain.submitTransaction(transaction1)) {
        std::cerr << "Failed to submit transaction 1\n";
    }
    if (!myBlockchain.submitTransaction(transaction2)) {
        std::cerr << "Failed to submit transaction 2\n";
    }
    Block blk = myBlockchain.getLatestBlock();

    myBlockchain.mineCurrentBlock();
    if (myBlockchain.verifyMinedBlock(blk)) {
        std::cout << "Block verified successfully!\n";
        myBlockchain.addToChain(blk);
    } else {
        std::cerr << "Block verification failed!\n";
    }
    
    myBlockchain.printChain();
    std::cout << "Current difficulty: " << myBlockchain.getDifficultyString() << "\n";
    myBlockchain.saveToFile("blockchain_data.txt");

    return 0;
}