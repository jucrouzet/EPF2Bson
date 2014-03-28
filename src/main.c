/**
 * Main program file.
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
#include "epf.h"
#include "bson.h"
#include "error.h"


programOptions* epf2bsonOptions;
bool            epfRecoverableReadEmpty;

/**
 * Trim list elements.
 *
 * \param argument List element.
 */
void _trimListElement(char* argument) {
    size_t length = strlen(argument);
    size_t index = 0;
    char* copy;
    bool stringStarted = false;

    copy = calloc(length + 1, 1);
    if (!copy) {
        error("Cannot allocate memory");
    }
    for(size_t i = 0; i < length; i++) {
        if (!stringStarted && !isspace(argument[i])) {
            stringStarted = true;
            copy[index++] = argument[i];
        } else if (stringStarted) {
            copy[index++] = argument[i];
        }
    }
    for(size_t i = index; i >= 0; i--) {
        if (!copy[i] || isspace(copy[i])) {
            copy[i] = 0;
        } else {
            break;
        }
    }
    memcpy(argument, copy, length);
    free(copy);
}

/**
 * Parse and explode EPF collections list to char**.
 *
 * \param argument EPF list as string.
 *
 * \return String array.
 */
char** _epfListToArray(char* argument) {
    char* argCopy;
    char** list;
    char** parts;
    size_t argumentLength = strlen(argument);
    size_t listLength = 1;
    size_t index = 1;

    //message("%s", argument);
    for(size_t i = 0; i < argumentLength; i++) {
        if (argument[i] == ',') {
            listLength++;
        }
    }
    //message("%i", listLength);
    list = calloc(listLength + 1, sizeof(void*));
    parts = calloc(listLength + 1, sizeof(void*));
    argCopy = calloc(argumentLength + 3, sizeof(char));
    if (!list || !parts || !argCopy) {
        error("Cannot allocate memory");
    }
    strncpy(argCopy, argument, argumentLength);
    parts[0] = argCopy;
    for(size_t i = 0; i < argumentLength; i++) {
        if (argCopy[i] == ',') {
            parts[index++] = (argCopy + i + 1);
            argCopy[i] = 0;
        }
    }
    index = 0;
    for(size_t i = 0; parts[i]; i++) {
        int elementLength = strlen(parts[i]);
        char* listElement;

        if (elementLength) {
            listElement = calloc(elementLength + 1, sizeof(char));
            memcpy(listElement, parts[i], elementLength);
            _trimListElement(listElement);
            if (strlen(listElement)) {
                list[index++] = listElement;
            } else {
                free(listElement);
            }
        }
    }
    free(parts);
    free(argCopy);
    return(list);
}

/**
 * Parse command line arguments.
 *
 * \param argc Arguments count.
 * \param argv Arguments list.
 *
 * \return Parsed options.
 */
void _getOpt(int argc, char** argv) {
    const char* shortOptions;
    int verbose = 0;
    int optionsIndex = 0;
    int option;
    char* collectionList = NULL;

    epf2bsonOptions = calloc(1, sizeof(programOptions));
    if (!epf2bsonOptions) {
        error("Unable to allocate memory for options (#1)");
    }
    epf2bsonOptions->verbose = false;

    shortOptions = "ve:n:l:d:";
    struct option longOptions[] = {

        {"verbose",     no_argument,        &verbose,   1},

        {"epf",         required_argument,  0,          0},
        {"dbName",      required_argument,  0,          0},
        {"list",        required_argument,  0,          0},
        {"dumpdir",     optional_argument,  0,          0},

        {0,0,0,0}
    };
    while ((option = getopt_long(argc, argv, shortOptions, longOptions, &optionsIndex)) != -1) {
        switch (option) {
            case 'v' :
                epf2bsonOptions->verbose = true;
                break;
            case 'e' :
                epf2bsonOptions->epfDir = optarg;
                break;
            case 'n' :
                epf2bsonOptions->dbName = optarg;
                break;
            case 'l' :
                collectionList = optarg;
                break;
            case 'd' :
                epf2bsonOptions->dumpDir = optarg;
                break;
            case '?' :
                error("Missing argument or invalid option.");
                break;
            default:
                error("Unknown option '%c'", option);
                exit(EXIT_FAILURE);
        }
    }
    if (!epf2bsonOptions->dbName) {
        error("MongoDB database name is required");
    }
    if (!epf2bsonOptions->epfDir) {
        error("EPF files directory is required");
    }
    if (!epf2bsonOptions->dumpDir) {
        epf2bsonOptions->dumpDir = "dump";
    }
    if (!collectionList) {
        epf2bsonOptions->epfList = NULL;
    } else {
        epf2bsonOptions->epfList = _epfListToArray(collectionList);
    }
}

