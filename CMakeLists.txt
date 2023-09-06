# Copyright (C) embedded ocean GmbH, Munich, Germany.
#
# This file is part of the C++20 Format Library.  This library is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the GCC Runtime Library Exception, version
# 3.1, as published by the Free Software Foundation.
#
# You should have received a copy of the GNU General Public License and
# a copy of the GCC Runtime Library Exception along with this program;
# see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
# <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.19)

project(c++20-format 
	VERSION 1.0.0
	DESCRIPTION "C++20 std::format for libstdc++ 12"
	HOMEPAGE_URL "https://gitlab.com/embedded-ocean/oss/c++20-format"
	LANGUAGES CXX)

###########################################################################
# Options and settings

set(CXX20_FORMAT_TARGET_TYPE SHARED CACHE STRING "the cmake target type for the library to build")
set(CXX20_FORMAT_NAMESPACE "__cxx20_format" CACHE STRING "the inline namespace used for format and related classes")
set(CXX20_FORMAT_VISIBILITY "default" CACHE STRING "the ELF visibility used for format and related classes")

set_property(CACHE CXX20_FORMAT_TARGET_TYPE PROPERTY STRINGS SHARED STATIC OBJECT)

###########################################################################
# Check system features

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

try_compile(HAVE_COMPATIBLE_GLIBCXX
	${CMAKE_CURRENT_BINARY_DIR}/check_libstdcxx_version
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_libstdcxx_version.cc
)

try_compile(HAVE_INCLUDE_NEXT
	${CMAKE_CURRENT_BINARY_DIR}/check_include_next
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_include_next.cc
)

if(HAVE_INCLUDE_NEXT AND HAVE_COMPATIBLE_GLIBCXX)
	set(PATCH_STD_HEADERS TRUE)
endif()

###########################################################################
# generate the config include

set(CONFIG_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/include/bits/c++20-format/config.h")
configure_file(config/config.h.in "${CONFIG_HEADER_FILE}" @ONLY)

###########################################################################
# define the library

add_library(
	${PROJECT_NAME} ${CXX20_FORMAT_TARGET_TYPE}

	src/c++17/floating_to_chars.cc
)

target_include_directories(
	${PROJECT_NAME}
	
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
		"$<INSTALL_INTERFACE:include/c++20-format>"
)

target_compile_options(
	${PROJECT_NAME}

	PRIVATE
		$<$<CXX_COMPILER_ID:GNU,Clang>:-fsized-deallocation>
		$<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-old-style-cast>
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(PATCH_STD_HEADERS)

target_include_directories(
	${PROJECT_NAME}
	
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/std>"
)

endif(PATCH_STD_HEADERS)

set_target_properties(
	${PROJECT_NAME}
	
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS ON

		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
)

###########################################################################
# generate test application
#
# This application tests the interface only. We rely on the original libstdc++ test cases
# for functional tests

add_executable(
	${PROJECT_NAME}-test
	
	"test/format-test.cc"
)

target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME}-test APPEND PROPERTY BUILD_RPATH "$ORIGIN")

###########################################################################
# define the headers and directories

set(HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/c++20-format.h"
)

set(PRIVATE_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/bits/c++20-format/charconv-bits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/bits/c++20-format/charconv.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/bits/c++20-format/global.h"
	"${CONFIG_HEADER_FILE}"
)

set(PRIVATE_HEADER_SUBDIR bits/c++20-format)

set(PATCH_STD_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/std/format"
)

###########################################################################
# export and install the project

# Skip this entire section if no installation is desired
if(NOT CMAKE_SKIP_INSTALL_RULES)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/c++20-format")
set(INSTALL_MODULEDIR "${CMAKE_INSTALL_DATADIR}/cmake/Cxx20Format")

set(MODULE_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/module/Cxx20FormatConfig.cmake")
set(MODULE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/module/Cxx20FormatVersion.cmake")

configure_package_config_file(
	config/Cxx20FormatConfig.cmake.in
	${MODULE_CONFIG_FILE}

	INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/Cxx20Format"
)

write_basic_package_version_file(
	${MODULE_VERSION_FILE}

	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets RUNTIME ARCHIVE)
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE Cxx20Format:: FILE Cxx20FormatTargets.cmake DESTINATION "${INSTALL_MODULEDIR}")

install(
	FILES ${HEADER_FILES}
	DESTINATION "${INSTALL_INCLUDEDIR}"
)

install(
	FILES ${PRIVATE_HEADER_FILES}
	DESTINATION "${INSTALL_INCLUDEDIR}/${PRIVATE_HEADER_SUBDIR}"
)

install(
	FILES ${PATCH_STD_HEADER_FILES}
	DESTINATION "${INSTALL_INCLUDEDIR}/std"
)

install(
	FILES
		"${MODULE_CONFIG_FILE}"
		"${MODULE_VERSION_FILE}"
		config/PatchLibstdcxx.cmake
		config/check_include_next.cc
		config/check_libstdcxx_version.cc
	
	DESTINATION "${INSTALL_MODULEDIR}"
)

endif(NOT CMAKE_SKIP_INSTALL_RULES)
