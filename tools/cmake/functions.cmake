function(estd_add_project_semver)
    set(prerelease "${ARGV0}")

    string(TOUPPER ${PROJECT_NAME} upper)

    set(semver ${PROJECT_VERSION})

    if(prerelease)
        string(APPEND semver "-${prerelease}")
        # Fancy full C-line to avoid embedding this line every time.  This way
        # interested parties can do an #ifdef on XYZ_PRERELEASE
        string(CONCAT prerelease_c
            "#define ${upper}_VER_PRERELEASE "
            "\"${prerelease}\"")
        set(PROJECT_VERSION_PRERELEASE_C ${prerelease_c} PARENT_SCOPE)
        set(${PROJECT_NAME}_PRERELEASE 1 PARENT_SCOPE)
    endif()

    set(PROJECT_NAME_UPPER ${upper} PARENT_SCOPE)
    set(PROJECT_SEMVER "${semver}" PARENT_SCOPE)
    set(${PROJECT_NAME}_SEMVER "${semver}" PARENT_SCOPE)
endfunction()
