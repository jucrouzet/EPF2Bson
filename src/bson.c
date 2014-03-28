/**
 * BSON Format.
 *
 * @author              Julien CROUZET <contact@juliencrouzet.fr>
 * @copyright           Julien CROUZET <contact@juliencrouzet.fr>
 */

/**
    This file is part of EPF2Bson.

    EPF2Bson is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EPF2Bson is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EPF2Bson. If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE

#include "EPF2Bson.h"
#include "error.h"
#include "bson.h"



/**
 * Increment field count and realloc if needed.
 *
 * \param document Document to increment.
 */
void _incrementCount(bsonDocument* document) {
    document->fieldCount++;
    if (document->fieldCount >= document->_lastAllocationsSize) {
        document->_lastAllocationsSize += 100;
        if (!document->fieldNames) {
            document->fieldNames = malloc(document->_lastAllocationsSize * sizeof(void*));
        } else {
            document->fieldNames = realloc(document->fieldNames, (document->_lastAllocationsSize * sizeof(void*)));
        }
        if (!document->fieldNames) {
            error("Cannot allocate memory for new BSON field");
        }
        if (!document->fieldTypes) {
            document->fieldTypes = malloc(document->_lastAllocationsSize * sizeof(bsonByte));
        } else {
            document->fieldTypes = realloc(document->fieldTypes, (document->_lastAllocationsSize * sizeof(bsonByte)));
        }
        if (!document->fieldTypes) {
            error("Cannot allocate memory for new BSON field");
        }
        if (!document->fields) {
            document->fields = malloc(document->_lastAllocationsSize * sizeof(void*));
        } else {
            document->fields = realloc(document->fields, (document->_lastAllocationsSize * sizeof(void*)));
        }
        if (!document->fields) {
            error("Cannot allocate memory for new BSON field");
        }
    }
}

/**
 * Appends a new value in field names.
 *
 * \param document Document to increment.
 * \param name     New name to append.
 */
void _appendFieldName(bsonDocument* document, char* name) {
    char* copy;

    copy = calloc(strlen(name) + 1, strlen(name));
    copy = strncpy(copy, name, strlen(name));
    document->fieldNames[document->fieldCount - 1] = copy;
}

/**
 * Serialize a value to BSON.
 *
 * \param document Document to increment.
 * \param index    Value index.
 *
 * \return Serialized value.
 */
bsonSerializedValue _serializedValue(bsonDocument* document, uint index) {
    bsonSerializedValue value;
    bsonSerializedValue subValue;
    bsonInt32 i32;
    bsonInt64 i64;
    char byteValue;
    bool boolValue;

    switch(document->fieldTypes[index]) {
        case BSON_TYPE_DOUBLE :
        case BSON_TYPE_INT64 :
            value.length = sizeof(bsonInt64);
            value.binaryValue = calloc(1, value.length);
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            memcpy(value.binaryValue, document->fields[index], value.length);
            break;
        case BSON_TYPE_STRING :
            //(int32) <byte length> <string> \x00
            i32 = strlen(document->fields[index]) + 1;
            value.length = 4 + i32;
            value.binaryValue = calloc(value.length, sizeof(char));
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            i32 = htole32(i32);
            memcpy(
                value.binaryValue,
                &i32,
                sizeof(bsonInt32)
            );
            memcpy(
                value.binaryValue + sizeof(bsonInt32),
                document->fields[index],
                strlen(document->fields[index])
            );
            break;
        case BSON_TYPE_DOCUMENT :
        case BSON_TYPE_ARRAY :
            subValue = bsonSerialize((bsonDocument*)document->fields[index]);
            value.length = subValue.length;
            value.binaryValue = calloc(value.length, sizeof(char));
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            memcpy(
                value.binaryValue,
                subValue.binaryValue,
                value.length
            );
            free(subValue.binaryValue);
            break;
        case BSON_TYPE_OBJECTID :
            value.length = 12;
            value.binaryValue = malloc(12);
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            memcpy(
                value.binaryValue,
                document->fields[index],
                value.length
            );
            break;
        case BSON_TYPE_BOOL :
            value.length = sizeof(char);
            value.binaryValue = malloc(value.length);
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            memcpy(
                &boolValue,
                document->fields[index],
                sizeof(bool)
            );
            if (boolValue) {
                byteValue = '\x01';
            } else {
                byteValue = '\x00';
            }
            memcpy(
                value.binaryValue,
                &byteValue,
                value.length
            );
            break;
        case BSON_TYPE_UTCDATE:
            value.length = sizeof(bsonInt64);
            value.binaryValue = malloc(value.length);
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            i64 = htole64((bsonInt64)document->fields[index]);
            memcpy(
                value.binaryValue,
                &i64,
                value.length
            );
            break;
        case BSON_TYPE_NULL:
            value.length = 0;
            value.binaryValue = NULL;
            break;
        case BSON_TYPE_INT32 :
            value.length = sizeof(bsonInt32);
            value.binaryValue = calloc(1, value.length);
            if (!value.binaryValue) {
                error("Could not allocate memory");
            }
            memcpy(value.binaryValue, document->fields[index], value.length);
            break;
        default :
            error("Unknown BSON field type (%d) while serializing", document->fieldTypes[index]);
    }
    return(value);
}

