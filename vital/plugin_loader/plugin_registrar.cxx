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

#include "plugin_registrar.h"

#include <vital/plugin_loader/plugin_loader.h>

namespace kwiver {
namespace vital {
class plugin_manager;
extern ::kwiver::vital::plugin_manager* kwiver_vital_plugin_manager_S_instance;

// ----------------------------------------------------------------------------
plugin_registrar
::plugin_registrar( vital::plugin_loader& vpl,
                    const std::string& name )
  : mod_name( name )
  , mod_organization( KWIVER_DEFAULT_PLUGIN_ORGANIZATION )
  , m_plugin_loader( vpl )
{
  update_vpm( vpl );
}

// ----------------------------------------------------------------------------
bool
plugin_registrar
::is_module_loaded()
{
  return m_plugin_loader.is_module_loaded( mod_name );
}

// ----------------------------------------------------------------------------
/// Mark module as loaded.
void
plugin_registrar::
mark_module_as_loaded()
{
  m_plugin_loader.mark_module_as_loaded( mod_name );
}

// ----------------------------------------------------------------------------
/// Return module name.
const std::string&
plugin_registrar
::module_name() const
{
  return this->mod_name;
}

// ----------------------------------------------------------------------------
/// Return module owning organization.
const std::string&
plugin_registrar
::organization() const
{
  return this->mod_organization;
}

// ----------------------------------------------------------------------------
/// Return reference to the plugin loader.
kwiver::vital::plugin_loader&
plugin_registrar
::plugin_loader()
{
  return this->m_plugin_loader;
}

// ----------------------------------------------------------------------------
void plugin_registrar
::update_vpm( vital::plugin_loader& vpl )
{
  // If we are being loaded and there is no active VPM, update
  // the instance pointer to be the one that is loading us.
  if ( nullptr == kwiver_vital_plugin_manager_S_instance )
  {
    // Downcast to get the expected context object.
    auto * ctxt = dynamic_cast< ::kwiver::vital::vpm_context* >( vpl.get_context() );
    kwiver_vital_plugin_manager_S_instance = ctxt->get_current_vpm();
  }


} }
