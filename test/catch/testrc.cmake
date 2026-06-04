CPMAddPackage("gh:vector-of-bool/cmrc#952ffddba731fc110bd50409e8d2b8a06abbd237")

# 04JUN26 MB DEBT: TESTRC is global across CMakeLists so change this to be
# something like ESTD_TESTRC
cmrc_add_resource_library(TESTRC resources/hello.txt ALIAS testrc)

include(resources/resources.cmake)

list(TRANSFORM RESOURCES PREPEND resources/)

cmrc_add_resources(TESTRC ${RESOURCES})
