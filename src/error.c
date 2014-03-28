/**
 * Error handling.
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

/**
 * Show usage.
 */
void usage() {
    fputs("Usage: EPF2Bson [arguments]\n\n", stderr);
    fputs("\t-v --verbose                       Run program in verbose mode\n", stderr);
    fputs("\n", stderr);
    fputs("\t-e --epf       <directory>     EPF files directory.\n", stderr);
    fputs("\t-n --dbName    <name>          MongoDB database name to dump for.\n", stderr);
    fputs("\t-d --dumpdir   <path>          NON EXISTANT dump directory path to export to. Defaults to './dump'\n", stderr);
    fputs("\t-l --list      <list>          List of EPF collections (comma separated) to export. Defaults to all\n", stderr);
    fputs("\n", stderr);
    fputs("\n\n", stderr);
}

/**
 * Shows an error.
 *
 * \param error Error message.
 * \param ... printf() like variable.
 */
void error(const char* format, ...) {
    va_list varArgs;
    char* newFormat;
    const char* prepend = "\n\n\t[ERROR][EPF2Bson] : ";

    newFormat = calloc((strlen(format) + strlen(prepend) + 3), (sizeof(char)));
    newFormat = strncpy(newFormat, prepend, strlen(prepend));
    newFormat = strncat(newFormat, format, strlen(format));
    newFormat = strncat(newFormat, "\n\n", 2);
    va_start(varArgs, format);
    vfprintf(stderr, newFormat, varArgs);
    va_end(varArgs);
    free(newFormat);
    free(epf2bsonOptions);
    usage();
    exit(EXIT_FAILURE);
}

/**
 * Shows a warnning.
 *
 * \param error Error message.
 * \param ... printf() like variable.
 */
void warning(const char* format, ...) {
    va_list varArgs;
    char* newFormat;
    const char* prepend = "\n\n\t[WARNING][EPF2Bson] : ";

    newFormat = calloc((strlen(format) + strlen(prepend) + 3), (sizeof(char)));
    newFormat = strncpy(newFormat, prepend, strlen(prepend));
    newFormat = strncat(newFormat, format, strlen(format));
    newFormat = strncat(newFormat, "\n\n", 2);
    va_start(varArgs, format);
    vfprintf(stderr, newFormat, varArgs);
    va_end(varArgs);
    free(newFormat);
}

/**
 * Shows a message.
 *
 * \param message Message (printf format).
 * \param ... printf() like variable.
 */
void message(const char* format, ...) {
    va_list varArgs;
    char* newFormat;
    const char* prepend = "[EPF2Bson] : ";

    newFormat = calloc((strlen(format) + strlen(prepend) + 2), (sizeof(char)));
    newFormat = strncpy(newFormat, prepend, strlen(prepend));
    newFormat = strncat(newFormat, format, strlen(format));
    newFormat = strncat(newFormat, "\n", 1);
    va_start(varArgs, format);
    vfprintf(stdout, newFormat, varArgs);
    va_end(varArgs);
    free(newFormat);
}


