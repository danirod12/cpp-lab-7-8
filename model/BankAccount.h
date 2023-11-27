#ifndef CPP_LAB_7_8_BANKACCOUNT_H
#define CPP_LAB_7_8_BANKACCOUNT_H


enum AccountType {
    CHECKING = 0,
    SAVINGS = 1,
    /*
     * We do not use enum fields, but IDs are reserved and used for internal login
    SYSTEM_DEPOSIT = 2,
    SYSTEM_WITHDRAW = 3,
    SYSTEM_RATE_DEPOSIT = 4
     */
};

enum AccountStatus {
    OPENED,
    CLOSED
};

class BankAccount {
private:
    const unsigned int id;
    const unsigned int owner;
    const AccountType accountType;
    AccountStatus accountStatus;
    long long registration;
protected:
    int balance;

public:
    BankAccount(unsigned int id, unsigned int owner, AccountType type, AccountStatus status,
                long long registration, int balance) : id(id), owner(owner), accountType(type),
                                                       accountStatus(status), registration(registration),
                                                       balance(balance) {
    };

    virtual ~BankAccount() = default;

    bool operator==(BankAccount &other) const {
        return this->id == other.id;
    }

    bool operator!=(BankAccount &other) const {
        return !(*this == other);
    }

    /**
     * Deposit amount to bank account without database synchronization
     *
     * @param amount Sum * 100, for instance if we want to withdraw 100.43, we should pass 10043
     * @return True if metadata was changed, False otherwise
     */
    virtual bool deposit(int amount) = 0;

    /**
     * Withdraw amount from bank account without database synchronization
     *
     * @param amount Sum * 100, for instance if we want to withdraw 100.43, we should pass 10043
     * @return True if metadata was changed, False otherwise
     * @throws std::invalid_argument on balance influence
     */
    virtual bool withdraw(int amount) = 0;

    unsigned int getId() const {
        return id;
    }

    unsigned int getOwner() const {
        return owner;
    }

    AccountType getAccountType() const {
        return accountType;
    }

    AccountStatus getAccountStatus() const {
        return accountStatus;
    }

    long long int getRegistration() const {
        return registration;
    }

    int getBalance() const {
        return balance;
    }

    /**
     * Mark account closed without database synchronization
     */
    void close() {
        this->accountStatus = CLOSED;
    }

    virtual char *genMetadata() = 0;
};

#endif //CPP_LAB_7_8_BANKACCOUNT_H
