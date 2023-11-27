#include <cstring>
#include <stdexcept>
#include <ctime>
#include "AccountsManager.h"
#include "FileDatabaseConnection.h"
#include "../model/CheckingAccount.h"
#include "../model/SavingsAccount.h"

static const unsigned int INT_SIZE = sizeof(int);
static const unsigned int LONG_SIZE = sizeof(long long);
static const unsigned int USER_NODE_BYTES = INT_SIZE + 21 + 33;
static const unsigned int ACCOUNT_NODE_BYTES = INT_SIZE * 3 + LONG_SIZE + 32 + 2;
static const unsigned int TRANSACTION_NODE_BYTES = INT_SIZE * 4 + 16;

AccountsManager::AccountsManager(const char *folder) {
    // USER_ID (int) | USER_LOGIN (21 bytes) | LOGIN_PASSWORD (33 bytes)
    this->users = new FileDatabaseConnection(folder, "users");
    // TRANSACTION_ID (int) | ACCOUNT_FROM (int) | ACCOUNT_TO (int) | AMOUNT (int) | METADATA (16 bytes)
    this->transactions = new FileDatabaseConnection(folder, "transactions");
    // ACCOUNT_ID (int) | USER_ID (int) | ACCOUNT_TYPE (byte) | STATUS (byte) |
    // OPEN_DATE (long long) | BALANCE (int) | METADATA (16 bytes)
    this->accounts = new FileDatabaseConnection(folder, "accounts");

    initAccountsDatabase();
}

void AccountsManager::initAccountsDatabase() {
    if (this->accounts->hasData()) {
        return;
    }

    char data[ACCOUNT_NODE_BYTES * 3];
    std::memset(data, 0, ACCOUNT_NODE_BYTES * 3);
    int id;
    char type;

    // deposit
    id = 0;
    type = 0x2;
    std::memcpy(data, &id, INT_SIZE);
    std::memcpy(data + INT_SIZE * 2, &type, 1);

    // withdraw
    id = 1;
    type = 0x3;
    std::memcpy(data + ACCOUNT_NODE_BYTES, &id, INT_SIZE);
    std::memcpy(data + ACCOUNT_NODE_BYTES + INT_SIZE * 2, &type, 1);

    // rate deposit
    id = 2;
    type = 0x4;
    std::memcpy(data + ACCOUNT_NODE_BYTES * 2, &id, INT_SIZE);
    std::memcpy(data + ACCOUNT_NODE_BYTES * 2 + INT_SIZE * 2, &type, 1);

    this->accounts->write(ACCOUNT_NODE_BYTES * 3, data);
}

Customer *AccountsManager::createCustomer(const char *login, const char *password) {
    if (std::strlen(login) > 20 || std::strlen(password) > 32) {
        throw std::invalid_argument("Data is too long");
    }
    if (isCustomerRegistered(login)) {
        throw std::invalid_argument("Customer already exists");
    }

    char *node = new char[USER_NODE_BYTES];
    unsigned int id = -1;
    std::memcpy(node, &id, INT_SIZE);
    std::memcpy(node + INT_SIZE, login, std::strlen(login) + 1);
    std::memcpy(node + INT_SIZE + 21, password, std::strlen(password) + 1);
    this->users->write(USER_NODE_BYTES, node);
    delete[] node;
    return getCustomer(login);
}

bool AccountsManager::isCustomerRegistered(const char *user) {
    LinkedList<const char> list = this->users->read(USER_NODE_BYTES, INT_SIZE,
                                                    std::strlen(user) + 1, user);
    return list.getNextEntry() != nullptr;
}

Customer *AccountsManager::getCustomer(const char *user) {
    LinkedList<const char> list = this->users->read(USER_NODE_BYTES, INT_SIZE,
                                                    std::strlen(user) + 1, user);
    if (list.getNextEntry() == nullptr) {
        return nullptr;
    }
    const char *node = list.getNextEntry()->getValue();

    int id = 0;
    char *login = new char[21];
    char *password = new char[33];
    std::memcpy(&id, node, INT_SIZE);
    std::memcpy(login, node + INT_SIZE, 21);
    std::memcpy(password, node + INT_SIZE + 21, 33);

    LinkedList<const char> fetchedAccounts = this->accounts->
            read(ACCOUNT_NODE_BYTES, INT_SIZE, INT_SIZE, node);

    LinkedList<BankAccount> bankAccounts;
    LinkedList<const char> *listNode = &fetchedAccounts;
    while (listNode->getNextEntry() != nullptr) {
        listNode = listNode->getNextEntry();
        bankAccounts.addValue(parseAccount(listNode->getValue()));
    }
    return new Customer(id, login, password, bankAccounts);
}

