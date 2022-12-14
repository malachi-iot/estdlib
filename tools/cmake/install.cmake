# Guidance from [1] and [1.1]

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

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}-${LIB_TYPE}-Targets.cmake
        NAMESPACE malachi-iot::
        DESTINATION lib/cmake/${PROJECT_NAME}
        )