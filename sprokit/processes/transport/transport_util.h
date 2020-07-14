/*ckwg +29
 * Copyright 2020 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KWIVER_TRANSPORT_UTIL_H
#define KWIVER_TRANSPORT_UTIL_H

#include "kwiver_processes_transport_export.h"

#include <sprokit/pipeline/types.h>
#include <sprokit/pipeline/datum.h>

namespace kwiver {

/**
 * \brief Transport related support methods.
 *
 * This class contains utility methods used by the various transports.
 */
class KWIVER_PROCESSES_TRANSPORT_EXPORT transport_util
{
private:
// Define the in-band (string) representation for the datum types.
static std::string const datum_type_data;
static std::string const datum_type_empty;
static std::string const datum_type_error;
static std::string const datum_type_invalid;
static std::string const datum_type_flush;
static std::string const datum_type_complete;

public:
/**
 * \brief Convert datum type to zmq encoding.
 *
 * This method converts the datum type code into a string that can be
 * passed.
 *
 * \param dat Datum type
 *
 * \returns Datum type is encoded as a string.
 */
static std::string encode_datum_type( sprokit::datum::type_t dat );

/**
 * \brief Convert datum type string to code number.
 *
 * This method converts the string representation of the datum type
 * code back to the native code.
 *
 * \param dat Message that has encoded datum type.
 *
 * \returns Datum type enumeration.
 */
static sprokit::datum::type_t decode_datum_type( std::string const& dat );


/**
 * \brief Remove datum type encoding from string.
 *
 * This method removes the encoded datum type from the message.
 *
 * \param msg Message with encoded datum type
 *
 * \returns Message string with the encoded datum type removed.
 */
static std::string strip_datum_type( std::string const& msg );

};

}

#endif // KWIVER_TRANSPORT_UTIL_H
