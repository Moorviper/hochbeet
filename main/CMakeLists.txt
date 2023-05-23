idf_component_register(SRCS "main.c" "mqtt.c" "relais.c" "sensorOneshot.c"  "wifi.c" "calibrateADC.c" "messureADC.c" 
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