/**
 * Creates a new BSON document.
 *
 * \return Document.
 */
bsonDocument* createBsonDocument() {
    bsonDocument* document;

    document = calloc(1, sizeof(bsonDocument));
    if (!document) {
        error("Cannot allocate memory for new BSON document");
    }
    document->fieldCount = 0;
    document->_lastAllocationsSize = 0;
    return(document);
}

/**
 * Destroys a bson document instance.
 *
 * \param document Document to destroy.
 */
void destroyBsonDocument(bsonDocument* document) {
    if (document->fieldCount) {
        for(int i = 0; i < document->fieldCount; i++) {
            free(document->fieldNames[i]);
            free(document->fields[i]);
        }
        free(document->fieldNames);
        free(document->fields);
        free(document->fieldTypes);
    }
    free(document);
}

/**
 * Checks if given field name already exists in document.
 *
 * \param document Document to insert into.
 * \param name     Field name.
 *
 * \return True if exists, false elsewhere.
 */
bool fieldNameExists(bsonDocument* document, char* name) {
    if (document->fieldNames && document->fieldCount) {
        for(int i = 0; i < document->fieldCount; i++) {
            if (!strcmp(document->fieldNames[i], name)) {
                return(true);
            }
        }
    }
    return(false);
}
//CDCCCCCCCCCC1440
/**
 * Serialize document as BSON.
 *
 * \param document Document to insert into.
 *
 * \return BSON serialized. (`binaryValue` must be free()'d after use).
 */
bsonSerializedValue bsonSerialize(bsonDocument* document) {
    char* bson = malloc(1024);
    size_t allocatedSize = 1024;
    size_t documentSize = 5;
    size_t bufferPosition = 4;
    bsonSerializedValue serializedDocument;
    bsonSerializedValue value;
    bsonByte fieldType;
    bool reallocNeeded = false;

    if (!bson) {
        error("Could not allocate memory");
    }
    if (document->fieldCount) {
        for(uint i = 0; i < document->fieldCount; i++) {
            value = _serializedValue(document, i);
            fieldType = document->fieldTypes[i];
            documentSize += (1 + strlen(document->fieldNames[i]) + 1 + value.length);
            while (documentSize >= allocatedSize) {
                allocatedSize += 1048576;
                reallocNeeded = true;
            }
            if (reallocNeeded) {
                bson = realloc(bson, allocatedSize);
                if (!bson) {
                    error("Could not allocate memory");
                }
                reallocNeeded = false;
            }
            memcpy(
                bson + bufferPosition,
                &fieldType,
                1
            );
            bufferPosition++;
            memcpy(
                bson + bufferPosition,
                document->fieldNames[i],
                strlen(document->fieldNames[i]) + 1
            );
            bufferPosition += (strlen(document->fieldNames[i]) + 1);
            if (value.length > 0) {
                memcpy(
                    bson + bufferPosition,
                    value.binaryValue,
                    value.length
                );
                bufferPosition += value.length;
                free(value.binaryValue);
            }
        }
    }
    documentSize = htole32(documentSize);
    bson[bufferPosition++] = 0;
    memcpy(
        bson,
        &bufferPosition,
        sizeof(bsonInt32)
    );
    serializedDocument.binaryValue = bson;
    serializedDocument.length = bufferPosition;
    return(serializedDocument);
}

