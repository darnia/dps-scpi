cmake_minimum_required(VERSION 3.0.2)
project( dps-scpid )

option(WITH_SYSTEMD "Set to ON to create unit files and enable systemd startup"  OFF)

if (WITH_SYSTEMD)
	add_definitions( -DWITH_SYSTEMD )
endif (WITH_SYSTEMD)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SYSTEMD libsystemd)
include_directories(${SYSTEMD_INCLUDE_DIRS})
if (SYSTEMD_FOUND AND "${SYSTEMD_SERVICES_INSTALL_DIR}" STREQUAL "")
	execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE}
		--variable=systemdsystemunitdir systemd
		OUTPUT_VARIABLE SYSTEMD_SERVICES_INSTALL_DIR)
	string(REGEX REPLACE "[ \t\n]+" "" SYSTEMD_SERVICES_INSTALL_DIR
		"${SYSTEMD_SERVICES_INSTALL_DIR}")
endif ()

pkg_check_modules(SCPI REQUIRED scpi)
include_directories(${SCPI_INCLUDE_DIRS})

pkg_check_modules(OPENDPS REQUIRED opendps)
include_directories(${OPENDPS_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DPS_SCPID_SRCS
  src/scpi-def.c
  src/main.c
)

# if systemd append sd-helper
if (WITH_SYSTEMD)
	set(DPS_SCPID_SRCS
		src/sd-helper.c
		${DPS_SCPID_SRCS}
	)
endif (WITH_SYSTEMD)

add_executable( dps-scpid ${DPS_SCPID_SRCS} )
set_target_properties(dps-scpid PROPERTIES COMPILE_FLAGS "-Wall -Wformat-nonliteral")

target_link_libraries( dps-scpid LINK_PUBLIC ${SCPI_LIBRARIES} ${OPENDPS_LIBRARIES} ${SYSTEMD_LIBRARIES})
install (TARGETS dps-scpid DESTINATION /usr/bin/)

if (WITH_SYSTEMD)
	install (FILES ${CMAKE_SOURCE_DIR}/scripts/dps-scpid.service
	DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR}/)
endif (WITH_SYSTEMD)
