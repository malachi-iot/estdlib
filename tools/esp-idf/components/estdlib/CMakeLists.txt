if(NOT DEFINED CMAKE_BUILD_EARLY_EXPANSION)
    # 25SEP24 MB DEBT
    # Due to my ancient test project structure, it's hard to tell if this
    # is working quite right.  Seems to fire too often.
    message(WARNING "estdlib target is deprecated.  Use estd instead")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../component.cmake NO_POLICY_SCOPE)
