#ifndef CPP_LAB_7_8_TRANSACTION_H
#define CPP_LAB_7_8_TRANSACTION_H


class Transaction {

private:
    const unsigned int transactionID;
    const unsigned int accountFromID;
    const unsigned int accountToID;
    const unsigned int amount;
    const char *metadata;

public:
    Transaction(unsigned int id, unsigned int accountFromID, unsigned int accountToID,
                unsigned int amount, const char *metadata)
            : transactionID(id), accountFromID(accountFromID), accountToID(accountToID), amount(amount),
              metadata(metadata) {
    }

    bool operator==(Transaction &other) const {
        return this->transactionID == other.transactionID;
    }

    bool operator!=(Transaction &other) const {
        return !(*this == other);
    }

    ~Transaction() {
        delete[] metadata;
    }

    unsigned int getTransactionId() const {
        return transactionID;
    }

    unsigned int getAccountFromId() const {
        return accountFromID;
    }

    unsigned int getAccountToId() const {
        return accountToID;
    }

    unsigned int getAmount() const {
        return amount;
    }

    const char *getMetadata() const {
        return metadata;
    }
};


#endif //CPP_LAB_7_8_TRANSACTION_H
