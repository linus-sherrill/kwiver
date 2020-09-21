// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "integer.h"

#include "convert_protobuf.h"

#include <vital/types/protobuf/integer.pb.h>

namespace kwiver {
namespace arrows {
namespace serialize {
namespace protobuf {

integer::integer()
{
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;
}

// ----------------------------------------------------------------------------
std::shared_ptr< std::string >
integer
::serialize( const vital::any& element )
{
  int32_t const data = kwiver::vital::any_cast< int32_t >( element );
  std::ostringstream msg;
  msg << "integer ";

  kwiver::protobuf::integer proto_string;
  convert_protobuf( data, proto_string );

  if ( ! proto_string.SerializeToOstream( &msg ) )
  {
    LOG_ERROR( logger(), "proto_string.SerializeToOStream failed" );
  }
  return std::make_shared< std::string > ( msg.str() );
}

// ----------------------------------------------------------------------------
vital::any
integer
::deserialize( const std::string& message )
{
  int32_t data;
  std::istringstream msg( message );
  std::string tag;

  msg >> tag;
  msg.get();

  if (tag != "integer")
  {
    LOG_ERROR( logger(), "Invalid data type tag received. Expected \"integer\", received \""
            << tag << "\". Message dropped.");
  }
  else
  {
    kwiver::protobuf::integer proto_str;
    if ( !proto_str.ParseFromIstream( &msg ) )
    {
      LOG_ERROR( logger(), "Incoming protobuf stream did not parse correctly");
    }
    convert_protobuf( proto_str, data);
  }
  return kwiver::vital::any(data);
}

} } } }
