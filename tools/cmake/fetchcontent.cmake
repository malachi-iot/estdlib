include(FetchContent)

# NOTE: Cache dir not a standard FetchContent mechanism
# DEBT: Not super Windows friendly, but won't cause any failures
set(FETCHCONTENT_CACHE_DIR $ENV{HOME}/.local/cache/cmake/proj)
if(EXISTS ${FETCHCONTENT_CACHE_DIR})
    set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_CACHE_DIR}/${PROJECT_NAME}-${PROJECT_VERSION})
endif()
