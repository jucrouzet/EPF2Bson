/**
 * Errors handling.
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

#ifndef _ERROR_H_INCLUDED_
#define _ERROR_H_INCLUDED_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/**
 * Show usage.
 */
void usage();

/**
 * Shows an error.
 *
 * \param message Message (printf format).
 * \param ... printf() like variable.
 */
void error(const char* format, ...);

/**
 * Shows an warning.
 *
 * \param message Message to warning (printf format).
 * \param ... printf() like variable.
 */
void warning(const char* format, ...);


/**
 * Shows a message.
 *
 * \param message Message (printf format).
 * \param ... printf() like variable.
 */
void message(const char* format, ...);


#endif /* _ERROR_H_INCLUDED_ */