//
// Created by user on 24.11.2023.
//

#include <stdexcept>
#include "CheckingAccount.h"

CheckingAccount::CheckingAccount(unsigned int id, unsigned int owner, AccountType type, AccountStatus status,
                                 long long int registration, int balance) : BankAccount(id, owner, type, status,
                                                                                        registration, balance) {
}

bool CheckingAccount::deposit(int amount) {
    this->balance += amount;
    return false;
}

bool CheckingAccount::withdraw(int amount) {
    if (this->balance < amount) {
        throw std::invalid_argument("Balance influence");
    }
    this->balance -= amount;
    return false;
}

char *CheckingAccount::genMetadata() {
    return nullptr;
}
