// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "transport_util.h"
#include "zmq_transport_send_process.h"

#include <kwiver_type_traits.h>
#include <zmq.hpp>

#include <memory.h>

namespace kwiver {

// (config-key, value-type, default-value, description )
create_config_trait( port, int, "5550",
                     "Port number to connect/bind to.\n\n"
                     "This will consume two ports. "
                     "The port specified and the next one." );

create_config_trait( expected_subscribers, int, "1",
                     "Number of subscribers to wait for before starting to publish.\n\n"
                     "If zero is entered, then this process will not wait for any subscribers." );

/**
 * \class zmq_transport_send_process
 *
 * \brief End cap that publishes incoming data on a ZeroMQ PUB Socket
 *
 * \process This process can be used to connect separate Sprokit pipelines
 * to one another in a publishi/subscribe framework.  This process
 * accepts serialized byte strings from serializer_process and transmits
 * them on a ZeroMQ PUB socket at the specified port.
 *
 * The process can be optionally configured to wait for an expected
 * number of subscribers before it begins publishing or it can begin
 * publishing immediately.  Use these options to control whether
 * subscribers see a consistent amount of data.  Waiting to publish
 * might be used when a data source is a directory of images, while
 * publishing immediately might be used when the data source is a
 * live video feed.
 *
 * The PUB/SUB mechanism makes use of two ports.  The actual
 * PUB/SUB socket pair is created on the port specified in
 * the configuration. One above that port is used to handle
 * subscription handshaking.
 *
 * \iports
 *
 * \iport{serialized_message} the incoming byte string to publish.
 *
 * \configs
 *
 * \config{port} the port number to publish on.
 *
 * \config{expected_subscribers} the number of subscribers that must
 * connect before publication commences.
 */

// ----------------------------------------------------------------
// Private implementation class
class zmq_transport_send_process::priv
{
public:
  priv();
  ~priv();

  void connect();

  // Configuration values
  int m_port;
  int m_expected_subscribers;

  // any other connection related data goes here
  zmq::context_t m_context;
  zmq::socket_t m_pub_socket;
  zmq::socket_t m_sync_socket;

  vital::logger_handle_t m_logger; // for logging in priv methods
}; // end priv class

// ================================================================

zmq_transport_send_process
::zmq_transport_send_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new zmq_transport_send_process::priv )
{
  make_ports();
  make_config();

  d->m_logger = logger();
}

zmq_transport_send_process
::~zmq_transport_send_process()
{
}

// ----------------------------------------------------------------
void
zmq_transport_send_process
::_configure()
{
  scoped_configure_instrumentation();

  // Get process config entries
  d->m_port = config_value_using_trait( port );
  d->m_expected_subscribers = config_value_using_trait( expected_subscribers );

  int major, minor, patch;
  zmq_version( &major, &minor, &patch );
  LOG_DEBUG(
    logger(), "ZeroMQ Version: " << major << "." << minor << "." << patch );
}

// ----------------------------------------------------------------
void
zmq_transport_send_process
::_init()
{
  d->connect();

  // Disable input checking by the framework
  this->set_data_checking_level( check_none );
}

// ----------------------------------------------------------------
void
zmq_transport_send_process
::_step()
{
  auto const mess_dat = grab_datum_from_port_using_trait( serialized_message );

  scoped_step_instrumentation();

  auto dat_type = mess_dat->type();

  // Encode the datum type as the first character of the message. The
  // datum type needs to be forwarded so that flow termination can be
  // signaled.
  // We know that the message is a pointer to a std::string
  // send mess to the transport.
  //
  // Need to prepend the datum type to the message.
  auto out_mess = transport_util::encode_datum_type( dat_type );

  // There is only data with this type of packet.
  if( dat_type == sprokit::datum::data )
  {
    auto const mess =
      mess_dat->get_datum< serialized_message_type_trait::type >();
    out_mess += *mess;
  }

  LOG_TRACE( logger(),
             "Sending datagram of size: "       << out_mess.size()
                                                << "  Type: " << out_mess.substr(
                 0,
                 4 ) );

  zmq::message_t datagram( out_mess.size() );
  memcpy( (void*) datagram.data(), ( out_mess.c_str() ), out_mess.size() );
  d->m_pub_socket.send( datagram );

  // We need to handle the "complete" datum locally because we have
  // selected "check_none" as checking_level.
  if( dat_type == sprokit::datum::complete )
  {
    mark_process_as_complete();
  }
}

// ----------------------------------------------------------------
void
zmq_transport_send_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( serialized_message, required );
}

// ----------------------------------------------------------------
void
zmq_transport_send_process
::make_config()
{
  declare_config_using_trait( port );
  declare_config_using_trait( expected_subscribers );
}

// ================================================================
zmq_transport_send_process::priv
::priv()
  : m_context( 1 ),
    m_pub_socket( m_context, ZMQ_PUB ),
    m_sync_socket( m_context, ZMQ_REP )
{
}

zmq_transport_send_process::priv
::~priv()
{
}

// ----------------------------------------------------------------------------
void
zmq_transport_send_process::priv
::connect()
{
  // Bind to the publisher socket
  std::ostringstream pub_connect_string;
  pub_connect_string << "tcp://*:" << m_port;
  m_pub_socket.setsockopt( ZMQ_LINGER, 5000 ); // wait 5 seconds
  LOG_TRACE( m_logger, "PUB Connect for " << pub_connect_string.str() );
  m_pub_socket.bind( pub_connect_string.str() );

  // Wait for replies from expected number of subscribers before sending
  // anything
  std::ostringstream sync_connect_string;
  sync_connect_string << "tcp://*:" << ( m_port + 1 );
  LOG_TRACE( m_logger, "SYNC Connect for " << sync_connect_string.str() );
  m_sync_socket.bind( sync_connect_string.str() );

  int subscribers = 0;
  LOG_TRACE( m_logger, "Entering Sync Loop, waiting for "
             << m_expected_subscribers << " subscribers" );
  while( subscribers < m_expected_subscribers )
  {
    // Receive empty message
    zmq::message_t datagram;
    m_sync_socket.recv( &datagram );
    LOG_TRACE( m_logger, "SYNC Loop, received reply from subscriber "
               << subscribers << " of " << m_expected_subscribers );

    // Send message as ack
    zmq::message_t datagram_o( "ACK", 3 );
    m_sync_socket.send( datagram_o );
    ++subscribers;
  }

  LOG_TRACE( m_logger, "SYNC Loop done, all " << subscribers << " received" );
}

} // end namespace
