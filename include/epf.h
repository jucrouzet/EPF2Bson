/**
 * Itunes EPF includes.
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


#ifndef _EPF_H_INCLUDED_
#define _EPF_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define EPFSeparator                '\x01'

#define EPF_FIELDTYPE_BIGINT        1
#define EPF_FIELDTYPE_INTEGER       2
#define EPF_FIELDTYPE_BOOLEAN       3
#define EPF_FIELDTYPE_VARCHAR       4
#define EPF_FIELDTYPE_DATETIME      5
#define EPF_FIELDTYPE_LONGTEXT      6
#define EPF_FIELDTYPE_DECIMAL       7



/**
 * EPF Field.
 */
typedef struct EPFField {
    /**
     * Field name.
     */
    char* fieldName;
    /**
     * Field type.
     */
    unsigned char fieldType;
    /**
     * Field capacity (EG: Varchar).
     */
    size_t capacity;
    /**
     * Field is indexed.
     */
    bool indexed;
} EPFField;



/**
 * EPF File informations.
 */
typedef struct EPFFile {
    /**
     * Fields.
     */
    EPFField** fields;
    /**
     * Field count.
     */
    size_t fieldsCount;
    /**
     * Is incremental export.
     */
    bool incremental;
    /**
     * EPF file pointer.
     */
    FILE* fp;
    /**
     * Lines read so far.
     */
    unsigned long readLines;
    /**
     * EPF Collection entries read so far.
     */
    unsigned long readEntries;
    /**
     * Start offset of last entry.
     */
    unsigned long lastEntryOffset;
    /**
     * File header is parsed and ready to read.
     */
    bool ready;
} EPFFile;


/**
 * Read EPF file header to get file infos and return an instance of EPFFile.
 *
 * \param fp File pointer to EPF file.
 *
 * \return Parsed data.
 */
EPFFile* epfInit(FILE* fp);

/**
 * Get an entry from collection.
 *
 * \param file EPFFile instance.
 *
 * \return Raw entry data (as strings) or null if EOF.
 */
char** epfNextEntry(EPFFile* file);

/**
 * Get field type.
 *
 * \param file EPFFile instance.
 * \param index Field index.
 *
 * \return Field type.
 */
unsigned char epfGetFieldType(EPFFile* file, size_t index);

/**
 * Get field capacity.
 *
 * \param file EPFFile instance.
 * \param index Field index.
 *
 * \return Field type.
 */
size_t epfGetFieldCapacity(EPFFile* file, size_t index);

/**
 * Destroy an EPF file object and release memory.
 *
 * \param file EPFFile instance.
 */
void epfDestroy(EPFFile* file);



#endif /* _EPF_H_INCLUDED_ */