/**
 * Open EPF File.
 *
 * \param file File path.
 *
 * \return File handle.
 */
FILE* _openEPFFile(char* file) {
    FILE* fp;
    struct stat statBuf;

    if (stat(file, &statBuf) == -1) {
        error("EPF File does not exists : %s", file);
    }
    if(!S_ISREG(statBuf.st_mode)) {
        error("EPF File is not a regular file : %s", file);
    }
    if (epf2bsonOptions->verbose) {
        message("EPF File size: %'d bytes", statBuf.st_size);
    }
    errno = 0;
    fp = fopen(file, "r");
    if (fp == 0 && errno != 0) {
        error("Error opening EPF file (%s) : %s", strerror(errno), file);
    }
    return(fp);
}

/**
 * Checks epf files dir and stores realpath.
 */
void _checkEpfDir() {
    char* path;
    char* realPath;
    struct stat statBuffer;

    path = calloc(PATH_MAX + 1, sizeof(char));
    realPath = calloc(PATH_MAX + 1, sizeof(char));
    if (epf2bsonOptions->epfDir[0] != '/') {
        if (getcwd(path, PATH_MAX) == NULL) {
           error("getcwd() error : %s", strerror(errno));
        }
        strcat(path, "/");
        strcat(path, epf2bsonOptions->epfDir);
    } else {
        strcpy(path, epf2bsonOptions->epfDir);
    }
    realPath = realpath(path, realPath);
    if (!realPath) {
        error("Error opening EPF files directory (%s) : %s", strerror(errno), path);
    }
    free(path);
    epf2bsonOptions->epfDir = realPath;
    if(stat(epf2bsonOptions->epfDir, &statBuffer) != -1) {
        if (!S_ISDIR(statBuffer.st_mode)) {
            error("EPF files directory is not a directory");
        } else if(access(epf2bsonOptions->epfDir, R_OK) == -1) {
            error("Cannot read in EPF files directory");
        }
    } else {
        error("Error opening EPF files directory (%s)", strerror(errno));
    }
}

/**
 * Checks and create if necessary dump dir and stores realpath.
 */
void _checkDumpDir() {
    char* baseName;
    char* copy;
    char* copy2;
    char* path;
    char* realPath;
    struct stat statBuffer;

    copy = strdup(epf2bsonOptions->dumpDir);
    if (!copy) {
        error("Cannot allocate memory");
    }
    if (copy[strlen(copy) - 1] == '/') {
        copy[strlen(copy) - 1] = 0;
    }
    copy2 = strdup(copy);
    if (!copy2) {
        error("Cannot allocate memory");
    }
    baseName = basename(copy2);
    path = calloc(PATH_MAX + 1, sizeof(char));
    realPath = calloc(PATH_MAX + 1, sizeof(char));
    if (!path || !realPath) {
        error("Cannot allocate memory");
    }
    if (copy[0] != '/') {
        if (getcwd(path, PATH_MAX - 1) == NULL) {
           error("getcwd() error : %s", strerror(errno));
        }
        strcat(path, "/");
        strcat(path, copy);
    } else {
        strcpy(path, copy);
    }
    realPath = realpath(dirname(path), realPath);
    if(!realPath) {
        error("Cannot access to dump directory parent (%s) : %s", strerror(errno), path);
    }
    free(path);
    if(access(realPath, W_OK) == -1) {
        error("Cannot write to dump directory parent (%s) : %s", strerror(errno), realPath);
    }
    strcat(realPath, "/");
    strcat(realPath, baseName);
    if(stat(realPath, &statBuffer) != -1) {
        error("Dump directory already exists : %s.", realPath);
    } else if (errno != ENOENT) {
        error("Cannot create dump directory (%s) : %s", strerror(errno), path);
    } else {
        if(mkdir(realPath, 0755)) {
            error("Cannot create dump directory (%s) : %s", strerror(errno), path);
        }
    }
    strcat(realPath, "/");
    strcat(realPath, epf2bsonOptions->dbName);
    if(mkdir(realPath, 0755)) {
        error("Cannot create dump directory (%s) : %s", strerror(errno), path);
    }
    free(copy);
    free(copy2);
    epf2bsonOptions->dumpDir = realPath;
}

