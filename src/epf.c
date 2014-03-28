/**
 * EPF Files parser.
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

#include "EPF2Bson.h"
#include "error.h"
#include "epf.h"

/**
 * Reads next record in EPF File.
 *
 * \param file EPFFile instance.
 *
 * \return Record as string or NULL is none (EOF).
 */
char* _readRecord(EPFFile* file) {
    int bufferSize = 4096;
    char* record = calloc(bufferSize, sizeof(char));
    char character, lastCharacter;
    int recordIndex = 0;
    int alocatedBuffer = 1;
    

    if (bufferSize < 2) {
        bufferSize = 1024;
    }
    if (!file->fp) {
        error("Could not read record in file (#100)");
    }
    if (!record) {
        error("Could not allocate memory read record in file (#101)");
    }
    file->lastEntryOffset = ftell(file->fp);
    while ((character = fgetc(file->fp)) != EOF) {
        if (recordIndex && ((recordIndex % bufferSize) == 0)) {
            char* newBuffer = calloc(++alocatedBuffer * bufferSize, sizeof(char));

            if (!newBuffer) {
                error("Could not allocate memory read record in file (#102)");
            }
            newBuffer = strcpy(newBuffer, record);
            free(record);
            record = newBuffer;
        }
        //http://www.apple.com/itunes/affiliates/resources/documentation/itunes-enterprise-partner-feed.html#fileformat
        //$record_separator = chr(2) . "\n"
        if ((character == 10) && (lastCharacter == 2)) {
            break;
        }
        record[recordIndex] = character;
        recordIndex++;
        lastCharacter = character;
    }
    if (character == EOF) {
        return(NULL);
    }
    file->readLines++;
    return(record);
}

/**
 * Get next record in file.
 *
 * \param file EPFFile instance.
 *
 * \return Record fields or NULL is none (EOF).
 */
char** _getNextRecord(EPFFile* file) {
    char* record = _readRecord(file);
    char** fields;
    void** separatorMarkers;
    size_t position = 0;
    size_t countedFields = 0;
    bool commentField = false;

    epfRecoverableReadEmpty = false;
    if (!record) {
        return(NULL);
    }
    separatorMarkers = calloc(((file->fieldsCount == -1) ? 1024 : file->fieldsCount + 1), sizeof(void*));
    if (!separatorMarkers) {
        error("Could not allocate memory");
    }
    if (!strlen(record)) {
        return(NULL);
    }
    if (record[0] == '#') {
        commentField = true;
        record++;
    }
    separatorMarkers[0] = record;
    for(countedFields = 0; record[position] != 0; position++) {
        if (record[position] == EPFSeparator) {
            separatorMarkers[countedFields + 1] = record + position + 1;
            countedFields++;
        }
    }
    if (file->fieldsCount != -1) {
        if (!commentField && ((countedFields + 1) != file->fieldsCount)) {
            warning("Invalid field count (#201) : %i - %s", countedFields, record);
            epfRecoverableReadEmpty = true;
            return(NULL);
        }
    } else {
        file->fieldsCount = countedFields + 1;
    }
    fields = calloc(file->fieldsCount + 1, sizeof(void*));
    if (!fields) {
        error("Could not allocate memory storing record fields (#200)");
    }
    if (countedFields) {
        for(size_t i = 0; ((i <= file->fieldsCount) && (i <= countedFields)); i++) {
            size_t stringLength;

            if (i == countedFields) {
                stringLength = strlen(separatorMarkers[i]);
            } else {
                stringLength = (separatorMarkers[i + 1] - separatorMarkers[i]) / sizeof(char);
            }
            fields[i] = calloc((stringLength + 1), sizeof(char));
            if (!fields[i]) {
                error("Could not allocate memory");
            }
            strncpy(fields[i], separatorMarkers[i], stringLength - 1);
        }
    } else {
        fields[0] = calloc(strlen(record), sizeof(char));
        if (!fields[0]) {
            error("Could not allocate memory");
        }
        strncpy(fields[0], record, strlen(record) - 1);
    }
    free(separatorMarkers);
    if (commentField) {
        free(record - 1);
    } else {
        free(record);
    }
    return(fields);
}

/**
 * Reads the first line aka field names.
 *
 * \param file EPFFile instance.
 *
 */
