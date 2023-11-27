#ifndef CPP_LAB_7_8_ACCOUNTSMANAGER_H
#define CPP_LAB_7_8_ACCOUNTSMANAGER_H


#include "DatabaseConnection.h"
#include "../model/Customer.h"
#include "../model/Transaction.h"

class AccountsManager {
private:
    DatabaseConnection *transactions;
    DatabaseConnection *accounts;
    DatabaseConnection *users;

    static AccountType parseAccountType(int identifier);

public:
    explicit AccountsManager(const char *folder);

    bool isCustomerRegistered(const char *login);

    Customer *createCustomer(const char *login, const char *password);

    Customer *getCustomer(const char *user);

    BankAccount *createAccount(unsigned int ownerID, AccountType type);

    BankAccount *fetchAccount(unsigned int id);

    static BankAccount *parseAccount(const char *node);

    void changePassword(Customer *customer, const char *string);

    void closeAccount(BankAccount *pAccount);

    void initAccountsDatabase();

    Transaction *createTransaction(unsigned int from, unsigned int to,
                                   unsigned int amount, const char *metadata);

    Transaction *sendMoney(BankAccount *sender, BankAccount *receiver, int amount);

    Transaction *sendMoney(BankAccount *sender, BankAccount *receiver, int amount, char *metadata);

    void writeData(BankAccount *pAccount, bool withdraw);

    LinkedList<Transaction> *getTransactions(size_t amount, unsigned int *ids);
};


#endif //CPP_LAB_7_8_ACCOUNTSMANAGER_H
