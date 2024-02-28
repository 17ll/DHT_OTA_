PROJECT_NAME := curl_http_mp3 
#include $(ADF_PATH)/project.mk
include $(IDF_PATH)/make/project.mk
# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'make flash'.
SPIFFS_IMAGE_FLASH_IN_PROJECT := 1
$(eval $(call spiffs_create_partition_image,spiffs,spiffs_image))

