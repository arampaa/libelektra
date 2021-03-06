include (LibAddMacros)

file (GLOB HDR_FILES
	   *.h)
file (GLOB SRC_FILES
	   *.c)

add_headers (ELEKTRA_HEADERS)
set (SOURCES ${SRC_FILES} ${HDR_FILES} ${ELEKTRA_HEADERS})

if (BUILD_FULL OR BUILD_STATIC)
	list (APPEND SOURCES
		     $<TARGET_OBJECTS:elektra-io-uv-objects>) # add sources for elektra-io-uv for static and full builds
endif ()

# Build test
set (EXAMPLE exampleio_uv)

add_executable (${EXAMPLE} ${SOURCES})
add_dependencies (${EXAMPLE} kdberrors_generated elektra_error_codes_generated)

target_include_directories (${EXAMPLE} SYSTEM PUBLIC ${libuv_INCLUDE_DIRS})

target_link_elektra (${EXAMPLE} elektra-kdb elektra-io elektra-io-uv)
if (BUILD_FULL OR BUILD_STATIC)
	target_link_libraries (${EXAMPLE} ${PC_libuv_LDFLAGS})
endif ()

if (libuv_VERSION VERSION_LESS "1.0")
	target_compile_definitions (${EXAMPLE} PRIVATE "HAVE_LIBUV0")
else ()
	target_compile_definitions (${EXAMPLE} PRIVATE "HAVE_LIBUV1")
endif ()
