/**
 * BSON Format.
 *
 * @author              Julien CROUZET <contact@juliencrouzet.fr>
 * @copyright           Julien CROUZET <contact@juliencrouzet.fr>
 */

/**
    This file is part of BSON2Bson.

    BSON2Bson is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BSON2Bson is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BSON2Bson. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _BSON_H_INCLUDED_
#define _BSON_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <stdbool.h>
#include <endian.h>
#include <iconv.h>

/**
 * Base types.
 */
typedef int8_t  bsonByte;
typedef int32_t bsonInt32;
typedef int64_t bsonInt64;
typedef double  bsonDouble;


/**
 * Values types.
 */
#define BSON_TYPE_DOUBLE            '\x01'
#define BSON_TYPE_STRING            '\x02'
#define BSON_TYPE_DOCUMENT          '\x03'
#define BSON_TYPE_ARRAY             '\x04'
//Not used in export/import
//#define BSON_TYPE_binary          '\x05';
//Deprecated
//#define BSON_TYPE_UNDEFINED       '\x06';
#define BSON_TYPE_OBJECTID          '\x07'
#define BSON_TYPE_BOOL              '\x08'
#define BSON_TYPE_UTCDATE           '\x09'
#define BSON_TYPE_NULL              '\x0A'
//Not used in export/import
//#define BSON_TYPE_REGEX           '\x0B'
//Deprecated
//#define BSON_TYPE_DBPOINTER       '\x0C';
//Not used in export/import
//#define BSON_TYPE_JSCODE          '\x0D'
//Deprecated
//#define BSON_TYPE_SYMBOL          '\x0E';
//Not used in export/import
//#define BSON_TYPE_JSCODEWITHSCOPE '\x0F'
#define BSON_TYPE_INT32             '\x10'
//Internal
//#define BSON_TYPE_TIMESTAMP       '\x11'
#define BSON_TYPE_INT64             '\x12'
//Internal
//#define BSON_TYPE_MINKEY          '\xFF'
//Internal
//#define BSON_TYPE_MAXKEY          '\x7F'

/**
 * Bson document.
 */
typedef struct bsonDocument {
    /**
     * Document field count.
     */
    size_t fieldCount;
    /**
     * Document field name.
     */
    char** fieldNames;
    /**
     * Document field types.
     */
    bsonByte* fieldTypes;
    /**
     * Document field values.
     */
    void** fields;
    /**
     * Last allocations size (internal).
     */
    size_t _lastAllocationsSize;
} bsonDocument;

/**
 * Bson serialized value.
 */
typedef struct bsonSerializedValue {
    /**
     * Binary value.
     */
    void* binaryValue;
    /**
     * Value length.
     */
    size_t length;
} bsonSerializedValue;


/**
 * Creates a new BSON document.
 *
 * \return Document.
 */
bsonDocument* createBsonDocument();

/**
 * Destroys a bson document instance.
 *
 * \param document Document to destroy.
 */
void destroyBsonDocument(bsonDocument* document);

/**
 * Checks if given field name already exists in document.
 *
 * \param document Document to insert into.
 * \param name     Field name.
 *
 * \return True if exists, false elsewhere.
 */
bool fieldNameExists(bsonDocument* document, char* name);

/**
 * Serialize document as BSON.
 *
 * \param document Document to insert into.
 *
 * \return BSON serialized. (`binaryValue` must be free()'d after use).
 */
bsonSerializedValue bsonSerialize(bsonDocument* document);

/**
 * Insert a double value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Double's value.
 *
 * \return true on success, false on error.
 */
bool bsonAddDouble(bsonDocument* document, char* name, double value);

/**
 * Insert a string value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    String's value.
 *
 * \return true on success, false on error.
 */
bool bsonAddString(bsonDocument* document, char* name, char* value);

/**
 * Insert a subdocument value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document.
 *
 * \return true on success, false on error.
 */
bool bsonAddSubDocument(bsonDocument* document, char* name, bsonDocument* value);

/**
 * Insert an array in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document (must be a Document with string numerical index).
 *
 * \return true on success, false on error.
 */
bool bsonAddArray(bsonDocument* document, char* name, bsonDocument* value);

/**
 * Insert a document id in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Document id (12 bytes).
 *
 * \return true on success, false on error.
 */
bool bsonAddDocumentId(bsonDocument* document, char* name, void* value);

/**
 * Insert a boolean value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddBool(bsonDocument* document, char* name, bool value);

/**
 * Insert a UTC Date value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value (UTC milliseconds since the Unix epoch).
 *
 * \return true on success, false on error.
 */
bool bsonAddDate(bsonDocument* document, char* name, bsonInt64 value);
/**
 * Insert a NULL value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 *
 * \return true on success, false on error.
 */
bool bsonAddNull(bsonDocument* document, char* name);

/**
 * Insert a int32 value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddInt32(bsonDocument* document, char* name, bsonInt32 value);

/**
 * Insert a int64 value in document.
 *
 * \param document Document to insert into.
 * \param name     Value name in document.
 * \param value    Value.
 *
 * \return true on success, false on error.
 */
bool bsonAddInt64(bsonDocument* document, char* name, bsonInt64 value);


#endif /* _BSON_H_INCLUDED_ */