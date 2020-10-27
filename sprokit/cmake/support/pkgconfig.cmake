function (sprokit_configure_pkgconfig module)
  if (UNIX)
    set(pkgconfig_file "${sprokit_binary_dir}/lib/pkgconfig/${module}.pc")

    kwiver_configure_file(sprokit-${module}.pc
      "${CMAKE_CURRENT_SOURCE_DIR}/${module}.pc.in"
      "${pkgconfig_file}"
      KWIVER_VERSION
      CMAKE_INSTALL_PREFIX
      LIB_SUFFIX
      ${ARGN})

    install(
      FILES       "${pkgconfig_file}"
      DESTINATION "lib${LIB_SUFFIX}/pkgconfig"
      COMPONENT   development)
  endif ()
endfunction ()
