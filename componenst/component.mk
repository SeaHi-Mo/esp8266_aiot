#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
#COMPONENT_ADD_INCLUDEDIRS := LinkSDK/core \
LinkSDK/core/sysdep \
LinkSDK/core/utils \
LinkSDK/components/ota \
LinkSDK/components/ntp \
LinkSDK/components/data-model \
LinkSDK/components/dynreg \
LinkSDK/components/dynreg-mqtt \

#COMPONENT_SRCDIRS := LinkSDK/core \
LinkSDK/core/utils \
LinkSDK/core/sysdep \
LinkSDK/components/ota \
LinkSDK/components/ntp \
LinkSDK/components/data-model \
LinkSDK/components/dynreg \
LinkSDK/components/dynreg-mqtt \
LinkSDK/portfiles/aiot_port \
LinkSDK/external