BankAccount *AccountsManager::createAccount(unsigned int ownerID, AccountType type) {
    if (type != SAVINGS && type != CHECKING) {
        throw std::invalid_argument("Cannot create system account");
    }

    char *node = new char[ACCOUNT_NODE_BYTES];
    unsigned int id = -1;
    std::memcpy(node, &id, INT_SIZE);
    std::memcpy(node + INT_SIZE, &ownerID, INT_SIZE);
    node[INT_SIZE * 2] = type == CHECKING ? 0x0 : 0x1;
    node[INT_SIZE * 2 + 1] = 0x0; // OPENED

    long long currentTime = std::time(nullptr);
    std::memcpy(node + INT_SIZE * 2 + 2, &currentTime, LONG_SIZE);
    id = 0;
    std::memcpy(node + INT_SIZE * 2 + 2 + LONG_SIZE, &id, INT_SIZE);
    std::memset(node + (ACCOUNT_NODE_BYTES - 32), 0, 32);

    id = this->accounts->write(ACCOUNT_NODE_BYTES, node);
    delete[] node;
    return fetchAccount(id);
}

BankAccount *AccountsManager::fetchAccount(unsigned int id) {
    char *byteID = new char[INT_SIZE];
    std::memcpy(byteID, &id, INT_SIZE);
    LinkedList<const char> list = this->accounts->read(ACCOUNT_NODE_BYTES,
                                                       0, INT_SIZE, byteID);
    BankAccount *bankAccount = list.getNextEntry() == nullptr ? nullptr
                                                              : parseAccount(list.getNextEntry()->getValue());
    delete[] byteID;

    if (bankAccount != nullptr && bankAccount->getAccountType() == SAVINGS) {
        auto *savingsAccount = dynamic_cast<SavingsAccount *>(bankAccount);
        LinkedList<Transaction> transaction = savingsAccount->checkRateTopUp();
        if (transaction.getNextEntry() != nullptr) {
            LinkedList<Transaction> *entry = transaction.getNextEntry();
            while (entry != nullptr) {
                Transaction *elem = entry->getValue();
                this->createTransaction(elem->getAccountFromId(), elem->getAccountToId(),
                                        elem->getAmount(), elem->getMetadata());
                delete elem;
                entry = entry->getNextEntry();
            }
            this->writeData(savingsAccount, true);
        }
    }
    return bankAccount;
}

BankAccount *AccountsManager::parseAccount(const char *accountRaw) {
    int accountID = 0;
    int userID = 0;
    AccountType accountType;
    AccountStatus status;
    long long registration = 0;
    int balance = 0;
    char *metadata = new char[32];

    std::memcpy(&accountID, accountRaw, INT_SIZE);
    std::memcpy(&userID, accountRaw + INT_SIZE, INT_SIZE);
    accountType = parseAccountType(accountRaw[INT_SIZE * 2]);
    status = accountRaw[INT_SIZE * 2 + 1] == 0x1 ? CLOSED : OPENED;
    std::memcpy(&registration, accountRaw + INT_SIZE * 2 + 2, LONG_SIZE);
    std::memcpy(&balance, accountRaw + LONG_SIZE + INT_SIZE * 2 + 2, INT_SIZE);
    std::memcpy(metadata, accountRaw + (ACCOUNT_NODE_BYTES - 32), 32);

    BankAccount *account;
    switch (accountType) {
        case CHECKING: {
            account = new CheckingAccount(accountID, userID, accountType, status,
                                          registration, balance);
            break;
        }
        case SAVINGS: {
            int minimumBalance = 0;
            int fixedRate = 0;
            unsigned long long lastRateTopUp = 0;
            std::memcpy(&minimumBalance, metadata, INT_SIZE);
            std::memcpy(&fixedRate, metadata + INT_SIZE, INT_SIZE);
            std::memcpy(&lastRateTopUp, metadata + INT_SIZE * 2, LONG_SIZE);
            account = new SavingsAccount(accountID, userID, accountType, status,
                                         registration, balance, minimumBalance, fixedRate, lastRateTopUp);
            break;
        }
        default: {
            throw std::invalid_argument("System account decode");
        }
    }
    delete[] metadata;
    return account;
}

void AccountsManager::changePassword(Customer *customer, const char *password) {
    customer->setPassword(password);

    char compareID[INT_SIZE];
    int id = customer->getId();
    std::memcpy(compareID, &id, INT_SIZE);

    this->users->write(USER_NODE_BYTES, // base info
                       0, INT_SIZE, compareID, // compare
                       INT_SIZE + 21, 33, password); // write
}

