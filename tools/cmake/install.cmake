# Guidance from [1] and [1.1]

# DEBT: Perhaps we can do this with a generator?
if(FEATURE_ESTD_INTERFACE)
    set(LIB_TYPE interface)
else()
    set(LIB_TYPE static)
endif()

message(VERBOSE "estd: LIB_TYPE=${LIB_TYPE}")

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        ${PROJECT_NAME}Version.cmake
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        #LIBRARY DESTINATION lib
        DESTINATION lib/malachi-iot/${PROJECT_NAME}-${PROJECT_VERSION}
        #DESTINATION lib
        #ARCHIVE DESTINATION lib
        #RUNTIME DESTINATION bin
        INCLUDES DESTINATION include/malachi-iot/${PROJECT_NAME}/${PROJECT_VERSION}
        )

# DEBT: Don't fully understand why we have to disable this
# for ESP-IDF to be happy as a "pure cmake component".  Smells namespace related
# https://docs.espressif.com/projects/esp-idf/en/v5.1.3/esp32/api-guides/build-system.html#writing-pure-cmake-components 
if(NOT ESP_PLATFORM)
    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}-${LIB_TYPE}-Targets.cmake
        NAMESPACE malachi-iot::
        DESTINATION lib/cmake/${PROJECT_NAME}
        )
endif()
