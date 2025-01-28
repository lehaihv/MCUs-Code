idf_component_register(SRCS "bmp280_i2c.c" "bmp280_i2c_hal.c"
                    REQUIRES driver
                    INCLUDE_DIRS "include")
