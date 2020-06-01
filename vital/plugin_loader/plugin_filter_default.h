/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

#ifndef KWIVER_FITAL_PLUGIN_FILTER_DEFAULT_H
#define KWIVER_FITAL_PLUGIN_FILTER_DEFAULT_H

#include <vital/plugin_loader/vital_vpm_export.h>

#include <vital/plugin_loader/plugin_loader_filter.h>

#include <map>

namespace kwiver {
namespace vital {

class plugin_manager;

// -----------------------------------------------------------------
/** Default plugin loader filter.
 *
 * This filter excludes duplicate plugins. An exception is thrown if a
 * duplicate is found.
 */
class VITAL_VPM_EXPORT plugin_filter_default
  : public plugin_loader_filter
{
public:
  // -- CONSTRUCTORS --
  plugin_filter_default( plugin_manager* vpm )
    : m_vpm{ vpm }
  {}

  virtual ~plugin_filter_default() = default;

  bool add_factory( plugin_factory_handle_t fact ) const override;
  bool load_plugin( path_t const& path,
                    DL::LibraryHandle lib_handle ) const override;

  void add_fixup( std::string const& sym, void* ptr ); //+ ???

private:
  plugin_manager* m_vpm;

  // Could be a list of symbols to fixup
  std::map< std::string, void*> m_fixups;
}; // end class plugin_filter_default

} } // end namespace

#endif // KWIVER_FITAL_PLUGIN_FILTER_DEFAULT_H
