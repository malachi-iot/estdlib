CPMAddPackage("gh:vector-of-bool/cmrc#2.0.1")

cmrc_add_resource_library(TESTRC resources/hello.txt ALIAS testrc)

list(TRANSFORM RESOURCES PREPEND resources/)

cmrc_add_resources(TESTRC ${RESOURCES})
