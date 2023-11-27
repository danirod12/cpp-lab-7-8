#ifndef CPP_LAB_7_8_SAVINGSACCOUNT_H
#define CPP_LAB_7_8_SAVINGSACCOUNT_H


#include "BankAccount.h"
#include "Transaction.h"
#include "../util/LinkedList.h"

class SavingsAccount : public BankAccount {
private:
    // We store balance * 100, so 47873.33 balance should be 4787333
    int minimumBalance;
    // We store year rate in format (rate * 1000000), so 14.1% year should be 14100000
    int fixedRate;
    unsigned long long lastRateTopUp;

public:
    SavingsAccount(unsigned int id, unsigned int owner, AccountType type, AccountStatus status,
                   long long registration, int balance,
                   int minimumBalance, int fixedRate, unsigned long long lastRateTopUp);

    ~SavingsAccount() override = default;

    /**
     * Check if there is time to top up us with rate
     *
     * @return Empty list if nothing happened, list of transactions that top up us
     */
    LinkedList<Transaction> checkRateTopUp();

    bool deposit(int amount) override;

    bool withdraw(int amount) override;

    char *genMetadata() override;
};


#endif //CPP_LAB_7_8_SAVINGSACCOUNT_H