/**
 * Insert a double value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Double's value.
 *
 * \return true on success, false on error.
 */
bool bsonAddDouble(bsonDocument* document, char* name, double value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_DOUBLE;
    document->fields[document->fieldCount - 1] = malloc(sizeof(double));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], &value, sizeof(double));
    return(true);
}

/**
 * Insert a string value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    String's value.
 *
 * \return true on success, false on error.
 */
bool bsonAddString(bsonDocument* document, char* name, char* value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_STRING;
    document->fields[document->fieldCount - 1] = calloc(strlen(value) + 1, sizeof(char));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    document->fields[document->fieldCount - 1] = strncpy(document->fields[document->fieldCount - 1], value, strlen(value));
    return(true);
}

/**
 * Insert a subdocument value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document.
 *
 * \return true on success, false on error.
 */
bool bsonAddSubDocument(bsonDocument* document, char* name, bsonDocument* value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_DOCUMENT;
    document->fields[document->fieldCount - 1] = value;
    return(true);
}

/**
 * Insert an array in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document (must be a Document with string numerical index).
 *
 * \return true on success, false on error.
 */
bool bsonAddArray(bsonDocument* document, char* name, bsonDocument* value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_ARRAY;
    document->fields[document->fieldCount - 1] = value;
    return(true);
}

/**
 * Insert a document id in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document id (12 bytes).
 *
 * \return true on success, false on error.
 */
bool bsonAddDocumentId(bsonDocument* document, char* name, void* value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_OBJECTID;
    document->fields[document->fieldCount - 1] = malloc(12);
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], value, 12);
    return(true);
}

/**
 * Insert a boolean value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddBool(bsonDocument* document, char* name, bool value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_BOOL;
    document->fields[document->fieldCount - 1] = malloc(sizeof(bool));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], &value, sizeof(bool));
    return(true);
}

/**
 * Insert a UTC Date value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value (UTC milliseconds since the Unix epoch).
 *
 * \return true on success, false on error.
 */
bool bsonAddDate(bsonDocument* document, char* name, bsonInt64 value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_UTCDATE;
    document->fields[document->fieldCount - 1] = malloc(sizeof(bsonInt64));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], &value, sizeof(bsonInt64));
    return(true);
}

/**
 * Insert a NULL value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 *
 * \return true on success, false on error.
 */
bool bsonAddNull(bsonDocument* document, char* name) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_NULL;
    document->fields[document->fieldCount - 1] = NULL;
    return(true);
}

/**
 * Insert a int32 value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddInt32(bsonDocument* document, char* name, bsonInt32 value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_INT32;
    document->fields[document->fieldCount - 1] = malloc(sizeof(bsonInt32));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], &value, sizeof(bsonInt32));
    return(true);
}

/**
 * Insert a int64 value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddInt64(bsonDocument* document, char* name, bsonInt64 value) {
    if (!strlen(name)) {
        return(false);
    }
    if (fieldNameExists(document, name)) {
        return(false);
    }
    _incrementCount(document);
    _appendFieldName(document, name);
    document->fieldTypes[document->fieldCount - 1] = BSON_TYPE_INT64;
    document->fields[document->fieldCount - 1] = malloc(sizeof(bsonInt64));
    if (!document->fields[document->fieldCount - 1]) {
        error("Could not allocate memory");
    }
    memcpy(document->fields[document->fieldCount - 1], &value, sizeof(bsonInt64));
    return(true);
}
