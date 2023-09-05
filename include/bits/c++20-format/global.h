// Copyright (C) embedded ocean GmbH, Munich, Germany.
//
// This file is part of the Charconv Float Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef CXX20_FORMAT_GLOBAL_H
#define	CXX20_FORMAT_GLOBAL_H

#include <bits/c++20-format/config.h>

#define CXX20_FORMAT_VISIBILITY_ATTRIBUTE_WRAPPER(visibility) _GLIBCXX_VISIBILITY(visibility)
#define CXX20_FORMAT_VISIBILITY_ATTRIBUTE CXX20_FORMAT_VISIBILITY_ATTRIBUTE_WRAPPER(CXX20_FORMAT_VISIBILITY)

#define CXX20_FORMAT_PASTE_TOKENS_WRAPPER(token1, token2) token1##token2
#define CXX20_FORMAT_PASTE_TOKENS(token1, token2) CXX20_FORMAT_PASTE_TOKENS_WRAPPER(token1, token2)
#define CXX20_FORMAT_DECORATE_NAME(base_namespace) CXX20_FORMAT_PASTE_TOKENS(CXX20_FORMAT_NAMESPACE, base_namespace)

#endif // CXX20_FORMAT_GLOBAL_H
