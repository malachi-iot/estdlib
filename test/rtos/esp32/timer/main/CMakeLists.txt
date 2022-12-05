set(FREERTOS_TEST_DIR ../../../freertos)

idf_component_register(SRCS 
                    "main.cpp"
                    ${FREERTOS_TEST_DIR}/timer/timer.cpp
                    INCLUDE_DIRS "." ${FREERTOS_TEST_DIR})