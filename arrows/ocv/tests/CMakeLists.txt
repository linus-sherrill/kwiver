project(arrows_test_ocv)

set(CMAKE_FOLDER "Arrows/OCV/Tests")

include(kwiver-test-setup)

set(test_libraries      vital vital_vpm kwiver_algo_ocv)

##############################
# Algorithms OpenCV tests
##############################
kwiver_discover_gtests(ocv algo_config                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv bounding_box                LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv descriptor_set              LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv distortion                  LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv feature_set                 LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv image                       LIBRARIES ${test_libraries})
kwiver_discover_gtests(ocv match_set                   LIBRARIES ${test_libraries})


# Additional tests that depend on the MVG arrow
if (KWIVER_ENABLE_MVG)
  list(APPEND test_libraries kwiver_algo_mvg)

  kwiver_discover_gtests(ocv estimate_fundamental_matrix LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv estimate_homography         LIBRARIES ${test_libraries})
  kwiver_discover_gtests(ocv estimate_pnp                LIBRARIES ${test_libraries})
endif()
