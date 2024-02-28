idf_component_register(SRCS "main.c"
                    "utils/dht/DHT.h"
                    INCLUDE_DIRS ".")
spiffs_create_partition_image(spiffs ../spiffs_image FLASH_IN_PROJECT)