void AccountsManager::closeAccount(BankAccount *account) {
    account->close();

    char compareID[INT_SIZE];
    unsigned int id = account->getId();
    std::memcpy(compareID, &id, INT_SIZE);

    char status[1];
    status[0] = 0x1;

    this->accounts->write(ACCOUNT_NODE_BYTES, // base info
                          0, INT_SIZE, compareID, // compare
                          INT_SIZE * 2 + 1, 1, status);
}

AccountType AccountsManager::parseAccountType(int identifier) {
    if (identifier < 0 || identifier > 4) {
        throw std::invalid_argument("Incorrect identifier");
    }
    return static_cast<AccountType>(identifier);
}

Transaction *AccountsManager::createTransaction(unsigned int from, unsigned int to,
                                                unsigned int amount, const char *metadata) {
    size_t metadataLength = std::strlen(metadata);
    if (std::strlen(metadata) > 15) {
        throw std::invalid_argument("Metadata is too long");
    }
    char *data = new char[TRANSACTION_NODE_BYTES];
    std::memset(data, 0, TRANSACTION_NODE_BYTES);
    unsigned int number = -1;
    std::memcpy(data, &number, INT_SIZE);
    number = from;
    std::memcpy(data + INT_SIZE, &number, INT_SIZE);
    number = to;
    std::memcpy(data + INT_SIZE * 2, &number, INT_SIZE);
    number = amount;
    std::memcpy(data + INT_SIZE * 3, &number, INT_SIZE);
    if (metadataLength > 0) {
        std::memcpy(data + INT_SIZE * 4, metadata, metadataLength + 1);
    }
    std::memset(data + (TRANSACTION_NODE_BYTES - 16 + metadataLength), 0, 16 - metadataLength);

    unsigned int id = this->transactions->write(TRANSACTION_NODE_BYTES, data);
    delete[] data;
    return new Transaction(id, from, to, amount, metadata);
}

Transaction *AccountsManager::sendMoney(BankAccount *sender, BankAccount *receiver, int amount) {
    char *metadata = new char[16];
    std::memset(metadata, 0, 16);
    return this->sendMoney(sender, receiver, amount, metadata);
}

Transaction *AccountsManager::sendMoney(BankAccount *sender, BankAccount *receiver, int amount, char *metadata) {
    unsigned int senderID = 2;
    unsigned int receiverID = 3;
    if (sender != nullptr) {
        writeData(sender, sender->withdraw(amount));
        senderID = sender->getId();
    }
    if (receiver != nullptr) {
        writeData(receiver, receiver->deposit(amount));
        receiverID = receiver->getId();
    }

    return createTransaction(senderID, receiverID, amount, metadata);
}

void AccountsManager::writeData(BankAccount *account, bool metadata) {
    unsigned int id = account->getId();
    char compareData[INT_SIZE];
    std::memcpy(compareData, &id, INT_SIZE);

    unsigned int writeLength = INT_SIZE + (metadata ? 16 : 0);
    char writeData[writeLength];
    int balance = account->getBalance();
    std::memcpy(writeData, &balance, INT_SIZE);
    if (metadata) {
        char *metadataData = account->genMetadata();
        std::memcpy(writeData + INT_SIZE, metadataData, 16);
    }

    this->accounts->write(ACCOUNT_NODE_BYTES,
                          0, INT_SIZE, compareData,
                          INT_SIZE * 2 + 2 + LONG_SIZE, writeLength, writeData);
}

LinkedList<Transaction> *AccountsManager::getTransactions(size_t amount, unsigned int *ids) {
    unsigned int positions[2];
    positions[0] = INT_SIZE;
    positions[1] = INT_SIZE * 2;
    LinkedList<const char> data = this->transactions->read(TRANSACTION_NODE_BYTES,
                                                           2, positions,
                                                           amount, ids);

    auto *result = new LinkedList<Transaction>();
    LinkedList<const char> *rawEntry = data.getNextEntry();
    while (rawEntry != nullptr) {
        const char *rawValue = rawEntry->getValue();

        unsigned int trID = 0;
        unsigned int fromID = 0;
        unsigned int toID = 0;
        unsigned int balance = 0;
        char *metadata = new char[16];

        std::memcpy(&trID, rawValue, INT_SIZE);
        std::memcpy(&fromID, rawValue + INT_SIZE, INT_SIZE);
        std::memcpy(&toID, rawValue + INT_SIZE * 2, INT_SIZE);
        std::memcpy(&balance, rawValue + INT_SIZE * 3, INT_SIZE);
        std::memcpy(metadata, rawValue + INT_SIZE * 4, 16);

        delete[] rawValue;

        result->addValue(new Transaction(trID, fromID,
                                         toID, balance, metadata));
        rawEntry = rawEntry->getNextEntry();
    }
    return result;
}
