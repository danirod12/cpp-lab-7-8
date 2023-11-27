#ifndef CPP_LAB_7_8_CUSTOMER_H
#define CPP_LAB_7_8_CUSTOMER_H


#include "BankAccount.h"
#include "../util/LinkedList.h"

class Customer {
private:
    int id;

    const char *login;
    char *password;

    LinkedList<BankAccount> bankAccounts;

public:
    Customer(int id, const char *login, char *password, LinkedList<BankAccount> bankAccounts);

    ~Customer();

    bool operator==(Customer &other) const {
        return this->id == other.id;
    }

    bool operator!=(Customer &other) const {
        return !(*this == other);
    }

    int getId() const;

    const char *getLogin() const;

    const LinkedList<BankAccount> &getBankAccounts() const;

    void addBankAccount(BankAccount *account);

    bool isValidPassword(const char *string);

    void setPassword(const char *string);
};


#endif //CPP_LAB_7_8_CUSTOMER_H
