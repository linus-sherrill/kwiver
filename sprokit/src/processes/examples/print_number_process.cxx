// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "print_number_process.h"

#include <vital/config/config_block.h>
#include <sprokit/pipeline/process_exception.h>

#include <fstream>
#include <string>

/**
 * \file print_number_process.cxx
 *
 * \brief Implementation of the number printer process.
 */

namespace sprokit {

class print_number_process::priv
{
public:
  typedef int32_t number_t;

  priv(std::string const& output_path);
  ~priv() = default;

  std::string const path;
  std::ofstream fout;

  static kwiver::vital::config_block_key_t const config_path;
  static port_t const port_input;
};

kwiver::vital::config_block_key_t const print_number_process::priv::config_path = kwiver::vital::config_block_key_t("output");
  process::port_t const print_number_process::priv::port_input = port_t("number"); // port name

// ----------------------------------------------------------------------------
print_number_process
::print_number_process(kwiver::vital::config_block_sptr const& config)
  : process(config)
  , d()
{
  declare_configuration_key(
    priv::config_path,
    kwiver::vital::config_block_value_t(),
    kwiver::vital::config_block_description_t("The path of the file to output to."));

  port_flags_t required;

  required.insert(flag_required);

  declare_input_port(
    priv::port_input,
    "integer", // port type
    required,
    port_description_t("Where numbers are read from."));
}

print_number_process
::~print_number_process()
{
}

// ----------------------------------------------------------------------------
void
print_number_process
::_configure()
{
  // Configure the process.
  {
    std::string const path = config_value< std::string >(priv::config_path);

    d.reset(new priv(path));
  }

  if (d->path.empty())
  {
    static std::string const reason = "The path given was empty";
    kwiver::vital::config_block_value_t const value;

    VITAL_THROW( invalid_configuration_value_exception,
                 name(), priv::config_path, value, reason);
  }

  d->fout.open(d->path);

  if (!d->fout.good())
  {
    std::string const reason = "Failed to open the path: " + d->path;

    VITAL_THROW( invalid_configuration_exception,
                 name(), reason);
  }

  process::_configure();
}

// ----------------------------------------------------------------------------
void
print_number_process
::_reset()
{
  d->fout.close();

  process::_reset();
}

// ----------------------------------------------------------------------------
void
print_number_process
::_step()
{
  priv::number_t const input = grab_from_port_as<priv::number_t>(priv::port_input);

  d->fout << input << std::endl;
}

// ----------------------------------------------------------------------------
print_number_process::priv
::priv(std::string const& output_path)
  : path(output_path)
  , fout()
{
}

}