/**
 * Write an epf file as bson.
 *
 * \param epfFile   EPF File instance.
 * \param bsonFile  BSON file path.
 */
void _writeEpfInBson(EPFFile* epfFile, char* bsonFile) {
    FILE* bson;
    bsonDocument* doc;
    bsonSerializedValue serialized;
    char** entry;
    size_t i = 0;
    long j = 0;
    bsonInt64 i64Value;
    bsonInt32 i32Value;
    bsonDouble doubleValue;

    message("Exporting to BSON file: %s", bsonFile);
    bson = fopen(bsonFile, "w");
    if (!bson) {
        error("Could not create file (%s) : %s", strerror(errno), bsonFile);
    }
    while(
            (entry = epfNextEntry(epfFile)) ||
            epfRecoverableReadEmpty
    ) {
        if (epfRecoverableReadEmpty) {
            continue;
        }
        i = 0;
        doc = createBsonDocument();
        while(i < epfFile->fieldsCount) {
            if (!entry[i]) {
                break;
            }
            if (!strlen(entry[i])) {
                bsonAddNull(doc, epfFile->fields[i]->fieldName);
            } else {
                switch(epfGetFieldType(epfFile, i)) {
                    case EPF_FIELDTYPE_BIGINT :
                    case EPF_FIELDTYPE_INTEGER :
                        i64Value = strtol(entry[i], NULL, 10);
                        if (i64Value >= INT_MIN && i64Value <= INT_MAX) {
                            i32Value = i64Value;
                            bsonAddInt32(doc, epfFile->fields[i]->fieldName, i32Value);
                        } else {
                            bsonAddInt64(doc, epfFile->fields[i]->fieldName, i64Value);
                        }                    
                        break;
                    case EPF_FIELDTYPE_BOOLEAN :
                        if (!strncmp("0", entry[i], 1)) {
                            bsonAddBool(doc, epfFile->fields[i]->fieldName, false);
                        } else {
                            bsonAddBool(doc, epfFile->fields[i]->fieldName, true);
                        }
                        break;
                    case EPF_FIELDTYPE_VARCHAR :
                    case EPF_FIELDTYPE_LONGTEXT :
                        bsonAddString(doc, epfFile->fields[i]->fieldName, entry[i]);
                        break;
                    case EPF_FIELDTYPE_DATETIME :
                        i64Value = strtol(entry[i], NULL, 10);
                        i64Value *= 1000;
                        bsonAddDate(doc, epfFile->fields[i]->fieldName, i64Value);
                        break;
                    case EPF_FIELDTYPE_DECIMAL :
                        doubleValue = strtod(entry[i], NULL);
                        bsonAddDouble(doc, epfFile->fields[i]->fieldName, doubleValue);
                        break;
                    case 0:
                    default :
                        error("Unknown EPF field type, aborting");
                        break;
                }
            }
            free(entry[i]);
            i++;
        }

        free(entry);
        serialized = bsonSerialize(doc);

        fwrite(serialized.binaryValue, 1, serialized.length, bson);

        free(serialized.binaryValue);
        destroyBsonDocument(doc);

        if (j && !(j % 10000)) {
            message("Exported %'li entries.", j);
        }
        j++;
    }
    message("Exported %li entries.", j);
    fclose(bson);
}


/**
 * Tells if a collection file is to parse.
 *
 * \return True if file is to parse, false elsewhere.
 */
