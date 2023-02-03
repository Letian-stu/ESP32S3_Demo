idf_component_register(
SRCS 
                "sta_ap.c"
                "web_file/web_config.c"

INCLUDE_DIRS    "."
                "web_file/include"
                
EMBED_FILES "../spiffs/config.html"                
)

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