void _parseFieldNames(EPFFile* file) {
    char** fieldNames = NULL;
    EPFField** fields;
    unsigned int i = 0;

    if (file->readLines != 0) {
        error("Field names should be the first line (#300)");
    }
    fieldNames = _getNextRecord(file);
    if (!fieldNames) {
        error("Premature end of file (#302)");
    }
    while(fieldNames[i]) {
        if (strstr(fieldNames[i], "\n")) {
            error("Header records should not contain a new line, probably not an EPF File");
        }
        i++;
    }
    if (i < 1) {
        error("No field name defined, probably not an EPF File");
    }
    file->fieldsCount = i;
    fields = malloc(i * sizeof(EPFField*));
    if (!fields) {
        error("Could not allocate memory");
    }
    i = 0;
    while(fieldNames[i]) {
        EPFField* field = calloc(1, sizeof(EPFField));

        if (!field) {
            error("Could not allocate memory (#301)");
        }
        if (epf2bsonOptions->verbose) {
        	message("Declared field :%s", fieldNames[i]);
        }
        field->fieldName = fieldNames[i];
        fields[i] = field;
        i++;
    }
    free(fieldNames);
    file->fields = fields;
}

/**
 * Reads the second line aka indexed fields.
 *
 * \param file EPFFile instance.
 */
void _parseIndexedFields(EPFFile* file) {
    char** fields = NULL;
    char* correctedFirstField;
    int i = 0;

    if (file->readLines != 1) {
        error("Indexed fields names should be the second line (#400)");
    }
    fields = _getNextRecord(file);
    if (!fields) {
        error("Premature end of file (#402)");
    }
    if (
        !fields[0] ||
        (strlen(fields[0]) < 11) ||
        strncmp(fields[0], "primaryKey:", 11)
    ) {
        error("Invalid primaryKey record, probably not an EPF File");
    }
    correctedFirstField = calloc((strlen(fields[0]) - 11) + 1, sizeof(char));
    if (!correctedFirstField) {
        error("Could not allocate memory (#401)");
    }
    strncpy(correctedFirstField, fields[0] + 11, (strlen(fields[0]) - 11));
    free(fields[0]);
    fields[0] = correctedFirstField;
    while(fields[i]) {
        if (strstr(fields[i], "\n")) {
            error("Header records should not contain a new line, probably not an EPF File");
        }
        for(int j = 0; j < file->fieldsCount; j++) {
            if (!strcmp(file->fields[j]->fieldName, fields[i])) {
                file->fields[j]->indexed = true;
		        if (epf2bsonOptions->verbose) {
		        	message("Field '%s' is indexed", file->fields[j]->fieldName);
		        }
                break;
            }
        }
        free(fields[i]);
        i++;
    }
    free(fields);
}

/**
 * Reads the third line aka fields type.
 *
 * \param file EPFFile instance.
 */
void _parseFieldsType(EPFFile* file) {
    char** fields = NULL;
    char* correctedFirstField;
    char* capacitedTypeName;
    size_t i = 0;
    unsigned int capacity;
    int scanRet;

    if (file->readLines != 2) {
        error("Field types should be the third line (#500)");
    }
    fields = _getNextRecord(file);
    if (!fields) {
        error("Premature end of file (#502)");
    }
    if (
        !fields[0] ||
        (strlen(fields[0]) < 8) ||
        strncmp(fields[0], "dbTypes:", 8)
    ) {
        error("Invalid dbTypes record, probably not an EPF File");
    }
    correctedFirstField = calloc((strlen(fields[0]) - 8) + 1, sizeof(char));
    if (!correctedFirstField) {
        error("Could not allocate memory (#501)");
    }
    strncpy(correctedFirstField, fields[0] + 8, (strlen(fields[0]) - 8));
    free(fields[0]);
    fields[0] = correctedFirstField;
    while(fields[i]) {
        if (strstr(fields[i], "\n")) {
            error("Header records should not contain a new line, probably not an EPF File");
        }
        capacitedTypeName = calloc(strlen(fields[i]) + 1, sizeof(char));
        if (!capacitedTypeName) {
            error("Could not allocate memory");
        }
        if ((scanRet = sscanf(fields[i], "%[^(](%d,%d)", capacitedTypeName, &capacity, &capacity)) == 3) {
            capacity = 1;
        } else if ((scanRet = sscanf(fields[i], "%[^(](%d)", capacitedTypeName, &capacity)) != 2) {
            capacitedTypeName = strcpy(capacitedTypeName, fields[i]);
            capacity = 1;
        }
        if (epf2bsonOptions->verbose) {
        	message("Field '%s' is declared as %s", file->fields[i]->fieldName, fields[i]);
        }
        free(fields[i]);
        if (!strncmp(capacitedTypeName, "BIGINT", 6)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_BIGINT;
        } else if (!strncmp(capacitedTypeName, "INTEGER", 7)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_INTEGER;
        } else if (!strncmp(capacitedTypeName, "BOOLEAN", 7)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_BOOLEAN;
        } else if (!strncmp(capacitedTypeName, "VARCHAR", 7)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_VARCHAR;
        } else if (!strncmp(capacitedTypeName, "DATETIME", 8)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_DATETIME;
        } else if (!strncmp(capacitedTypeName, "LONGTEXT", 8)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_LONGTEXT;
        } else if (!strncmp(capacitedTypeName, "DECIMAL", 7)) {
            file->fields[i]->fieldType = EPF_FIELDTYPE_DECIMAL;
        } else {
            error("Invalid field type : %s", capacitedTypeName);
        }
        free(capacitedTypeName);
        file->fields[i]->capacity = capacity;
        i++;
    }
    free(fields);
    if (i != file->fieldsCount) {
        error("Fields type count is not equal to fields count, aborting.");
    }
}

