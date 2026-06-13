if(EXISTS "${APPLICATION_SOURCE_DIR}/include/drivers/input_processor.h")
  zephyr_library()

  set(_zmk_app_include_candidates
    ${ZEPHYR_BASE}/../app/include
    ${ZEPHYR_BASE}/../zmk/app/include
  )
  foreach(_inc ${_zmk_app_include_candidates})
    if(EXISTS ${_inc})
      zephyr_library_include_directories(${_inc})
      break()
    endif()
  endforeach()

  zephyr_library_sources(input_processors/input_processor_pinch_zoom_keys.c)
endif()
