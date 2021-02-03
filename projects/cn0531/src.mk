#See No-OS/tool/scripts/src_model.mk for variable description

SRC_DIRS += $(PROJECT)/src
SRC_DIRS += $(NO-OS)/iio/iio_app

# Add to SRCS source files to be build in the project
SRCS += $(NO-OS)/drivers/dac/ad5791/ad5791.c \
	$(NO-OS)/drivers/dac/ad5791/iio_ad5791.c

# Add to INCS inlcude files to be build in the porject
INCS += $(NO-OS)/drivers/dac/ad5791/ad5791.h \
	$(NO-OS)/drivers/dac/ad5791/iio_ad5791.h

SRC_DIRS += $(PLATFORM_DRIVERS)
SRC_DIRS += $(NO-OS)/util
SRC_DIRS += $(INCLUDE)

TINYIIOD=y

