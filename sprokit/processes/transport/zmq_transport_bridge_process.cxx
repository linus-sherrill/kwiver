// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "zmq_transport_bridge_process.h"
#include "transport_util.h"

#include <kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>
#include <vital/util/bounded_buffer.h>

#include <zmq.hpp>
#include <memory.h>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace kwiver {

using lock =  std::unique_lock<std::mutex>;

// (config-key, value-type, default-value, description )
create_config_trait( port, int, "5550",
                     "Port number to connect/bind to.\n\n"
                     "This connection will consume four ports, "
                     "the first two are for sending "
                     "and the next two are for receiving. "
                     "Supply (port+0) to desired zmq_receive process and "
                     "supply (port+2) to desired zmq_send process."
  );

create_config_trait( connect_host, std::string, "localhost",
                     "Hostname (or IP address) to connect to and receive from.\n\n"
                     "This is the host name/address of the sender that we want "
                     "receive messages from.");

/**
 * \class zmq_transport_bridge_process
 *
 * \brief
 * \iports
 *
 * \iport{serialized_message} the incoming byte string to send.
 *
 * \oports
 *
 * \oport {serialized_message} the received byte string for output.
 * \configs
 *
 * \config{port} the port number to publish on.
 *
 */

//----------------------------------------------------------------
// Private implementation class
class zmq_transport_bridge_process::priv
{
public:
  priv() ;
  ~priv() = default;

  void connect();

  // Configuration values
  int m_port; // base port number for connecting to other pipeline

  zmq::context_t m_context;
  std::string m_connect_host;

  // Send related data areas
  zmq::socket_t m_pub_socket;
  zmq::socket_t m_pub_sync_socket;

  // Receive related data areas
  zmq::socket_t m_sub_socket;
  zmq::socket_t m_sub_sync_socket;

  vital::logger_handle_t m_logger; // for logging in priv methods

  ::kwiver::vital::bounded_buffer< std::shared_ptr< std::string > > send_buffer;
  ::kwiver::vital::bounded_buffer< std::shared_ptr< std::string > > receive_buffer;
  std::condition_variable data_ready;
  std::mutex monitor; // held if step() is working.

  // Thread control. Threads terminate when 'running' turns FALSE.
  std::unique_ptr< std::thread > send_thread;
  std::unique_ptr< std::thread > receive_thread;
  bool running{ true };
}; // end priv class

// ================================================================
zmq_transport_bridge_process
::zmq_transport_bridge_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new zmq_transport_bridge_process::priv )
{
  make_ports();
  make_config();

  d->m_logger = logger();
}


