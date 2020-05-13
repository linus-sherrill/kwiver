// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 *
 * \brief interface to the serializer process.
 */

#ifndef SPROKIT_PROCESS_FLOW_SERIALIZER_PROCESS_H
#define SPROKIT_PROCESS_FLOW_SERIALIZER_PROCESS_H

#include <sprokit/pipeline/process.h>
#include "kwiver_processes_export.h"

#include "serializer_base.h"

namespace kwiver {

class KWIVER_PROCESSES_NO_EXPORT serializer_process
  : public sprokit::process
  , public serializer_base
{
public:
  PLUGIN_INFO( "serializer",
               "Serializes data types to byte streams.\n\n"
               "Input and output ports are dynamically created based on connection. "
               "Input data elements are martialed using a specified encoding format and "
               "grouped as a message. This message is usually sent to a transport process."
    )

  serializer_process( kwiver::vital::config_block_sptr const& config );
  virtual ~serializer_process();

protected:
  virtual void _configure();
  virtual void _init();
  virtual void _step();

  void input_port_undefined( port_t const& port ) override;
  void output_port_undefined( port_t const& port ) override;

  virtual bool _set_input_port_type( port_t const&      port_name,
                                     port_type_t const& port_type ) override;

private:
  void make_config();

  class priv;
  const std::unique_ptr< priv > d;

}; // end class serializer_process

}  // end namespace

#endif /* SPROKIT_PROCESS_FLOW_SERIALIZER_PROCESS_H */
