##################################################################################################################################
# CMakeLists.txt    Profil de compilation du projet Watchdog
# Projet Abls-Habitat version 4.7      Gestion d'habitat
# Auteur: LEFEVRE Sebastien
##################################################################################################################################

cmake_minimum_required(VERSION 3.16)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE WTD_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND git branch --show-current
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE WTD_BRANCHE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT WTD_VERSION)
  set(WTD_VERSION "4.x-unknown")
endif()
if(NOT WTD_BRANCHE)
  set(WTD_BRANCHE "unknown")
endif()

project(Watchdog
  DESCRIPTION "Abls Habitat Watchdog daemon"
  HOMEPAGE_URL "https://github.com/sebaru"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-result -Wno-comment -Wno-pointer-sign -Wno-discarded-qualifiers -pipe -g -ggdb -fdiagnostics-color")

add_compile_definitions(
  WTD_VERSION="${WTD_VERSION}"
  WTD_BRANCHE="${WTD_BRANCHE}"
  WTD_PKGDATADIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB      REQUIRED glib-2.0)
pkg_check_modules(GTHREAD   REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(OPENSSL   REQUIRED openssl)
pkg_check_modules(UUID      REQUIRED uuid)
pkg_check_modules(LIBSYSTEMD REQUIRED libsystemd)
pkg_check_modules(LIBJWT    REQUIRED libjwt)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

include(GNUInstallDirs)

# Déclarer le répertoire de bibliothèques à ldconfig
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/watchdogd.conf" "${CMAKE_INSTALL_FULL_LIBDIR}\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/watchdogd.conf"
        DESTINATION /etc/ld.so.conf.d
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

add_subdirectory(Watchdogd)
