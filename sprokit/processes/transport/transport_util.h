// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_TRANSPORT_UTIL_H
#define KWIVER_TRANSPORT_UTIL_H

#include "kwiver_processes_transport_export.h"

#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/types.h>

namespace kwiver {

/**
 * \brief Transport related support methods.
 *
 * This class contains utility methods used by the various transports.
 */
class KWIVER_PROCESSES_TRANSPORT_EXPORT transport_util
{
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

/**
 * \brief Create datum of correct type.
 *
 * This method creates a new sprokit::datum based on the type code
 * supplied. If the type is 'data', then the supplied data element is
 * stored in the datum. If the datum is a control type, then no data
 * is associated with the datum.
 *
 * \param typ Datum type code
 * \param dat Data for datum
 *
 * \return sprokit::datum of the specified type.
 */
  template < typename T >
  static sprokit::datum_t new_datum_from_type( sprokit::datum::type_t type,
                                               T const& dat );

private:
// Define the in-band (string) representation for the datum types.
  static std::string const datum_type_data;
  static std::string const datum_type_empty;
  static std::string const datum_type_error;
  static std::string const datum_type_invalid;
  static std::string const datum_type_flush;
  static std::string const datum_type_complete;
};

} // namespace kwiver

#endif // KWIVER_TRANSPORT_UTIL_H
