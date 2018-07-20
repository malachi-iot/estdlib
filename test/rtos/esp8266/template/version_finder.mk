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
# works, just not ready yet
#IDF_VER := $(subst -, ,$(IDF_VER))

$(info ************  TEST VERSION: $(IDF_VER) ************)

# simply awful.  but beats having no idea whatsoever of esp-idf version to target
ifeq ($(IDF_VER),v2.0.0-444-g0d718b2)
	IDF_VER_MAJOR=2
	IDF_VER_MINOR=0
	IDF_VER_SUFFIX=444
else ifeq ($(IDF_VER),v2.0.0-644-g765754f)
	IDF_VER_MAJOR=2
	IDF_VER_MINOR=0
	IDF_VER_SUFFIX=644
else
	IDF_VER_MAJOR=2
	IDF_VER_MINOR=0
	IDF_VER_SUFFIX=0
endif

CPPFLAGS += -D ESTD_IDF_VER_MAJOR=$(IDF_VER_MAJOR) \
	-D ESTD_IDF_VER_MINOR=$(IDF_VER_MINOR) \
	-D ESTD_IDF_VER_SUFFIX=$(IDF_VER_SUFFIX)
