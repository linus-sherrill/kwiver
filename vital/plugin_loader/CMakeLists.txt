###
# plugin loader

###
# vital plugin manager
set(vital_vpm_headers
  plugin_loader.h
  plugin_factory.h
  plugin_manager.h
  plugin_info.h
  plugin_registrar.h
  )

set(vital_vpm_sources
  plugin_loader.cxx
  plugin_factory.cxx
  plugin_manager.cxx
  plugin_filter_default.cxx
  plugin_filter_category.cxx
  plugin_registrar.cxx
  )

set(vital_vpm_headers_private
  plugin_loader_filter.h
  plugin_filter_default.h
  plugin_filter_category.h
  plugin_manager_internal.h
  ${CMAKE_CURRENT_BINARY_DIR}/vital_vpm_export.h
  )

kwiver_add_library(vital_vpm STATIC
  ${vital_vpm_headers}
  ${vital_vpm_headers_private}
  ${vital_vpm_sources}
  )

kwiver_install_headers(
  ${vital_vpm_headers}
  SUBDIR      vital/plugin_loader
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_vpm_export.h
  SUBDIR      vital/plugin_loader
  NOPATH
  )

target_link_libraries(vital_vpm
  PUBLIC              vital_util
                      kwiversys
  PRIVATE             vital_logger
                      vital_exceptions
                      ${VITAL_BOOST_REGEX}
  )

###
# Add tests if enbled
if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()

### TESTING
kwiver_add_library( get_VPM STATIC
  get_VPM.cxx )

target_link_libraries( get_VPM
  PRIVATE        kwiversys
  )

###
kwiver_add_executable( test_get_vpm
  test_get_vpm.cxx
  )

target_link_libraries( test_get_vpm
  vital_vpm   get_VPM
  static_a static_b
  )

###
kwiver_add_library( static_a STATIC
  static_a_get.cxx
  )

target_link_libraries( static_a
  get_VPM vital_vpm)

###
kwiver_add_library( static_b STATIC
  static_b_get.cxx
  )

target_link_libraries( static_b
  get_VPM vital_vpm)