zmq_transport_bridge_process
::~zmq_transport_bridge_process()
{
  // Need to terminate threads if they are still running
  if (d->running)
  {
    LOG_DEBUG( logger(), "Destructor found threads still running. Threads terminated." );
    d->running = false;
    try
    {
      d->send_thread->detach();
      d->receive_thread->detach();
    }
    catch ( ... )
    {
      // nothing
    }
  }
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::_configure()
{
  scoped_configure_instrumentation();

  // Get process config entries
  d->m_port = config_value_using_trait( port );
  d->m_connect_host = config_value_using_trait( connect_host );

  int major, minor, patch;
  zmq_version(&major, &minor, &patch);
  LOG_DEBUG( logger(), "ZeroMQ Version: " << major << "." << minor << "." << patch );
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::_init()
{
  d->connect();

  // start threads.
  d->send_thread.reset( new std::thread( &zmq_transport_bridge_process::send_thread, this ) );
  d->receive_thread.reset( new std::thread( &zmq_transport_bridge_process::receive_thread, this ) );

  // Disable input checking by the framework
  this->set_data_checking_level( check_none );
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::_finalize()
{
  d->running = false;
  try
  {
    d->send_thread->detach();
    d->receive_thread->detach();
  }
  catch ( ... )
  {
    // nothing
  }

  d->m_context.close();
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::_step()
{
  // N.B. There is no scoped instrumentation here because of the
  // blocking behaviour, all the work done in the supporting threads,
  // and there is not much work being done here anyway.

  lock lk(d->monitor);

  if ( d->send_buffer.Empty() && d->receive_buffer.Empty() )
  {
    LOG_TRACE( logger(), "step() waiting for work");
    d->data_ready.wait(lk);
  }

  LOG_TRACE( logger(), "step() has the lock");

  // Handle traffic going to remote pipeline.

  // In this case, we always empty the input buffer since all inputs
  // have been collected by the send thread (which is running
  // asynchronously). Normally we should abide by the one input and
  // one output per step().
  while ( ! d->send_buffer.Empty() )
  {
    auto mess = d->send_buffer.Receive();
    // We know that the message is a pointer to a std::string.
    // Send mess to the transport. Also, the datum type has already
    // been encoded in the message.
    LOG_TRACE( logger(), "Sending datagram of size " << mess->size()
               << "  Type: " << mess->substr(0,4) );

    zmq::message_t datagram(mess->size());
    memcpy((void *) datagram.data(), mess->c_str(), mess->size());
    d->m_pub_socket.send(datagram);
  } // end while

  // Handle traffic from the remote pipeline. The datum encoding is
  // still in the message.
  while ( ! d->receive_buffer.Empty() )
  {
    auto msg = d->receive_buffer.Receive();

    LOG_TRACE( logger(), "Received datagram of size " << msg->size()
               << "  Type: " << msg->substr(0,4) );

    // Retrieve the datum type form the message
    auto const type { transport_util::decode_datum_type( *msg ) };

    // Complete datum does not carry any data.
    if ( type == sprokit::datum::complete )
    {
      LOG_TRACE( logger(), "Received complete datum" );

      // The remote pipeline has terminated.
      mark_process_as_complete();
      d->running = false; // stop threads;

      const sprokit::datum_t dat { sprokit::datum::complete_datum() };
      push_datum_to_port_using_trait( serialized_message, dat );

      this->_finalize(); //+ test
    }
    else
    {
      *msg = transport_util::strip_datum_type( *msg );
      auto out_datum = transport_util::new_datum_from_type( type, msg );
      push_datum_to_port_using_trait( serialized_message, out_datum );
    }
  } // end while
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( serialized_message, required );
  declare_output_port_using_trait( serialized_message, required );
}

// ----------------------------------------------------------------
void zmq_transport_bridge_process
::make_config()
{
  declare_config_using_trait( port );
  declare_config_using_trait( connect_host );
}

// ================================================================
zmq_transport_bridge_process::priv
::priv()
  : m_context( 1 )
  , m_pub_socket( m_context, ZMQ_PUB )
  , m_pub_sync_socket( m_context, ZMQ_REP )
  , m_sub_socket( m_context, ZMQ_SUB )
  , m_sub_sync_socket( m_context, ZMQ_REQ )
  , send_buffer(5)
  , receive_buffer(5)
{
}

// ----------------------------------------------------------------------------
void
zmq_transport_bridge_process::priv
::connect()
{
  { // -- publish - output connections --
    // Bind to the publisher socket
    std::ostringstream pub_connect_string;
    pub_connect_string << "tcp://*:" << m_port;
    m_pub_socket.setsockopt( ZMQ_LINGER, 5000 ); // wait 5 seconds
    LOG_TRACE( m_logger, "PUB Connect for " << pub_connect_string.str() );
    m_pub_socket.bind( pub_connect_string.str() );

    // Wait for replies from subscriber before sending anything
    std::ostringstream sync_connect_string;
    sync_connect_string << "tcp://*:" << ( m_port + 1 );
    LOG_TRACE( m_logger, "SYNC Connect for " << sync_connect_string.str() );
    m_pub_sync_socket.bind( sync_connect_string.str() );

    // Receive empty message
    zmq::message_t datagram;
    m_pub_sync_socket.recv( &datagram );
    LOG_TRACE( m_logger, "SYNC received reply from subscriber.");

    // Send message as ack
    zmq::message_t datagram_o { "ACK", 3 };
    m_pub_sync_socket.send( datagram_o );
  }

  // -- subscribe - input connections --
  {
    m_sub_socket.setsockopt(ZMQ_SUBSCRIBE,"",0);

    std::ostringstream sub_connect_string;
    sub_connect_string << "tcp://" << m_connect_host << ":" << ( m_port + 2 );
    LOG_TRACE( m_logger, "SUB Connect for " << sub_connect_string.str() );
    m_sub_socket.connect( sub_connect_string.str() );

    std::ostringstream sync_connect_string;
    sync_connect_string << "tcp://" << m_connect_host << ":" << ( m_port + 3 );
    LOG_TRACE( m_logger, "SYNC Connect for " << sync_connect_string.str() );
    m_sub_sync_socket.connect( sync_connect_string.str() );

    // Send REQ to process
    zmq::message_t datagram { "REQ", 3 };
    m_sub_sync_socket.send(datagram);

    zmq::message_t datagram_i;
    LOG_TRACE( m_logger, "Waiting for SYNC reply, pub." );
    m_sub_sync_socket.recv(&datagram_i);
    LOG_TRACE( m_logger, "SYNC reply received, pub." );
  }
}

// ============================================================================
// support threads

// This thread monitors data coming from the input port and queues it
// to the step() methods.
void zmq_transport_bridge_process
::send_thread()
{
  LOG_DEBUG( logger(), "Starting send thread");
  while ( d->running )
  {
    LOG_TRACE(logger(), "send_thread waiting for data");
    auto const mess_dat = grab_datum_from_port_using_trait( serialized_message );
    auto const dat_type = mess_dat->type();

    serialized_message_type_trait::type mess;

    // Need to prepend the datum type to the message.
    auto const out_type = transport_util::encode_datum_type( dat_type );

    // There is only data with this type of packet.
    if ( dat_type == sprokit::datum::data )
    {
      mess = mess_dat->get_datum< serialized_message_type_trait::type >();
    }
    else
    {
      mess = std::make_shared< std::string >();
    }

    LOG_TRACE( logger(), "Received message from port. Size: " << mess->size()
               << " Type " << out_type );

    // Prepend datum type
    *mess = out_type + *mess;

    { // coordinate with step()
      lock lk(d->monitor);
      LOG_TRACE(logger(), "send_thread has the lock");

      d->send_buffer.Send( mess );
      d->data_ready.notify_one();
    }

    if ( dat_type == sprokit::datum::complete )
    {
      LOG_TRACE( logger(), "Sent complete datum" );
      break;
    }
  } // end while

  LOG_DEBUG( logger(), "Ending send thread");
}

// ----------------------------------------------------------------------------
// This thread monitors data coming from the inbound ZMQ socket and
// queues it to the step() method.
void zmq_transport_bridge_process
::receive_thread()
{
  LOG_DEBUG( logger(), "Starting receive thread");
  while ( d->running )
  {
    zmq::message_t datagram;
    LOG_TRACE(logger(), "receive_thread waiting for datagram");

    try {
      d->m_sub_socket.recv( &datagram );
    } catch (...)
    {
      //+ it is not clear that more should be done here
      continue;
    }

    auto msg = std::make_shared< std::string >( static_cast<char *>(datagram.data()), datagram.size() );
    LOG_TRACE( logger(), "Received datagram of size " << msg->size()
               << "  Type: " << msg->substr(0,4) );


    { // coordinate with step()
      lock lk(d->monitor);
      LOG_TRACE(logger(), "receive_thread has the lock");
      d->receive_buffer.Send( msg );
      d->data_ready.notify_one();
    }

    // Complete datum does not carry any data.
    auto const dat_type { transport_util::decode_datum_type( *msg ) };
    if ( dat_type == sprokit::datum::complete )
    {
      LOG_TRACE( logger(), "Received complete datum" );

      // The remote pipeline has terminated.
      break;
    }
}

  LOG_DEBUG( logger(), "Ending receive thread");
}

} // end namespace
