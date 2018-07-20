# wired for ESP8266 but very likely can and should be used for ESP32 also

# this is completely lame.  IDF_VER is not available from main project.mk
# either here or in Makefile, so we have to dup code to make it
# furthermore, the lameness continues in that IDF_VER is stock-populated
# in such a way you can't react to its contents with an #ifdef

# If we have `version.txt` then prefer that for extracting IDF version
ifeq ("$(wildcard ${IDF_PATH}/version.txt)","")
IDF_VER := $(shell cd ${IDF_PATH} && git describe --always --tags --dirty)
else
IDF_VER := `cat ${IDF_PATH}/version.txt`
endif

# kinda tricky, turning dashes into whitespace so that makefiles text operations work
# more smoothly
IDF_VER := $(subst -, ,$(IDF_VER))

#$(info ************  VERSION: $(IDF_VER) ************)

# I would decompose v2.0.0 more, but I don't trust espressif will necessarily
# do something like a v2.1.0.  If they do, then I'll write the decomposer
ifeq ($(word 1,$(IDF_VER)),v2.0.0)
	#$(info got here)
	IDF_VER_MAJOR=2
	IDF_VER_MINOR=0
	IDF_VER_PATCH=0
	IDF_VER_SUFFIX=$(word 2,$(IDF_VER))
	IDF_VER_GIT=$(word 3,$(IDF_VER))
else
	IDF_VER_MAJOR=0
	IDF_VER_MINOR=0
	IDF_VER_PATCH=0
	IDF_VER_SUFFIX=0
endif

# for v2.0.0
# notable IDF_VER_SUFFIX:
#   444 = fairly stable version, lightly buggy
#   644 = more esp32 esp-idf like, noticable API breaking changes from 444

CPPFLAGS += -D ESTD_IDF_VER_MAJOR=$(IDF_VER_MAJOR) \
	-D ESTD_IDF_VER_MINOR=$(IDF_VER_MINOR) \
	-D ESTD_IDF_VER_PATCH=$(IDF_VER_PATCH) \
	-D ESTD_IDF_VER_SUFFIX=$(IDF_VER_SUFFIX) \
	-D ESP8266
