# wired for ESP8266 but very likely can and should be used for ESP32 also

# IDF_VER is not available from main project.mk
# either here or in Makefile, so we have to dup code to make it
# furthermore, the IDF_VER is stock-populated
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

# decompose actual primary v2.0.0 (or whatever v#.#.# we are presented with)
# this *excludes* the dashed suffix for the tag
IDF_VER_MAIN := $(subst ., ,$(word 1, $(IDF_VER)))

$(info ************  VERSION: $(IDF_VER) / $(IDF_VER_MAIN) ************)

# useful for auto-stripping the v off the front
#$(info -- $(IDF_VER_MAIN:v%=%) --)

# I would decompose v2.0.0 more, but I don't trust espressif will necessarily
# do something like a v2.1.0.  If they do, then I'll write the decomposer
ifeq ($(word 1,$(IDF_VER_MAIN)),v2)
	IDF_VER_MAJOR=2
	IDF_VER_MINOR=$(word 2,$(IDF_VER_MAIN))
	IDF_VER_PATCH=0
	IDF_VER_SUFFIX=$(word 2,$(IDF_VER))
	IDF_VER_GIT=$(word 3,$(IDF_VER))
else ifeq ($(word 1,$(IDF_VER_MAIN)),v3)
	IDF_VER_MAJOR=3
	IDF_VER_MINOR=$(word 2,$(IDF_VER_MAIN))
	ifeq ($(words $(IDF_VER_MAIN)),3)
		IDF_VER_PATCH=$(word 3,$(IDF_VER_MAIN))
	else
		IDF_VER_PATCH=0
	endif
	# has 'dev' or similar in the middle, complicating matters
	IDF_VER_TAG=$(word 2,$(IDF_VER))
	IDF_VER_SUFFIX=$(word 3,$(IDF_VER))
	ifeq ($(IDF_VER_SUFFIX),)
		IDF_VER_SUFFIX = 0
	endif
	IDF_VER_GIT=$(word 4,$(IDF_VER))
else

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

endif

# for v2.0.0 (ESP8266)
# notable IDF_VER_SUFFIX:
#   444 = fairly stable version, lightly buggy
#   644 = more esp32 esp-idf like, noticable API breaking changes from 444

$(info ** v$(IDF_VER_MAJOR).$(IDF_VER_MINOR).$(IDF_VER_PATCH) ($(IDF_VER_TAG)) $(IDF_VER_SUFFIX) **)

# somehow IDF_VER_SUFFIX gets blasted to blank during CPPFLAGS.  
# Not sure how, but have to shim this in
IDF_VER_SUFFIX2 := $(IDF_VER_SUFFIX)

# because C doesn't do well comparing against strings during preprocessing
ifeq ($(IDF_VER_TAG),)
	IDF_VER_TAG_NUMERIC = 0
else ifeq ($(IDF_VER_TAG), dev)
	IDF_VER_TAG_NUMERIC = 1
else ifeq ($(IDF_VER_TAG), rc1)
	IDF_VER_TAG_NUMERIC = 2
else ifeq ($(IDF_VER_TAG), rc2)
	IDF_VER_TAG_NUMERIC = 3
else ifeq ($(IDF_VER_TAG), rc)
	IDF_VER_TAG_NUMERIC = 4
else ifeq ($(IDF_VER_TAG), beta1)
	IDF_VER_TAG_NUMERIC = 5
else
	IDF_VER_TAG_NUMERIC = -1
endif

CPPFLAGS += -D ESTD_IDF_VER_MAJOR=$(IDF_VER_MAJOR) \
	-D ESTD_IDF_VER_MINOR=$(IDF_VER_MINOR) \
	-D ESTD_IDF_VER_PATCH=$(IDF_VER_PATCH) \
	-D ESTD_IDF_VER_SUFFIX=$(IDF_VER_SUFFIX2) \
	-D ESTD_IDF_VER_TAG=$(IDF_VER_TAG_NUMERIC) \
	-D ESTD_IDF_VER_TAG_STR=${IDF_VER_TAG}
