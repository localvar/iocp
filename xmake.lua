target("iocp")
    set_kind("binary")
    set_languages("c11", "cxx14")
    add_files("src/*.c", "src/*.cpp")
    add_links("pthread")
    if is_arch("x86_64") and is_os("linux") then
        add_cxflags("-mcx16")
    end


-- CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
-- PROJECT(iocp)

-- SET(CMAKE_C_STANDARD 11)
-- SET(CMAKE_CXX_STANDARD 14)

-- IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    -- SET(CMAKE_GENERATOR "NMake Makefiles" CACHE INTERNAL "" FORCE)
    -- SET(COMMON_FLAGS "/WX- /Oi /Oy- /Gm- /EHsc /MT /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Gd /errorReport:prompt /analyze-")
    -- SET(DEBUG_FLAGS "/Zi /W3 /GL")
    -- SET(RELEASE_FLAGS "/W0 /GL")	
-- ELSEIF (NOT (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    -- MESSAGE(FATAL_ERROR "The current platform is not Linux or Windows, stop compile")
    -- EXIT ()
-- ELSE ()
-- #  IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")			
    -- SET(COMMON_FLAGS "-std=c11 -Wall -msse4.2 -mcx16")
    -- SET(DEBUG_FLAGS "-O0 -DDEBUG")
    -- SET(RELEASE_FLAGS "-O3")
-- ENDIF ()

-- #  IF (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
-- #    MESSAGE(STATUS "The current platform is Linux 64-bit")
-- #    ADD_DEFINITIONS(-D_M_X64)
-- #  ELSE ()
-- #    MESSAGE(FATAL_ERROR "The current platform is Linux 32-bit, not supported yet")
-- #    EXIT ()
-- #  ENDIF ()

-- #SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMMON_FLAGS} ${DEBUG_FLAGS}")
-- #SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${COMMON_FLAGS} ${RELEASE_FLAGS}")

-- #IF (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    -- #MESSAGE(STATUS "Build Debug Version")
-- #ELSEIF (${CMAKE_BUILD_TYPE} MATCHES "Release")
    -- #MESSAGE(STATUS "Build Release Version")
-- #ELSE ()
    -- #SET(CMAKE_BUILD_TYPE "Debug")
    -- #MESSAGE(STATUS "Build Debug Version")
-- #ENDIF ()

-- SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
-- ADD_SUBDIRECTORY(src)
