#include <stdexcept>
#include <ctime>
#include <cstring>
#include "SavingsAccount.h"
#include "Transaction.h"
#include "../util/LinkedList.h"

static const unsigned long long MS_IN_MONTH = 30LL * 24 * 60 * 60 * 1000;

SavingsAccount::SavingsAccount(unsigned int id, unsigned int owner, AccountType type, AccountStatus status,
                               long long registration, int balance,
                               int minimumBalance, int fixedRate, unsigned long long lastRateTopUp)
        : BankAccount(id, owner, type, status, registration, balance),
          minimumBalance(minimumBalance), fixedRate(fixedRate), lastRateTopUp(lastRateTopUp) {
}

bool SavingsAccount::deposit(int amount) {
    this->balance += amount;

    // Check if first deposit or previous rate top up was less than 24 hours ago
    if (this->lastRateTopUp == 0L || (std::time(nullptr) - this->lastRateTopUp < 24LL * 60 * 60 * 1000)) {
        this->minimumBalance = this->balance;
        if (this->lastRateTopUp == 0L) {
            this->lastRateTopUp = std::time(nullptr);
        }
        return true;
    }
    return false;
}

bool SavingsAccount::withdraw(int amount) {
    if (this->balance < amount) {
        throw std::invalid_argument("Balance influence");
    }
    this->balance -= amount;
    if (this->minimumBalance > this->balance) {
        this->minimumBalance = this->balance;
        return true;
    }
    return false;
}

LinkedList<Transaction> SavingsAccount::checkRateTopUp() {
    LinkedList<Transaction> list;
    if (this->lastRateTopUp == 0) {
        return list;
    }

    unsigned long long currentTime = std::time(nullptr);
    while (currentTime - this->lastRateTopUp > MS_IN_MONTH) {
        this->lastRateTopUp += MS_IN_MONTH;
        double currentBalance = this->balance / 100.0;
        double currentRate = (this->fixedRate / 1000.0 + 100) / 12;
        currentBalance *= currentRate;
        int newBalance = (int) currentBalance;
        list.addValue(new Transaction(-1, 2, this->getId(),
                                      (newBalance - this->balance), "Rate top up"));
        this->balance = newBalance;
    }
    return list;
}

char *SavingsAccount::genMetadata() {
    // FIXED_RATE (int) | MINIMUM_BALANCE (int) | LAST_RATE_TOP_UP (long long)
    char *data = new char[16];
    int intSize = sizeof(int);
    int longSize = sizeof(long long);
    std::memcpy(data, &(this->fixedRate), intSize);
    std::memcpy(data + intSize, &(this->minimumBalance), intSize);
    std::memcpy(data + intSize * 2, &(this->lastRateTopUp), longSize);
    return data;
}
