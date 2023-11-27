#include <cstring>
#include "Customer.h"

Customer::Customer(int id, const char *login, char *password, LinkedList<BankAccount> bankAccounts)
        : id(id), login(login), password(password), bankAccounts(bankAccounts) {
}

Customer::~Customer() {
    delete[] this->password;
    delete[] this->login;
}

int Customer::getId() const {
    return id;
}

const char *Customer::getLogin() const {
    return login;
}

const LinkedList<BankAccount> &Customer::getBankAccounts() const {
    return bankAccounts;
}

void Customer::addBankAccount(BankAccount *account) {
    if (account->getOwner() != this->id) {
        throw std::invalid_argument("Bank account does not related to this user");
    }

    LinkedList<BankAccount> *other = &this->bankAccounts;
    while (other->getNextEntry() != nullptr) {
        other = other->getNextEntry();
        if (other->getValue()->getId() == account->getId()) {
            return;
        }
    }
    other->addValue(account);
}

bool Customer::isValidPassword(const char *pwd) {
    int index = 0;
    while (true) {
        if (pwd[index] != this->password[index]) {
            return false;
        }
        if (pwd[index++] == '\0') {
            return true;
        }
    }
}

void Customer::setPassword(const char *string) {
    delete[] this->password;
    this->password = new char[33];
    std::memcpy(this->password, string, 33);
}
