#include <fstream>
#include <cstring>
#include <iostream>
#include "FileDatabaseConnection.h"

FileDatabaseConnection::FileDatabaseConnection(const char *folderPath, const char *fileName) {
    size_t folderLength = std::strlen(folderPath);
    size_t fileNameLength = std::strlen(fileName);

    char *fullFilePath = new char[folderLength + fileNameLength + 5];
    std::memcpy(fullFilePath, folderPath, folderLength); // NOLINT
    std::memcpy(fullFilePath + folderLength, fileName, fileNameLength); // NOLINT
    std::memcpy(fullFilePath + folderLength + fileNameLength, ".d12", 5);

    this->filePath = fullFilePath;
    init();
}

FileDatabaseConnection::FileDatabaseConnection(const char *filePath) {
    this->filePath = filePath;
    init();
}

void FileDatabaseConnection::init() {
    // Create file
    std::ofstream stream(this->filePath, std::ios::out | std::ios::app);
    stream.close();
}

LinkedList<const char> FileDatabaseConnection::read(unsigned int length, unsigned int compareOffset,
                                        unsigned int compareLength, const char *compareData) {
    if (compareOffset + compareLength > length) {
        throw std::invalid_argument("Incorrect comparison requirement");
    }

    std::ifstream stream(this->filePath);
    if (!stream.is_open()) {
        throw std::invalid_argument("Cannot open file connection");
    }

    LinkedList<const char> list;
    loop:
    while (!stream.eof()) {
        char *buffer = new char[length];
        stream.read(buffer, length);
        for (int i = 0; i < compareLength; ++i) {
            if (buffer[compareOffset + i] != compareData[i]) {
                goto loop;
            }
        }
        list.addValue(buffer);
    }
    return list;
}

LinkedList<const char>
FileDatabaseConnection::read(unsigned int length, unsigned int compareOffsetAmount, unsigned int *compareOffsets,
                             unsigned int compareDataAmount, unsigned int *compareData) {
    int intSize = sizeof(int);

    for (int i = 0; i < compareOffsetAmount; ++i) {
        if (compareOffsets[i] + intSize > length) {
            throw std::invalid_argument("Incorrect comparison requirement");
        }
    }

    std::ifstream stream(this->filePath);
    if (!stream.is_open()) {
        throw std::invalid_argument("Cannot open file connection");
    }

    LinkedList<const char> list;
    while (!stream.eof()) {
        char *buffer = new char[length];
        stream.read(buffer, length);

        if (!stream.good()) {
            break;
        }

        // offset variations
        for (int i = 0; i < compareOffsetAmount; ++i) {
            // given data variations
            for (int j = 0; j < compareDataAmount; ++j) {
                // comparison itself
                char intFormat[intSize];
                std::memcpy(intFormat, &compareData[j], intSize);

                bool foundMatch = true;
                for (int k = 0; k < intSize; ++k) {
                    if (buffer[compareOffsets[i] + k] != intFormat[k]) {
                        foundMatch = false;
                        break;
                    }
                }
                if (foundMatch) {
                    goto good;
                }
            }
        }
        continue;

        good:
        list.addValue(buffer);
    }
    return list;
}

unsigned int FileDatabaseConnection::write(unsigned int length, char *data) {
    return this->write(length, 0, sizeof(int),
                data, 0, length, data);
}

unsigned int FileDatabaseConnection::write(unsigned int length, unsigned int compareOffset, unsigned int compareLength,
                                           const char *compareData, unsigned int writeOffset, unsigned int writeLength,
                                           const char *writeData) {
    if (writeOffset + writeLength > length || compareOffset + compareLength > length) {
        throw std::invalid_argument("Incorrect comparison requirement / write options");
    }

    std::fstream stream(this->filePath, std::ios::binary | std::ios::in | std::ios::out);
    if (!stream.is_open()) {
        throw std::invalid_argument("Cannot open file connection");
    }

    int intSize = sizeof(int);

    // insert mode support if meet conditions
    bool insertMode = false;
    bool idCompareMode = false;
    int compareId = 0;
    if (compareOffset == 0 && compareLength == intSize && length == writeLength) {
        std::memcpy(&compareId, compareData, intSize);
        insertMode = compareId == -1;
        idCompareMode = true;
    }

    unsigned int lastId = 0;

    // possibly not really needed, but it is better to add
    stream.seekg(0, std::ios::beg);

    loop:
    while (true) {
        // Read buffer
        char *buffer = new char[length];
        stream.read(buffer, length);

        // Break if we have no more data to process
        if (!stream.good()) {
            break;
        }

        // Read current id from data set on buffer
        int currentId;
        std::memcpy(&currentId, buffer, intSize);
        if (insertMode) {
            if (currentId > lastId) {
                lastId = currentId;
            }
            continue;
        }

        // Compare with requirements
        for (int i = 0; i < compareLength; ++i) {
            if (buffer[compareOffset + i] != compareData[i]) {
                delete[] buffer;
                goto loop;
            }
        }
        delete[] buffer;
        // At this stage we are ready to make update (we know that we are not dealing wth insert mode)

        // Reset file position (go back to insert position. Actually we want to go back to buffer bytes
        // and go forward to writeOffset bytes)
        stream.seekp(-static_cast<std::streamoff>(length - writeOffset), std::ios::cur);

        // Write the new data
        stream.write(writeData, writeLength);

        // Return to correct write position
        stream.seekg(static_cast<std::streamoff>(length - writeOffset - writeLength), std::ios::cur);

        // Update last modified id
        lastId = currentId;
    }

    if (idCompareMode) {
        char writeDataCopy[length];
        std::memcpy(writeDataCopy, writeData, length);

        if (insertMode) {
            lastId++;
            std::memcpy(writeDataCopy, &lastId, intSize);
        } else if (lastId == 0) {
            lastId = compareId;
        } else {
            return lastId;
        }

        stream.seekg(0, std::ios::end);
        stream.clear(std::ios::goodbit);
        stream.write(writeDataCopy, length);
    }
    stream.close();
    return lastId;
}

bool FileDatabaseConnection::hasData() {
    std::ifstream stream(this->filePath);
    if (!stream.is_open()) {
        throw std::invalid_argument("Cannot open file connection");
    }

    char addr[1];
    stream.read(addr, 1);
    return !stream.eof();
}
