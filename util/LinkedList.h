#ifndef CPP_LAB_7_8_LINKEDLIST_H
#define CPP_LAB_7_8_LINKEDLIST_H

#include <iostream>

template<typename T>
class LinkedList {
protected:
    T *value;
    LinkedList<T> *nextEntry;

public:
    LinkedList();

    explicit LinkedList(T *value);

    ~LinkedList();

    int getSize() const;

    T *getValue() const;

    void addValue(T *newValue);

    void addAll(LinkedList<T> *values);

    LinkedList<T> *getNextEntry() const;

    LinkedList<T> *skip(long values);
};

template<typename T>
LinkedList<T>::~LinkedList() {
    // TODO?
    delete[] this->value;
}

template<typename T>
void LinkedList<T>::addAll(LinkedList<T> *values) {
    while (values != nullptr) {
        if (values->value != nullptr) {
            addValue(values->value);
        }
        values = values->getNextEntry();
    }
}

template<typename T>
LinkedList<T> *LinkedList<T>::skip(long values) {
    if (values == 0) {
        return this;
    }

    long left = values - 1;
    LinkedList<T> *next = this->getNextEntry();
    while (next != nullptr && left-- > 0) {
        next = next->getNextEntry();
    }
    return left > 0 ? new LinkedList<T>() : next;
}

template<typename T>
LinkedList<T>::LinkedList() {
    this->value = nullptr;
    this->nextEntry = nullptr;
}

template<typename T>
LinkedList<T>::LinkedList(T *value) {
    this->value = value;
    this->nextEntry = nullptr;
}

template<typename T>
T *LinkedList<T>::getValue() const {
    if (this->value == nullptr) {
        throw std::runtime_error("The tail does not contain an element");
    }
    return this->value;
}

template<typename T>
LinkedList<T> *LinkedList<T>::getNextEntry() const {
    return this->nextEntry;
}

template<typename T>
void LinkedList<T>::addValue(T *newValue) {
    if (newValue == nullptr) {
        throw std::runtime_error("Non-tail cannot contain an empty pointer");
    }

    LinkedList<T> *ptr = this;
    while (ptr->nextEntry != nullptr) {
        ptr = ptr->nextEntry;
    }

    ptr->nextEntry = new LinkedList<T>(newValue);
}

template<typename T>
int LinkedList<T>::getSize() const {
    int size = 0;
    LinkedList *node = this->nextEntry;
    while (node != nullptr) {
        node = node->nextEntry;
        size++;
    }
    return size;
}

#endif //CPP_LAB_7_8_LINKEDLIST_H