bool _collectionsIsToParse(char* filePath) {
    char* baseName;

    if (!epf2bsonOptions->epfList) {
        return(true);
    }
    baseName = basename(filePath);
    for(size_t i = 0; epf2bsonOptions->epfList[i]; i++) {
        if (!strcmp(epf2bsonOptions->epfList[i], baseName)) {
            return(true);
        }
    }
    return(false);
}


/**
 * Get EPF files list to parse.
 *
 * \return Files path to parse.
 */
char** _getCollectionsList() {
    char* wildcard;
    int globResult;
    char **filesList;
    size_t index = 0;
    glob_t glob_results;

    wildcard = calloc(strlen(epf2bsonOptions->epfDir) + 3, sizeof(char));
    if (!wildcard) {
        error("Cannot allocate memory");
    }
    strcpy(wildcard, epf2bsonOptions->epfDir);
    strcat(wildcard, "/*");
    globResult = glob(wildcard, GLOB_MARK, 0, &glob_results);
    free(wildcard);
    if (globResult || glob_results.gl_pathc <= 0) {
        switch(globResult) {
            case GLOB_NOMATCH:
                error("EPF files directory seems empty");
                break;
            case GLOB_NOSPACE:
                error("Not enough memory to accomodate expanded filenames in EPF files directory");
                break;
            default:
                error("Unknown error while searching EPF files");
        }
    }
    index = 0;
    filesList = calloc(glob_results.gl_pathc + 1, sizeof(void*));
    if (!filesList) {
        error("Cannot allocate memory");
    }
    for(size_t i = 0; i < glob_results.gl_pathc; i++) {
        size_t strLen = strlen(glob_results.gl_pathv[i]);

        if (
                (glob_results.gl_pathv[i][strLen - 1] != '/') &&
                _collectionsIsToParse(glob_results.gl_pathv[i])
        ) {
            filesList[index] = calloc(strLen + 1, sizeof(char));
            if (!filesList[index]) {
                error("Cannot allocate memory");
            }
            memcpy(filesList[index], glob_results.gl_pathv[i], strLen);
            index++;
        }
    }
    for(size_t i = 0; epf2bsonOptions->epfList[i]; i++) {
        free(epf2bsonOptions->epfList[i]);
    }
    free(epf2bsonOptions->epfList);
    globfree(&glob_results);
    return(filesList);
}

/**
 * Generate the bson file path for a given EPF file.
 *
 * \param epfFile EPF File path.
 *
 * \return File path.
 */
char* _getBsonFilePath(char* epfFile) {
    char* bsonPath;
    char* copy;

    copy = strdup(epfFile);
    epfFile = basename(copy);
    bsonPath = calloc(strlen(epf2bsonOptions->dumpDir) + strlen(epfFile) + 7, sizeof(char));
    if (!bsonPath) {
        error("Cannot allocate memory");
    }
    strcpy(bsonPath, epf2bsonOptions->dumpDir);
    strcat(bsonPath, "/");
    strcat(bsonPath, epfFile);
    strcat(bsonPath, ".bson");
    free(copy);
    return(bsonPath);
}

/**
 * Generate the metadata json file path for a given EPF file.
 *
 * \param epfFile EPF File path.
 *
 * \return File path.
 */
char* _getMetaFilePath(char* epfFile) {
    char* jsonPath;
    char* copy;

    copy = strdup(epfFile);
    epfFile = basename(copy);
    jsonPath = calloc(strlen(epf2bsonOptions->dumpDir) + strlen(epfFile) + 16, sizeof(char));
    if (!jsonPath) {
        error("Cannot allocate memory");
    }
    strcpy(jsonPath, epf2bsonOptions->dumpDir);
    strcat(jsonPath, "/");
    strcat(jsonPath, epfFile);
    strcat(jsonPath, ".metadata.json");
    free(copy);
    return(jsonPath);
}

/**
 * Export EPF index as Mongo metadata json file.
 *
 * \param epfFile     EPF File instance.
 * \param epfFilePath EPF file path.
 * \param jsonFile    JSON file path.
 */
