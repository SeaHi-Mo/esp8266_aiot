
set(include_dirs LinkSDK/core 
    LinkSDK/core/sysdep 
    LinkSDK/core/utils
    LinkSDK/components/data-model
    LinkSDK/components/dynreg 
    LinkSDK/components/dynreg-mqtt 
    LinkSDK/components/ntp 
    LinkSDK/components/ota 

)

file(GLOB_RECURSE srcs 
    LinkSDK/core/*.c 
    LinkSDK/core/utils/*.c
    LinkSDK/core/sysdep/*.c
    LinkSDK/portfiles/aiot_port/*.c
    LinkSDK/external/*.c
    LinkSDK/components/data-model/*.c
    LinkSDK/components/dynreg/*.c 
    LinkSDK/components/dynreg-mqtt/*.c
    LinkSDK/components/ntp/*.c  
    LinkSDK/components/ota/*.c

)

set(COMPONENT_SRCS ${srcs})
set(COMPONENT_ADD_INCLUDEDIRS ${include_dirs})
register_component()