if(NOT DEFINED CMAKE_BUILD_EARLY_EXPANSION)
    message(WARNING "estdlib target is deprecated.  Use estd instead")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../component.cmake NO_POLICY_SCOPE)
