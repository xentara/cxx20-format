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
# <http://www.gnu.org/licenses/>.POSIX

if(NOT TARGET Cxx20Format::libstdcxx-patch)

try_compile(
	_CXX20_FORMAT_PATCH_LIBSTDCXX

	"${CMAKE_CURRENT_BINARY_DIR}/Cxx20Format"
	"${CMAKE_CURRENT_LIST_DIR}/check_libstdcxx_version.cc"
)

if(_CXX20_FORMAT_PATCH_LIBSTDCXX)

	try_compile(
		_CXX20_FORMAT_PATCH_STD_HEADERS

		"${CMAKE_CURRENT_BINARY_DIR}/Cxx20Format"
		"${CMAKE_CURRENT_LIST_DIR}/check_include_next.cc")

	if(_CXX20_FORMAT_PATCH_STD_HEADERS)
		target_include_directories(Cxx20Format::c++20-format INTERFACE "${_IMPORT_PREFIX}/include/c++20-format/std>")
	endif(_CXX20_FORMAT_PATCH_STD_HEADERS)

	unset(_CXX20_FORMAT_PATCH_STD_HEADERS)

	add_library(Cxx20Format::libstdcxx-patch ALIAS Cxx20Format::c++20-format)

else(_CXX20_FORMAT_PATCH_LIBSTDCXX)

	add_library(Cxx20Format::libstdcxx-patch INTERFACE IMPORTED)

endif(_CXX20_FORMAT_PATCH_LIBSTDCXX)

unset(_CXX20_FORMAT_PATCH_LIBSTDCXX)

endif(NOT TARGET Cxx20Format::libstdcxx-patch)
