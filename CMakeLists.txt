cmake_minimum_required(VERSION 3.26)
project(cpp_lab_7_8)

set(CMAKE_CXX_STANDARD 11)

add_executable(cpp_lab_7_8 main.cpp
        util/LinkedList.h
        storage/DatabaseConnection.h
        storage/FileDatabaseConnection.h
        storage/FileDatabaseConnection.cpp
        storage/AccountsManager.cpp
        storage/AccountsManager.h
        model/Transaction.h
        model/BankAccount.h
        model/Customer.cpp
        model/Customer.h
        model/CheckingAccount.cpp
        model/CheckingAccount.h
        model/SavingsAccount.cpp
        model/SavingsAccount.h)
