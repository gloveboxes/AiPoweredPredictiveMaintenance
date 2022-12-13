#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

###################################################################################################################

# Select your developer board by removing the # tag from the beginning of the line

# The default board selected is the AVNET Azure Sphere Starter Kit Revision 1.

# If you are NOT using the AVNET Revision 1 board be sure to comment out the AVNET board

set(AVNET TRUE "AVNET Azure Sphere Starter Kit Revision 1 ")    
# set(AVNET_REV_2 TRUE "AVNET Azure Sphere Starter Kit Revision 2 ")

# set(SCD4x TRUE "MikroE HVAC Click with SCD4x sensor")
set(SCD30 TRUE "Seeed Studio Grove CO2 sensor with SCD30 sensor")          
###################################################################################################################

cmake_minimum_required (VERSION 3.10)

project (classify_solution C)

# For information on setting tools revision and target api set see 
# https://docs.microsoft.com/en-us/azure-sphere/app-development/using-cmake-functions

# This project is evergreen and will build to the SDK installed
include(tools/cmake/azsphere_config.cmake)
auto_generate_azsphere_config()



add_subdirectory("AzureSphereDevX" out)
add_subdirectory("Drivers/AVNET/HL" drivers)

if (SCD4x)
    set(scd4x
        "Drivers/embedded-i2c-scd4x/scd4x_i2c.c"
        "Drivers/embedded-i2c-scd4x/sensirion_common.c"
        "Drivers/embedded-i2c-scd4x/sensirion_i2c.c"
        "Drivers/embedded-i2c-scd4x/sensirion_i2c_hal.c"
    )
    source_group("scd4x" FILES ${scd4x})
endif(SCD4x)

# Create executable
add_executable (${PROJECT_NAME} main.c co2_sensor.c ${scd4x})
target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c azure_sphere_devx)
target_include_directories(${PROJECT_NAME} PUBLIC AzureSphereDevX/include )
target_include_directories(${PROJECT_NAME} PUBLIC ../IntercoreContract)

target_link_libraries (${PROJECT_NAME} avnet_drivers)
target_include_directories(${PROJECT_NAME} PUBLIC Drivers/AVNET/HL)

set_source_files_properties("Drivers/embedded-i2c-scd4x/sensirion_common.c" PROPERTIES COMPILE_FLAGS -Wno-conversion)
set_source_files_properties("Drivers/embedded-i2c-scd4x/sensirion_i2c.c" PROPERTIES COMPILE_FLAGS -Wno-conversion)

if(SCD30)
    add_definitions( -DSCD30=TRUE )
    target_link_libraries (${PROJECT_NAME} scd30_lib)
    add_subdirectory("Drivers/EmbeddedScd30" scd30)
endif(SCD30)

if (SCD4x)
    add_definitions( -DSCD4x=TRUE )
    target_include_directories(${PROJECT_NAME} PUBLIC Drivers/embedded-i2c-scd4x scd4x)
endif(SCD4x)


if(AVNET)
    add_definitions( -DOEM_AVNET=TRUE )
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../HardwareDefinitions/avnet_mt3620_sk" TARGET_DEFINITION "predictive_maintenance.json")
endif(AVNET)

if(AVNET_REV_2)
    add_definitions( -DOEM_AVNET=TRUE )
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../HardwareDefinitions/avnet_mt3620_sk_rev2" TARGET_DEFINITION "predictive_maintenance.json")
endif(AVNET_REV_2)


azsphere_target_add_image_package(${PROJECT_NAME})