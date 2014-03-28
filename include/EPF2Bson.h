/**
 * Program includes.
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

#ifndef _EPF2BSON_H_INCLUDED_
#define _EPF2BSON_H_INCLUDED_

#define __USE_C99_MATH
#define _GNU_SOURCE

#include <stdbool.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <libgen.h>
#include <glob.h>

#include <sys/types.h>
#include <sys/stat.h>


#include <sys/time.h>
#include <time.h>

/**
 * Program options.
 */
typedef struct programOptions {
    /**
     * Program should run verbosely.
     */
    bool verbose;

    /**
     * EPF files directory.
     */
    char* epfDir;

    /**
     * EPF collections to export.
     */
    char** epfList;

    /**
     * MongoDB database name.
     */
    char* dbName;

    /**
     * Dump directory.
     */
    char* dumpDir;
} programOptions;




extern programOptions*  epf2bsonOptions;
extern bool             epfRecoverableReadEmpty;
extern int              errno;


#endif /* _EPF2BSON_H_INCLUDED_ */