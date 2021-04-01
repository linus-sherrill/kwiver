// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "transport_util.h"

#include <vital/exceptions/serialize.h>

#include <sstream>
#include <string>

namespace kwiver {

// These all have 4 letter encoding so that they can easily be
// extracted from the message.  I know this does not read well, but it
// is a parsing shortcut.
std::string const transport_util::datum_type_data { "data" };
std::string const transport_util::datum_type_empty { "empt" };
std::string const transport_util::datum_type_error { "erro" };
std::string const transport_util::datum_type_invalid { "inva" };
std::string const transport_util::datum_type_flush { " flus" };
std::string const transport_util::datum_type_complete { "comp" };

// ----------------------------------------------------------------------------
std::string
transport_util
::encode_datum_type( sprokit::datum::type_t type )
{
  switch( type )
  {
    case sprokit::datum::data:     return datum_type_data;
    case sprokit::datum::empty:    return datum_type_empty;
    case sprokit::datum::error:    return datum_type_error;
    case sprokit::datum::invalid:  return datum_type_invalid;
    case sprokit::datum::flush:    return datum_type_flush;
    case sprokit::datum::complete: return datum_type_complete;

    default:
      // This really unexpected. There is no recovery.
      std::stringstream msg;
      msg       << "Unexpected datum type. Received \""
                << type << "\"";
      VITAL_THROW( vital::serialization_exception, msg.str() );
  } // end switch
}

// ----------------------------------------------------------------------------
sprokit::datum::type_t
transport_util
::decode_datum_type( std::string const& dat )
{
  // Extract the datum type from the start of the string
  auto const datum_string { dat.substr( 0, 4 ) };

  // Decode the type string
  if( datum_string == datum_type_data ) { return sprokit::datum::data; }
  if( datum_string == datum_type_empty ) { return sprokit::datum::empty; }
  if( datum_string == datum_type_invalid ) { return sprokit::datum::invalid; }
  if( datum_string == datum_type_flush ) { return sprokit::datum::flush; }
  if( datum_string == datum_type_complete ) { return sprokit::datum::complete; }
  if( datum_string == datum_type_error ) { return sprokit::datum::error; }

  // This really unexpected. There is no recovery.
  std::stringstream msg;
  msg   << "Unexpected datum type encoding. Received \""
        << datum_string << "\"";
  VITAL_THROW( vital::serialization_exception, msg.str() );
}

// ----------------------------------------------------------------------------
std::string
transport_util
::strip_datum_type( std::string const& msg )
{
  if( msg.size() > 4 )
  {
    return msg.substr( 4 );
  }

  return std::string();
}

// ----------------------------------------------------------------------------
template < typename T >
sprokit::datum_t
transport_util
::new_datum_from_type( sprokit::datum::type_t type, T const& data )
{
  switch( type )
  {
    case sprokit::datum::data:     return sprokit::datum::new_datum< T >( data );
    case sprokit::datum::empty:    return sprokit::datum::empty_datum();
    case sprokit::datum::error:    return sprokit::datum::error_datum(
        "Error lost in translation" );
    case sprokit::datum::flush:    return sprokit::datum::flush_datum();
    case sprokit::datum::complete: return sprokit::datum::complete_datum();

    default:
      // This really unexpected. There is no recovery.
      std::stringstream msg;
      msg       << "Unexpected datum type. Received \""
                << type << "\"";
      VITAL_THROW( vital::serialization_exception, msg.str() );
  } // end switch
}

// Explicit instantiation for serialized messages
template sprokit::datum_t transport_util
::new_datum_from_type< std::shared_ptr< std::string > >(
  sprokit::datum::type_t type,
  std::shared_ptr< std::string > const& data );

} // end namespace kwiver
