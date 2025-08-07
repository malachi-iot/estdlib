CPMAddPackage("gh:vector-of-bool/cmrc#952ffddba731fc110bd50409e8d2b8a06abbd237")

cmrc_add_resource_library(TESTRC resources/hello.txt ALIAS testrc)

include(resources/resources.cmake)

list(TRANSFORM RESOURCES PREPEND resources/)

cmrc_add_resources(TESTRC ${RESOURCES})
