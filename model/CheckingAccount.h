#ifndef CPP_LAB_7_8_CHECKINGACCOUNT_H
#define CPP_LAB_7_8_CHECKINGACCOUNT_H


#include "BankAccount.h"

class CheckingAccount : public BankAccount {

public:
    CheckingAccount(unsigned int id, unsigned int owner, AccountType type, AccountStatus status,
                    long long registration, int balance);

    ~CheckingAccount() override = default;

    bool withdraw(int amount) override;

    bool deposit(int amount) override;

    char *genMetadata() override;
};


#endif //CPP_LAB_7_8_CHECKINGACCOUNT_H
