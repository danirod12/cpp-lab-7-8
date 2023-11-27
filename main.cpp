
#include <iostream>
#include <cstring>
#include <iomanip>
#include "storage/AccountsManager.h"
#include "storage/FileDatabaseConnection.h"

AccountsManager *manager;
Customer *customer = nullptr;

bool equals(const char *origin, const char *target);

char *getAccountFormatted(unsigned int id);

int main() {
    manager = new AccountsManager("./../database/");

    std::cout << std::setprecision(20);

    char command[256];

    // Yes, if with "false" condition, questions?
    // You will not believe, it even works as desired
    // A kind of C++ magic with goto operators
    if (false) {
        syntax:
        std::cout << "Syntax error";
    }
    while (true) {
        std::cout << std::endl << std::endl << "Waiting for input (type \"help\" for help)..." << std::endl;
        std::cin.getline(command, 256);

        char *arg1 = nullptr;
        char *arg2 = nullptr;
        char *arg3 = nullptr;
        for (int i = 0; i < 256; ++i) {
            if (command[i] == ' ') {
                command[i] = '\0';
                if (arg1 == nullptr) {
                    arg1 = command + i + 1;
                } else if (arg2 == nullptr) {
                    arg2 = command + i + 1;
                } else if (arg3 == nullptr) {
                    arg3 = command + i + 1;
                }
            } else if (command[i] == '\0') {
                break;
            }
        }

        if (equals(command, "exit")) {
            std::cout << std::endl << "Goodbye!" << std::endl;
            break;
        } else if (equals(command, "help")) {
            std::cout << "Welcome to simple bank system! Commands list:" << std::endl;
            std::cout << " * help - Show current menu" << std::endl;
            std::cout << " * help-admin - Show admin help" << std::endl;
            std::cout << " * exit - Finish program" << std::endl;
            std::cout << " * login <user> <password> - Log into your account" << std::endl;
            std::cout << " * logout - Log out of your account" << std::endl;
            std::cout << " * create <user> <password> - Create account" << std::endl;
            std::cout << " * changepwd <password> - Change password (log-in required)" << std::endl;
            std::cout << " * accounts - Print your bank accounts (log-in required)" << std::endl;
            std::cout << " * open <type> - Open new account (log-in required)" << std::endl;
            std::cout << " * close <id> - Close account (log-in required)" << std::endl;
            std::cout << " * transfer <id> <id> <amount> - Transfer money from to (log-in required)"
                      << std::endl;
            std::cout << " * history <page> - Transfers history (log-in required)" << std::endl;

            if (customer != nullptr) {
                std::cout << "[ACCOUNT] Currently logged as " << customer->getLogin();
            } else {
                std::cout << "[ACCOUNT] Currently you are not logged in";
            }
            continue;
        } else if (equals(command, "help-admin")) {
            std::cout << "Welcome to simple bank system! Admin commands list:" << std::endl;
            std::cout << " * ~force-login <user> - Log into account without password" << std::endl;
            std::cout << " * ~deposit <id> <amount> - Top up account balance from bank" << std::endl;
            std::cout << " * ~withdraw <id> <amount> - Withdraw money from account" << std::endl;
            continue;
        } else if (equals(command, "logout")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
            } else {
                customer = nullptr;
                std::cout << "Successfully logged out";
            }
            continue;
        } else if (equals(command, "login")) {
            if (arg1 == nullptr || arg2 == nullptr || std::strlen(arg1) > 20 || std::strlen(arg2) > 32) {
                goto syntax;
            }

            Customer *target = manager->getCustomer(arg1);
            if (target == nullptr || !target->isValidPassword(arg2)) {
                std::cout << "Incorrect username or password";
                continue;
            }
            customer = target;
            std::cout << "Welcome back, " << customer->getLogin() << "!";
            continue;
        } else if (equals(command, "~force-login")) {
            if (arg1 == nullptr || std::strlen(arg1) > 20) {
                goto syntax;
            }

            Customer *target = manager->getCustomer(arg1);
            if (target == nullptr) {
                std::cout << "Incorrect username";
                continue;
            }
            customer = target;
            std::cout << "Force logged as " << customer->getLogin() << ", he beg you to not steal all money";
            continue;
        } else if (equals(command, "create")) {
            if (arg1 == nullptr || arg2 == nullptr) {
                goto syntax;
            }
            size_t len1 = std::strlen(arg1);
            size_t len2 = std::strlen(arg2);
            if (len1 < 5 || len1 > 20 || len2 < 4 || len2 > 32) {
                std::cout << "User name is from 5 to 20 symbols, password is from 4 to 32 symbols";
                continue;
            }

            if (manager->isCustomerRegistered(arg1)) {
                std::cout << "Invalid user name";
                continue;
            }

            customer = manager->createCustomer(arg1, arg2);
            std::cout << "User " << arg1 << " registered and checked out. Use \"logout\" to log out";
            continue;
        } else if (equals(command, "changepwd")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            size_t len1 = std::strlen(arg1);
            if (arg1 == nullptr || len1 < 4 || len1 > 32) {
                std::cout << "Password is from 4 to 32 symbols";
                continue;
            }

            manager->changePassword(customer, arg1);
            std::cout << "Password successfully changed";
            continue;
        } else if (equals(command, "accounts")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            LinkedList<BankAccount> *account = customer->getBankAccounts().getNextEntry();
            if (account == nullptr) {
                std::cout << "You have no accounts, why :(" << std::endl;
            }
            while (account != nullptr) {
                BankAccount *bankAccount = account->getValue();
                std::cout << " * Number: " << bankAccount->getId() << ", Type: "
                          << (bankAccount->getAccountType() == 0 ? "SAVINGS" : "CHECKING") << ", Status: "
                          << (bankAccount->getAccountStatus() == 0 ? "OPENED" : "CLOSED")
                          << ", Balance: " << (bankAccount->getBalance() / 100.0) << std::endl;
                account = account->getNextEntry();
            }
            std::cout << "Open new account with \"open <type>\"";
            continue;
        } else if (equals(command, "open")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            if (arg1 == nullptr) {
                goto syntax;
            }
            AccountType type;
            if (equals(arg1, "savings")) {
                type = SAVINGS;
            } else if (equals(arg1, "checking")) {
                type = CHECKING;
            } else {
                std::cout << R"(Incorrect account type, use "savings" or "checking")";
                continue;
            }

            BankAccount *account = manager->createAccount(customer->getId(), type);
            customer->addBankAccount(account);
            std::cout << "Opened new bank account with ID " << account->getId() << ". Use \"accounts\"";
            continue;
        } else if (equals(command, "close")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            if (arg1 == nullptr) {
                goto syntax;
            }

            char *endPtr;
            long targetID = std::strtol(arg1, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            BankAccount *target = nullptr;
            LinkedList<BankAccount> *iterator = customer->getBankAccounts().getNextEntry();
            while (iterator != nullptr) {
                BankAccount *current = iterator->getValue();
                if (current->getId() == targetID) {
                    target = current;
                    break;
                }
                iterator = iterator->getNextEntry();
            }

            if (target->getAccountStatus() == CLOSED) {
                std::cout << "Account is already closed";
                continue;
            }
            if (target->getBalance() > 0) {
                std::cout << "You cannot close account with non-zero balance";
                continue;
            }
            manager->closeAccount(target);
            std::cout << "Account was successfully closed";
            continue;
        } else if (equals(command, "transfer")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            if (arg1 == nullptr || arg2 == nullptr || arg3 == nullptr) {
                goto syntax;
            }

            char *endPtr;
            long senderID = std::strtol(arg1, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            long targetID = std::strtol(arg2, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            if (senderID == targetID) {
                std::cout << "Have you been tested for schizophrenia?";
                continue;
            }

            double amount = std::strtod(arg3, &endPtr);
            if (*endPtr != '\0') {
                goto syntax;
            }

            int amountFixed = (int) (amount * 100);
            if (amountFixed < 1) {
                goto syntax;
            }

            LinkedList<BankAccount> *account = customer->getBankAccounts().getNextEntry();
            BankAccount *sender = nullptr;
            while (account != nullptr) {
                if (account->getValue()->getId() == senderID) {
                    sender = account->getValue();
                    break;
                }
                account = account->getNextEntry();
            }

            if (sender == nullptr || sender->getAccountStatus() == CLOSED) {
                std::cout << "Account with ID " << senderID << " is not your or closed";
                continue;
            }

            if (sender->getBalance() < amountFixed) {
                std::cout << "Balance influence. You have only " << sender->getBalance() / 100.0;
                continue;
            }

            BankAccount *target = manager->fetchAccount(targetID);
            if (target == nullptr || target->getAccountStatus() == CLOSED) {
                std::cout << "Account with ID " << targetID << " not found or closed";
                continue;
            }

            if (target->getOwner() == customer->getId()) {
                account = customer->getBankAccounts().getNextEntry();
                while (account != nullptr) {
                    if (account->getValue()->getId() == target->getId()) {
                        target = account->getValue();
                        break;
                    }
                    account = account->getNextEntry();
                }
            }

            manager->sendMoney(sender, target, amountFixed);
            std::cout << "Sent " << amountFixed / 100.0 << " from " << senderID << " to " << targetID << std::endl;
            std::cout << "New balance on " << senderID << " is " << sender->getBalance() / 100.0;
            if (target->getOwner() == customer->getId()) {
                std::cout << std::endl << "New balance on " << targetID << " is " << target->getBalance() / 100.0;
            }
            continue;
        } else if (equals(command, "~deposit")) {
            if (arg1 == nullptr || arg2 == nullptr) {
                goto syntax;
            }

            char *endPtr;
            long id = std::strtol(arg1, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            long amount = std::strtol(arg2, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            int amountFixed = (int) (amount * 100);
            if (amountFixed < 1) {
                goto syntax;
            }

            BankAccount *account = manager->fetchAccount(id);
            if (account == nullptr || account->getAccountStatus() == CLOSED) {
                std::cout << "Account with ID " << id << " not found or closed";
                continue;
            }

            if (customer != nullptr && account->getOwner() == customer->getId()) {
                LinkedList<BankAccount> *list = customer->getBankAccounts().getNextEntry();
                while (list != nullptr) {
                    if (list->getValue()->getId() == account->getId()) {
                        account = list->getValue();
                        break;
                    }
                    list = list->getNextEntry();
                }
            }

            manager->writeData(account, account->deposit(amountFixed));
            manager->createTransaction(0, account->getId(), amountFixed, "Admin deposit");
            std::cout << "Topped up " << id << " for " << amountFixed / 100.0;
            continue;
        } else if (equals(command, "~withdraw")) {
            if (arg1 == nullptr || arg2 == nullptr) {
                goto syntax;
            }

            char *endPtr;
            long id = std::strtol(arg1, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            long amount = std::strtol(arg2, &endPtr, 10);
            if (*endPtr != '\0') {
                goto syntax;
            }

            int amountFixed = (int) (amount * 100);
            if (amountFixed < 1) {
                goto syntax;
            }

            BankAccount *account = manager->fetchAccount(id);
            if (account == nullptr || account->getAccountStatus() == CLOSED) {
                std::cout << "Account with ID " << id << " not found or closed";
                continue;
            }

            if (account->getBalance() < amountFixed) {
                std::cout << "Balance influence ("
                          << account->getBalance() / 100.0 << " < " << amountFixed / 100.0 << ")";
                continue;
            }

            if (customer != nullptr && account->getOwner() == customer->getId()) {
                LinkedList<BankAccount> *list = customer->getBankAccounts().getNextEntry();
                while (list != nullptr) {
                    if (list->getValue()->getId() == account->getId()) {
                        account = list->getValue();
                        break;
                    }
                    list = list->getNextEntry();
                }
            }

            manager->writeData(account, account->withdraw(amountFixed));
            manager->createTransaction(account->getId(), 1, amountFixed, "Admin withdraw");
            std::cout << "Withdraw from " << id << " for " << amountFixed / 100.0;
            continue;
        } else if (equals(command, "history")) {
            if (customer == nullptr) {
                std::cout << "Currently you are not logged in";
                continue;
            }

            long page = 1;
            if (arg1 != nullptr) {
                char *endPtr;
                page = std::strtol(arg1, &endPtr, 10);
                if (*endPtr != '\0') {
                    goto syntax;
                }
            }

            if (page < 1) {
                goto syntax;
            }

            LinkedList<Transaction> transactions;

            LinkedList<BankAccount> accountsList = customer->getBankAccounts();
            LinkedList<BankAccount> *accountEntry = accountsList.getNextEntry();

            size_t accountsSize = accountsList.getSize();
            if (accountsSize > 0) {
                unsigned int accounts[accountsSize];
                size_t i = 0;
                while (accountEntry != nullptr) {
                    accounts[i++] = accountEntry->getValue()->getId();
                    accountEntry = accountEntry->getNextEntry();
                }
                transactions.addAll(manager->getTransactions(accountsSize, accounts));
            }

            size_t pages = (transactions.getSize() + 9) / 10;
            if (pages == 0) {
                std::cout << "No transactions so far";
                continue;
            }
            if (page > pages) {
                std::cout << "No such page " << page << " of " << pages;
                continue;
            }

            LinkedList<Transaction> *transactionEntry = transactions.skip((page - 1) * 10)->getNextEntry();
            int limit = 10;
            while (transactionEntry != nullptr && limit-- > 0) {
                Transaction *value = transactionEntry->getValue();

                char *fromString = getAccountFormatted(value->getAccountFromId());
                char *toString = getAccountFormatted(value->getAccountToId());

                std::cout << " * ID: " << value->getTransactionId() << " (" << fromString << " -> " << toString
                          << "), Amount: " << value->getAmount() / 100.0 << " (Comment: "
                          << (std::strlen(value->getMetadata()) > 0 ? value->getMetadata() : "no comment") << ")"
                          << std::endl;

                delete[] fromString;
                delete[] toString;
                transactionEntry = transactionEntry->getNextEntry();
            }
            std::cout << "Page " << page << " of " << pages;
            continue;
        }
    }
    return 0;
}

char *getAccountFormatted(unsigned int id) {
    char *array = new char[20];
    switch (id) {
        case 0: {
            std::memcpy(array, "DEPOSIT", 8);
            break;
        }
        case 1: {
            std::memcpy(array, "WITHDRAW", 9);
            break;
        }
        case 2: {
            std::memcpy(array, "RATE DEPOSIT", 13);
            break;
        }
        default: {
            // Again we cannot use std::string (Mr. Bulgakov uses on lectures already !!!)
            int digits= snprintf(nullptr, 0, "%d", id);
            array = new char[digits];
            snprintf(array, digits + 1, "%d", id);

            if (customer != nullptr) {
                LinkedList<BankAccount> *linkedList = customer->getBankAccounts().getNextEntry();
                while (linkedList != nullptr) {
                    if (linkedList->getValue()->getId() == id) {
                        std::memcpy(array + digits, " [yours]", 9);
                        break;
                    }
                    linkedList = linkedList->getNextEntry();
                }
            }
            break;
        }
    }
    return array;
}

bool equals(const char *origin, const char *target) {
    if (origin == nullptr || target == nullptr) {
        return origin == target;
    }

    size_t index = 0;
    while (true) {
        if (origin[index] != target[index]) {
            return false;
        }
        if (origin[index] == '\0' || origin[index] == ' ') {
            return true;
        }
        index++;
    }
}
