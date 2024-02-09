# 3rd party things

# https://github.com/malachi-iot/estdlib/issues/23 
# Mentioned also in https://github.com/malachi-iot/estdlib/issues/22
# DEBT: Not conforming to FEATURE_ESTD_xxx convention.  Experimenting
# with reducing that formality

option(ESTD_DRAGONBOX
    "3rd party float->int lib used for float->string conversions"
    false)

if(ESTD_DRAGONBOX)
    FetchContent_Declare(
        dragonbox
        GIT_REPOSITORY  https://github.com/jk-jeon/dragonbox
        GIT_TAG         1.1.3
        GIT_PROGRESS    TRUE
        GIT_SHALLOW     TRUE
    )

    FetchContent_MakeAvailable(dragonbox)
endif()

