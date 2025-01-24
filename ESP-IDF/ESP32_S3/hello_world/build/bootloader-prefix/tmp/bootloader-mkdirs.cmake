# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/macbookpro5530/esp/v5.1.5/esp-idf/components/bootloader/subproject"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/tmp"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/src"
  "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/macbookpro5530/Documents/GitHub/MCUs Code/ESP-IDF/ESP32_S3/hello_world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