/**
 * Reads the fourth line aka export mode (full / incremental).
 *
 * \param file EPFFile instance.
 */
void _parseExportMode(EPFFile* file) {
    char** fields = NULL;
    int i = 0;

    if (file->readLines != 3) {
        error("Export mode should be the fourth line (#600)");
    }
    fields = _getNextRecord(file);
    if (!fields) {
        error("Premature end of file (#601)");
    }
    if (
        !fields[0] ||
        (strlen(fields[0]) < 11) ||
        strncmp(fields[0], "exportMode:", 11)
    ) {
        error("Invalid export mode, probably not an EPF File");
    }
    if (!strncmp(fields[0], "exportMode:FULL", 15)) {
        file->incremental = false;
        if (epf2bsonOptions->verbose) {
        	message("Full export mode declared");
        }
    } else if (!strncmp(fields[0], "exportMode:INCREMENTAL", 22)) {
        if (epf2bsonOptions->verbose) {
        	message("Incremental export mode declared");
        }
        file->incremental = true;
    } else {
        error("Unknown export mode");
    }
    while(fields[i]) {
        free(fields[i++]);
    }
    free(fields);
}

/**
 * Skip all the following comment lines (Like ##LEGAL, etc.).
 *
 * \param file EPFFile instance.
 */
void _parseSkipComments(EPFFile* file) {
    char* record = NULL;

    if (file->readLines != 4) {
        error("Comments lines should be after the fourth line (#700)");
    }
    while ((record = _readRecord(file))) {
        if (strncmp(record, "##", 2)) {
            fseek(file->fp, file->lastEntryOffset, SEEK_SET);
            free(record);
            break;
        }
        free(record);
    }
}



/**
 * Read EPF file header to get file infos and return an instance of EPFFile.
 *
 * \param fp File pointer to EPF file.
 *
 * \return Parsed data.
 */
EPFFile* epfInit(FILE* fp) {
    EPFFile* file;

    file = malloc(sizeof(EPFFile));
    if (!file) {
        error("Could not allocate memory");
    }
    file->fp = fp;
    file->fieldsCount = -1;
    file->readLines = file->readEntries = 0;
    _parseFieldNames(file);
    _parseIndexedFields(file);
    _parseFieldsType(file);
    _parseExportMode(file);
    _parseSkipComments(file);
    file->ready = true;
    return(file);
}

/**
 * Get an entry from collection.
 *
 * \param file EPFFile instance.
 *
 * \return Raw entry data (as strings) or null if EOF.
 */
char** epfNextEntry(EPFFile* file) {
    
    if (!file->ready) {
        error("EPF File is not initialized");
    }
    return(_getNextRecord(file));
}

/**
 * Get field type.
 *
 * \param file EPFFile instance.
 * \param index Field index.
 *
 * \return Field type or 0 on error.
 */
unsigned char epfGetFieldType(EPFFile* file, size_t index) {

    if (index > file->fieldsCount) {
        return(0);
    }
    return(file->fields[index]->fieldType);
}

/**
 * Get field capacity.
 *
 * \param file EPFFile instance.
 * \param index Field index.
 *
 * \return Field type.
 */
size_t epfGetFieldCapacity(EPFFile* file, size_t index) {
    
    if (index > file->fieldsCount) {
        return(0);
    }
    return(file->fields[index]->capacity);
}


/**
 * Destroy an EPF file object and release memory.
 *
 * \param file EPFFile instance.
 */
void epfDestroy(EPFFile* file) {
    
    for (int i = 0; i < file->fieldsCount; i++) {
        if (file->fields[i]) {
            free(file->fields[i]->fieldName);
            free(file->fields[i]);
        }
    }
    free(file->fields);   
    free(file);
}
