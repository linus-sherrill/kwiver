// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_TRANSPORT_ZMQ_TRANSPORT_BRIDGE_PROCESS_H
#define KWIVER_TRANSPORT_ZMQ_TRANSPORT_BRIDGE_PROCESS_H

#include <sprokit/pipeline/process.h>

#include "kwiver_processes_transport_export.h"

namespace kwiver {

// ----------------------------------------------------------------

/*
 * zmq_transport_send_process
 *
 *  Writes serialized data to a file.
 */
class KWIVER_PROCESSES_TRANSPORT_NO_EXPORT zmq_transport_bridge_process
  : public sprokit::process
{
public:
  PLUGIN_INFO( "zmq_transport_bridge",
               "Send and receive serialized buffers to/from ZMQ transport.\n\n"
               "This process acts as a bridge to another pipeline. "
               "The original application is to bridge to a pipeline "
               "running in a docker container." )

  zmq_transport_bridge_process( kwiver::vital::config_block_sptr const& config );
  virtual ~zmq_transport_bridge_process();

protected:
  void _configure() override;
  void _init() override;
  void _finalize() override;
  void _step() override;

private:
  void make_ports();
  void make_config();

  void send_thread();
  void receive_thread();

  class priv;

  const std::unique_ptr< priv > d;
}; // end class zmq_transport_bridge_process

}  // end namespace

#endif
