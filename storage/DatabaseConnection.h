#ifndef CPP_LAB_7_8_DATABASECONNECTION_H
#define CPP_LAB_7_8_DATABASECONNECTION_H

#include "./../util/LinkedList.h"

class DatabaseConnection {

public:
    /**
     * Read data entries by condition
     *
     * @param length        Data length
     * @param compareOffset Compare start offset (How much bytes to skip)
     * @param compareLength Compare length from offset (How much bytes should compare)
     * @param compareData   Compare data itself (Should contain compareLength bytes)
     * @return List of matched entries
     */
    virtual LinkedList<const char> read(unsigned int length, unsigned int compareOffset,
                                        unsigned int compareLength, const char *compareData) = 0;

    /**
     * Find data entries with specific INT values (Select data entries where one of columns with type INT
     * matches one of given values)
     *
     * @param length              Data length
     * @param compareOffsetAmount Possibly comparison positions (INT) amount
     * @param compareOffsets      Comparison offsets (compareOffsetAmount length)
     * @param compareDataAmount   Compare data (INT) variations
     * @param compareData         Compare data (compareDataAmount length)
     * @return List of matched entries
     */
    virtual LinkedList<const char> read(unsigned int length,
                                        unsigned int compareOffsetAmount, unsigned int *compareOffsets,
                                        unsigned int compareDataAmount, unsigned int *compareData) = 0;

    /**
     * INSERT / UPDATE based on provided ID. If compare mode is INT ID and writeLength equals length, performs
     * insert on missed/not set (-1) ID or update for exist ID.
     * <p>
     * Otherwise updates all data sets that matches comparison condition at fixed part
     *
     * @param length        Data length
     * @param compareOffset Compare offset from data start (How much bytes to skip)
     * @param compareLength Compare length from offset (How much bytes should compare)
     * @param compareData   Compare data itself (Should contain compareLength bytes)
     * @param writeOffset   Insert data offset for data set
     * @param writeLength   Insert data length for data set
     * @param writeData     The data to be written (Should contain writeLength bytes)
     * @return last modified ID (Inserted ID for insert mode)
     */
    virtual unsigned int write(unsigned int length,
                               unsigned int compareOffset, unsigned int compareLength, const char *compareData,
                               unsigned int writeOffset, unsigned int writeLength, const char *writeData) = 0;

    /**
     * INSERT / UPDATE based on provided ID. If provided ID is -1, inserts data to storage with new unique,
     * if provided ID contains in storage, replaces full data with new one, if provided ID does not contain
     * in file, inserts data with that ID, not recommended
     *
     * @param length Data length
     * @param data   Data chart with first INT bytes as an ID
     * @return last modified ID (Inserted ID for insert mode)
     */
    virtual unsigned int write(unsigned int length, char *data) = 0;

    virtual bool hasData() = 0;

    virtual ~DatabaseConnection() = default;
};

#endif //CPP_LAB_7_8_DATABASECONNECTION_H
