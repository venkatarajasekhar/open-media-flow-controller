#  Makefile
#
#

# This file is included by common.mk , so do not include anything here.

# This file is used for all of our "normal" builds that use the build
# system's tools.  It is used for both host and target builds.
#
# Note: if you make changes here, you should keep Makefile.host.inc in
# sync.

PROD_TARGET_OS=${PROD_HOST_OS}
PROD_TARGET_PLATFORM=${PROD_HOST_PLATFORM}
PROD_TARGET_PLATFORM_VERSION=${PROD_HOST_PLATFORM_VERSION}
PROD_TARGET_PLATFORM_FULL=${PROD_TARGET_PLATFORM}_${PROD_TARGET_PLATFORM_VERSION}

PROD_TARGET_CPU=${PROD_HOST_CPU}
PROD_TARGET_CPU_SUPPORTED_WORDSIZES=${PROD_HOST_CPU_SUPPORTED_WORDSIZES}
PROD_TARGET_CPU_DEFAULT_WORDSIZE=${PROD_HOST_CPU_DEFAULT_WORDSIZE}

ifneq ($(strip ${PROD_TARGET_ARCH}),)
      # XXX This is a hack to allow 32 bit builds on x86_64
      ifeq (${PROD_HOST_ARCH}-${PROD_TARGET_ARCH},X86_64-I386)
            PROD_TARGET_ARCH=I386
            PROD_TARGET_CPU=I686
            PROD_TARGET_CPU_SUPPORTED_WORDSIZES=32
            PROD_TARGET_CPU_DEFAULT_WORDSIZE=32
      else ifneq ($(strip ${PROD_TARGET_ARCH}),$(strip ${PROD_HOST_ARCH}))
            $(error Invalid target arch ${PROD_TARGET_ARCH} for host ${PROD_HOST_ARCH} .  May be missing PROD_TARGET_ID)
      endif
else

# This is the common case

	PROD_TARGET_ARCH=${PROD_HOST_ARCH}

endif

# Lookup the endian-ness and alignment settings for the target cpu
ifeq (${PROD_TARGET_ARCH},I386)
	PROD_TARGET_CPU_ENDIAN=LITTLE
	PROD_TARGET_CPU_ALIGNMENT=8
endif
ifeq (${PROD_TARGET_ARCH},X86_64)
	PROD_TARGET_CPU_ENDIAN=LITTLE
	PROD_TARGET_CPU_ALIGNMENT=8
endif
ifeq (${PROD_TARGET_ARCH},SPARC)
	PROD_TARGET_CPU_ENDIAN=BIG
	PROD_TARGET_CPU_ALIGNMENT=32
endif
ifeq (${PROD_TARGET_ARCH},SPARC64)
	PROD_TARGET_CPU_ENDIAN=BIG
	PROD_TARGET_CPU_ALIGNMENT=32
endif
ifeq (${PROD_TARGET_ARCH},PPC)
	PROD_TARGET_CPU_ENDIAN=BIG
	PROD_TARGET_CPU_ALIGNMENT=8
endif
ifeq (${PROD_TARGET_ARCH},PPC64)
	PROD_TARGET_CPU_ENDIAN=BIG
	PROD_TARGET_CPU_ALIGNMENT=8
endif


# Default compiler and make requirements
PROD_TARGET_CC_VERSION=3.4.6
PROD_HOST_MAKE_VERSION=3.80

ifeq (${PROD_TARGET_PLATFORM_FULL},LINUX_EL_EL5)
	PROD_TARGET_CC_VERSION=4.1.2
	PROD_TARGET_JAVAC_VERSION=1.6.0_24
	PROD_HOST_MAKE_VERSION=3.81
	PROD_TARGET_PLATFORM_PREFIX=el5
endif

# Note EL6 is not currently supported, this is just for testing
ifeq (${PROD_TARGET_PLATFORM_FULL},LINUX_EL_EL6)
	PROD_TARGET_CC_VERSION=4.4.7
	PROD_TARGET_JAVAC_VERSION=1.6.0_24
	PROD_HOST_MAKE_VERSION=3.81
	PROD_TARGET_PLATFORM_PREFIX=el6
endif

ifeq (${PROD_TARGET_PLATFORM_FULL}-${PROD_HOST_ARCH}-${PROD_TARGET_ARCH},LINUX_EL_EL5-PPC-PPC)
	PROD_TARGET_CC_VERSION=4.2.2
	PROD_TARGET_JAVAC_VERSION=1.6.0_20
	PROD_HOST_MAKE_VERSION=3.81
	PROD_TARGET_PLATFORM_PREFIX=el5
endif

ifeq (${PROD_TARGET_PLATFORM_FULL},FREEBSD_FREEBSD_FBR62)
	PROD_TARGET_CC_VERSION=3.4.6
	PROD_HOST_MAKE_VERSION=3.81
	PROD_TARGET_PLATFORM_PREFIX=fbr62
endif

ifeq (${PROD_TARGET_PLATFORM_USE_PREFIX},1)
	PROD_TARGET_ID_NAME=${PROD_TARGET_PLATFORM_PREFIX}_${PROD_TARGET_ARCH_LC}${PROD_TARGET_HWNAME_FILE_DASH}
else
	PROD_TARGET_ID_NAME=${PROD_TARGET_ARCH_LC}${PROD_TARGET_HWNAME_FILE_DASH}
endif

ifndef PROD_TARGET_HOST
	PROD_TARGET_COMPILE_FOR_TEXT=
else
	PROD_TARGET_COMPILE_FOR_TEXT=for host 
endif