void _writeMetadataInJson(EPFFile* epfFile, char* epfFilePath, char* jsonFile) {
    char* jsonData = calloc(1024, sizeof(char));
    size_t allocated = 1024;
    size_t index = 0;
    size_t i = 0;
    size_t count = 0;
    FILE* json;    
    char* indexFormat = "{\"ns\" : \"%s.%s\", \"name\" : \"_EPF2Bson_%s_\", \"v\" : 1, \"key\" : { \"%s\" : 1 } },";
    char* copy;
    char* collectionName;

    message("Exporting to metadata JSON file: %s", jsonFile);

    copy = strdup(epfFilePath);
    collectionName = basename(copy);

    if (!jsonData) {
        error("Cannot allocate memory");
    }
    strcat(jsonData, "{\"indexes\" : [ ");
    while(i < epfFile->fieldsCount) {
        int entryLength;
        char* entry;

        if (epfFile->fields[i]->indexed) {
            entryLength = strlen(indexFormat);
            entryLength += strlen(epf2bsonOptions->dbName);
            entryLength += strlen(collectionName);
            entryLength += strlen(epfFile->fields[i]->fieldName);
            entryLength += strlen(epfFile->fields[i]->fieldName);
            entry = calloc(entryLength + 1, sizeof(char));
            sprintf(
                entry,
                indexFormat,
                epf2bsonOptions->dbName,
                collectionName,
                epfFile->fields[i]->fieldName,
                epfFile->fields[i]->fieldName
            );
            while(index + entryLength > allocated) {
                allocated += 1024;
                jsonData = realloc(jsonData, allocated);
                if (!jsonData) {
                    error("Cannot allocate memory");
                }
            }
            strcat(jsonData, entry);
            free(entry);
            count++;
        }
        i++;
    }
    if (jsonData[strlen(jsonData) - 1] == ',') {
        jsonData[strlen(jsonData) - 1] = 0;
    }
    strcat(jsonData, " ] }");

    json = fopen(jsonFile, "w");
    if (!json) {
        error("Could not create file (%s) : %s", strerror(errno), jsonFile);
    }
    fwrite(jsonData, sizeof(char), strlen(jsonData), json);
    fclose(json);

    free(jsonData);
    free(copy);

    message("Exported %i indexe(s)", count);
}

/**
 * Validates MongoDB db name is valid.
 */
void _checkDbName() {
    size_t length = strlen(epf2bsonOptions->dbName);

    if (!length || length > 256) {
        error("Empty MongoDB database name");
    }
    for(size_t i = 0; i < length; i++) {
        char character = epf2bsonOptions->dbName[i];

        if (
            !((character >= '0') && (character <= '9')) &&
            !((character >= 'a') && (character <= 'z')) &&
            !((character >= 'A') && (character <= 'Z')) &&
            !(character == '-') &&
            !(character == '_')
        ) {
            error("Invalid MongoDB database name");
        }
    }    
}

/**
 * Entry point.
 *
 * \param argc Arguments count.
 * \param argv Arguments list.
 *
 * \return Status code.
 */
int main(int argc, char** argv) {
    FILE* fp;
    EPFFile* epfFile;
    char** files;
    char* bsonFile;
    char* jsonFile;

    setlocale(LC_ALL, "en_US.utf-8");

    _getOpt(argc, argv);
    _checkDbName();
    _checkEpfDir();
    _checkDumpDir();

    files = _getCollectionsList();

    for(size_t i = 0; files[i]; i++) {
        fp = _openEPFFile(files[i]);
        bsonFile = _getBsonFilePath(files[i]);
        jsonFile = _getMetaFilePath(files[i]);

        message("Parsing EPF File: %s", files[i]);
        epfFile = epfInit(fp);
        message("Parsed !");

        _writeEpfInBson(epfFile, bsonFile);
        _writeMetadataInJson(epfFile, files[i], jsonFile);

        epfDestroy(epfFile);
        fclose(fp);
        free(files[i]);
        free(bsonFile);
        free(jsonFile);
    }
    free(files);
    free(epf2bsonOptions->epfDir);
    free(epf2bsonOptions->dumpDir);
    free(epf2bsonOptions);
    return (EXIT_SUCCESS);
}

