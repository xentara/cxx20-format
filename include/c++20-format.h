// <format> Formatting -*- C++ -*-

// Copyright The GNU Toolchain Authors.
//
// This file is part of the GNU ISO C++ Library.  This library is free
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

#ifndef CXX20_FORMAT_H
#define CXX20_FORMAT_H 1

#include <bits/c++20-format/global.h>

#pragma GCC system_header

#if __cplusplus >= 202002L

#include <array>
#include <bits/c++20-format/charconv.h>
#include <concepts>
#include <limits>
#include <locale>
#include <optional>
#include <span>
#include <string_view>
#include <string>
#include <variant>	       // monostate (TODO: move to bits/utility.h?)
#include <bits/ranges_base.h>  // input_range, range_reference_t
#include <bits/ranges_algobase.h> // ranges::copy
#include <bits/stl_iterator.h> // back_insert_iterator
#include <bits/stl_pair.h>     // __is_pair
#include <bits/utility.h>      // tuple_size_v
#include <ext/numeric_traits.h> // __int_traits

#if !__has_builtin(__builtin_toupper)
# include <cctype>
#endif

namespace std CXX20_FORMAT_VISIBILITY_ATTRIBUTE
{
inline namespace CXX20_FORMAT_NAMESPACE
{

// 201907 Text Formatting, Integration of chrono, printf corner cases.
// 202106 std::format improvements.
// 202110 Fixing locale handling in chrono formatters, generator-like types.
// 202207 Encodings in localized formatting of chrono, basic-format-string.
#define __cpp_lib_format 202106L

#if __cplusplus > 202002L
// 202207 P2286R8 Formatting Ranges
// 202207 P2585R1 Improving default container formatting
// TODO: #define __cpp_lib_format_ranges 202207L
#endif

  // [format.context], class template basic_format_context
  template<typename _Out, typename _CharT> class basic_format_context;

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  // Type-erased character sink.
  template<typename _CharT> struct _Sink;
  // Output iterator that writes to a type-erase character sink.
  template<typename _CharT>
    class _Sink_iter;
} // namespace __format
/// @endcond

  using format_context
    = basic_format_context<CXX20_FORMAT_DECORATE_NAME(__format)::_Sink_iter<char>, char>;
  using wformat_context
    = basic_format_context<CXX20_FORMAT_DECORATE_NAME(__format)::_Sink_iter<wchar_t>, wchar_t>;

  // [format.args], class template basic_format_args
  template<typename _Context> class basic_format_args;
  using format_args = basic_format_args<format_context>;
  using wformat_args = basic_format_args<wformat_context>;

  // [format.arguments], arguments
  // [format.arg], class template basic_format_arg
  template<typename _Context>
    class basic_format_arg;

  // [format.fmt.string], class template basic_format_string

  /** A compile-time checked format string for the specified argument types.
   *
   * @since C++23 but available as an extension in C++20.
   */
  template<typename _CharT, typename... _Args>
    struct basic_format_string
    {
      template<typename _Tp>
	requires convertible_to<const _Tp&, basic_string_view<_CharT>>
	consteval
	basic_format_string(const _Tp& __s);

      [[__gnu__::__always_inline__]]
      constexpr basic_string_view<_CharT>
      get() const noexcept
      { return _M_str; }

    private:
      basic_string_view<_CharT> _M_str;
    };

  template<typename... _Args>
    using format_string = basic_format_string<char, type_identity_t<_Args>...>;

  template<typename... _Args>
    using wformat_string
      = basic_format_string<wchar_t, type_identity_t<_Args>...>;

  // [format.formatter], formatter

  /// The primary template of std::formatter is disabled.
  template<typename _Tp, typename _CharT = char>
    struct formatter
    {
      formatter() = delete; // No std::formatter specialization for this type.
      formatter(const formatter&) = delete;
      formatter& operator=(const formatter&) = delete;
    };

  // [format.error], class format_error
  class format_error : public runtime_error
  {
  public:
    explicit format_error(const string& __what) : runtime_error(__what) { }
    explicit format_error(const char* __what) : runtime_error(__what) { }
  };

  /// @cond undocumented
  [[noreturn]]
  inline void
  __throw_format_error(const char* __what)
  { _GLIBCXX_THROW_OR_ABORT(format_error(__what)); }

namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  // XXX use named functions for each constexpr error?

  [[noreturn]]
  inline void
  __unmatched_left_brace_in_format_string()
  { __throw_format_error("format error: unmatched '{' in format string"); }

  [[noreturn]]
  inline void
  __unmatched_right_brace_in_format_string()
  { __throw_format_error("format error: unmatched '}' in format string"); }

  [[noreturn]]
  inline void
  __conflicting_indexing_in_format_string()
  { __throw_format_error("format error: conflicting indexing style in format string"); }

  [[noreturn]]
  inline void
  __invalid_arg_id_in_format_string()
  { __throw_format_error("format error: invalid arg-id in format string"); }

  [[noreturn]]
  inline void
  __failed_to_parse_format_spec()
  { __throw_format_error("format error: failed to parse format-spec"); }
} // namespace __format
  /// @endcond

  // [format.parse.ctx], class template basic_format_parse_context
  template<typename _CharT> class basic_format_parse_context;
  using format_parse_context = basic_format_parse_context<char>;
  using wformat_parse_context = basic_format_parse_context<wchar_t>;

  template<typename _CharT>
    class basic_format_parse_context
    {
    public:
      using char_type = _CharT;
      using const_iterator = typename basic_string_view<_CharT>::const_iterator;
      using iterator = const_iterator;

      constexpr explicit
      basic_format_parse_context(basic_string_view<_CharT> __fmt,
				 size_t __num_args = 0) noexcept
      : _M_begin(__fmt.begin()), _M_end(__fmt.end()), _M_num_args(__num_args)
      { }

      basic_format_parse_context(const basic_format_parse_context&) = delete;
      void operator=(const basic_format_parse_context&) = delete;

      constexpr const_iterator begin() const noexcept { return _M_begin; }
      constexpr const_iterator end() const noexcept { return _M_end; }

      constexpr void
      advance_to(const_iterator __it) noexcept
      { _M_begin = __it; }

      constexpr size_t
      next_arg_id()
      {
	if (_M_indexing == _Manual)
	  CXX20_FORMAT_DECORATE_NAME(__format)::__conflicting_indexing_in_format_string();
	_M_indexing = _Auto;

	// _GLIBCXX_RESOLVE_LIB_DEFECTS
	// 3825. Missing compile-time argument id check in next_arg_id
	if (std::is_constant_evaluated())
	  if (_M_next_arg_id == _M_num_args)
	    CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
	return _M_next_arg_id++;
      }

      constexpr void
      check_arg_id(size_t __id)
      {
	if (_M_indexing == _Auto)
	  CXX20_FORMAT_DECORATE_NAME(__format)::__conflicting_indexing_in_format_string();
	_M_indexing = _Manual;

	if (std::is_constant_evaluated())
	  if (__id >= _M_num_args)
	    CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
      }

    private:
      iterator _M_begin;
      iterator _M_end;
      enum _Indexing { _Unknown, _Manual, _Auto };
      _Indexing _M_indexing = _Unknown;
      size_t _M_next_arg_id = 0;
      size_t _M_num_args;
    };

/// @cond undocumented
  template<typename _Tp, template<typename...> class _Class>
    static constexpr bool __is_specialization_of = false;
  template<template<typename...> class _Class, typename... _Args>
    static constexpr bool __is_specialization_of<_Class<_Args...>, _Class>
      = true;

namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  // pre: first != last
  template<typename _CharT>
    constexpr pair<unsigned short, const _CharT*>
    __parse_integer(const _CharT* __first, const _CharT* __last)
    {
      if (__first == __last)
	__builtin_unreachable();

      if constexpr (is_same_v<_CharT, char>)
	{
	  const auto __start = __first;
	  unsigned short __val = 0;
	  // N.B. std::from_chars is not constexpr in C++20.
	  if (CXX20_FORMAT_DECORATE_NAME(__detail)::__from_chars_alnum<true>(__first, __last, __val, 10)
		&& __first != __start) [[likely]]
	    return {__val, __first};
	}
      else
	{
	  unsigned short __val = 0;
	  constexpr int __n = 32;
	  char __buf[__n]{};
	  for (int __i = 0; __i < __n && (__first + __i) != __last; ++__i)
	    __buf[__i] = __first[__i];
	  auto [__v, __ptr] = CXX20_FORMAT_DECORATE_NAME(__format)::__parse_integer(__buf, __buf + __n);
	  return {__v, __first + (__ptr - __buf)};
	}
      return {0, nullptr};
    }

  template<typename _CharT>
    constexpr pair<unsigned short, const _CharT*>
    __parse_arg_id(const _CharT* __first, const _CharT* __last)
    {
      if (__first == __last)
	__builtin_unreachable();

      if (*__first == '0')
	return {0, __first + 1}; // No leading zeros allowed, so '0...' == 0

      if ('1' <= *__first && *__first <= '9')
	{
	  const unsigned short __id = *__first - '0';
	  const auto __next = __first + 1;
	  // Optimize for most likely case of single digit arg-id.
	  if (__next == __last || !('0' <= *__next && *__next <= '9'))
	    return {__id, __next};
	  else
	    return CXX20_FORMAT_DECORATE_NAME(__format)::__parse_integer(__first, __last);
	}
      return {0, nullptr};
    }

  enum _Pres_type {
    _Pres_none = 0, // Default type (not valid for integer presentation types).
    // Presentation types for integral types (including bool and charT).
    _Pres_d = 1, _Pres_b, _Pres_B, _Pres_o, _Pres_x, _Pres_X, _Pres_c,
    // Presentation types for floating-point types.
    _Pres_a = 1, _Pres_A, _Pres_e, _Pres_E, _Pres_f, _Pres_g, _Pres_G,
    _Pres_p = 0, _Pres_P,   // For pointers.
    _Pres_s = 0,            // For strings and bool.
    _Pres_esc = 0xf,        // For strings and charT.
  };

  enum _Align {
    _Align_default,
    _Align_left,
    _Align_right,
    _Align_centre,
  };

  enum _Sign {
    _Sign_default,
    _Sign_plus,
    _Sign_minus,  // XXX does this need to be distinct from _Sign_default?
    _Sign_space,
  };

  enum _WidthPrec {
    _WP_none,    // No width/prec specified.
    _WP_value,   // Fixed width/prec specified.
    _WP_from_arg // Use a formatting argument for width/prec.
  };

  template<typename _Context>
    size_t
    __int_from_arg(const basic_format_arg<_Context>& __arg);

  constexpr bool __is_digit(char __c)
  { return std::CXX20_FORMAT_DECORATE_NAME(__detail)::__from_chars_alnum_to_val(__c) < 10; }

  constexpr bool __is_xdigit(char __c)
  { return std::CXX20_FORMAT_DECORATE_NAME(__detail)::__from_chars_alnum_to_val(__c) < 16; }

  template<typename _CharT>
    struct _Spec
    {
      _Align     _M_align : 2;
      _Sign      _M_sign : 2;
      unsigned   _M_alt : 1;
      unsigned   _M_localized : 1;
      unsigned   _M_zero_fill : 1;
      _WidthPrec _M_width_kind : 2;
      _WidthPrec _M_prec_kind : 2;
      _Pres_type _M_type : 4;
      unsigned short _M_width;
      unsigned short _M_prec;
      _CharT _M_fill = ' ';

      using iterator = typename basic_string_view<_CharT>::iterator;

      static constexpr _Align
      _S_align(_CharT __c) noexcept
      {
	switch (__c)
	{
	  case '<': return _Align_left;
	  case '>': return _Align_right;
	  case '^': return _Align_centre;
	  default: return _Align_default;
	}
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_fill_and_align(iterator __first, iterator __last) noexcept
      {
	if (*__first != '{')
	  {
	    // TODO: accept any UCS scalar value as fill character.
	    // If narrow source encoding is UTF-8 then accept multibyte char.
	    if (__last - __first >= 2)
	      {
		if (_Align __align = _S_align(__first[1]))
		  {
		    _M_fill = *__first;
		    _M_align = __align;
		    return __first + 2;
		  }
	      }

	    if (_Align __align = _S_align(__first[0]))
	      {
		_M_fill = ' ';
		_M_align = __align;
		return __first + 1;
	      }
	  }
	return __first;
      }

      static constexpr _Sign
      _S_sign(_CharT __c) noexcept
      {
	switch (__c)
	{
	  case '+': return _Sign_plus;
	  case '-': return _Sign_minus;
	  case ' ': return _Sign_space;
	  default:  return _Sign_default;
	}
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_sign(iterator __first, iterator) noexcept
      {
	if (_Sign __sign = _S_sign(*__first))
	  {
	    _M_sign = __sign;
	    return __first + 1;
	  }
	return __first;
      }

      // pre: *__first is valid
      constexpr iterator
      _M_parse_alternate_form(iterator __first, iterator) noexcept
      {
	if (*__first == '#')
	  {
	    _M_alt = true;
	    ++__first;
	  }
	return __first;
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_zero_fill(iterator __first, iterator /* __last */) noexcept
      {
	if (*__first == '0')
	  {
	    _M_zero_fill = true;
	    ++__first;
	  }
	return __first;
      }

      // pre: __first != __last
      static constexpr iterator
      _S_parse_width_or_precision(iterator __first, iterator __last,
				  unsigned short& __val, bool& __arg_id,
				  basic_format_parse_context<_CharT>& __pc)
      {
	if (CXX20_FORMAT_DECORATE_NAME(__format)::__is_digit(*__first))
	  {
	    auto [__v, __ptr] = CXX20_FORMAT_DECORATE_NAME(__format)::__parse_integer(__first, __last);
	    if (!__ptr)
	      __throw_format_error("format error: invalid width or precision "
				   "in format-spec");
	    __first = __ptr;
	    __val = __v;
	  }
	else if (*__first == '{')
	  {
	    __arg_id = true;
	    ++__first;
	    if (__first == __last)
	      CXX20_FORMAT_DECORATE_NAME(__format)::__unmatched_left_brace_in_format_string();
	    if (*__first == '}')
	      __val = __pc.next_arg_id();
	    else
	      {
		auto [__v, __ptr] = CXX20_FORMAT_DECORATE_NAME(__format)::__parse_arg_id(__first, __last);
		if (__ptr == nullptr || __ptr == __last || *__ptr != '}')
		  CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
		__first = __ptr;
		__pc.check_arg_id(__v);
		__val = __v;
	      }
	    ++__first; // past the '}'
	  }
	return __first;
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_width(iterator __first, iterator __last,
		     basic_format_parse_context<_CharT>& __pc)
      {
	bool __arg_id = false;
	if (*__first == '0')
	  __throw_format_error("format error: width must be non-zero in "
			       "format string");
	auto __next = _S_parse_width_or_precision(__first, __last, _M_width,
						  __arg_id, __pc);
	if (__next != __first)
	  _M_width_kind = __arg_id ? _WP_from_arg : _WP_value;
	return __next;
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_precision(iterator __first, iterator __last,
			 basic_format_parse_context<_CharT>& __pc)
      {
	if (__first[0] != '.')
	  return __first;

	++__first;
	bool __arg_id = false;
	auto __next = _S_parse_width_or_precision(__first, __last, _M_prec,
						  __arg_id, __pc);
	if (__next == __first)
	  __throw_format_error("format error: missing precision after '.' in "
			       "format string");
	_M_prec_kind = __arg_id ? _WP_from_arg : _WP_value;
	return __next;
      }

      // pre: __first != __last
      constexpr iterator
      _M_parse_locale(iterator __first, iterator /* __last */) noexcept
      {
	if (*__first == 'L')
	  {
	    _M_localized = true;
	    ++__first;
	  }
	return __first;
      }

      template<typename _Context>
	size_t
	_M_get_width(_Context& __ctx) const
	{
	  size_t __width = 0;
	  if (_M_width_kind == _WP_value)
	    __width = _M_width;
	  else if (_M_width_kind == _WP_from_arg)
	    __width = CXX20_FORMAT_DECORATE_NAME(__format)::__int_from_arg(__ctx.arg(_M_width));
	  return __width;
	}

      template<typename _Context>
	size_t
	_M_get_precision(_Context& __ctx) const
	{
	  size_t __prec = -1;
	  if (_M_prec_kind == _WP_value)
	    __prec = _M_prec;
	  else if (_M_prec_kind == _WP_from_arg)
	    __prec = CXX20_FORMAT_DECORATE_NAME(__format)::__int_from_arg(__ctx.arg(_M_prec));
	  return __prec;
	}
    };

  template<typename _Int>
    inline char*
    __put_sign(_Int __i, _Sign __sign, char* __dest) noexcept
    {
      if (__i < 0)
	*__dest = '-';
      else if (__sign == _Sign_plus)
	*__dest = '+';
      else if (__sign == _Sign_space)
	*__dest = ' ';
      else
	++__dest;
      return __dest;
    }

  // Write STR to OUT (and do so efficiently if OUT is a _Sink_iter).
  template<typename _Out, typename _CharT>
    requires output_iterator<_Out, const _CharT&>
    inline _Out
    __write(_Out __out, basic_string_view<_CharT> __str)
    {
      if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>)
	{
	  if (__str.size())
	    __out = __str;
	}
      else
	for (_CharT __c : __str)
	  *__out++ = __c;
      return __out;
    }

  // Write STR to OUT with NFILL copies of FILL_CHAR specified by ALIGN.
  // pre: __align != _Align_default
  template<typename _Out, typename _CharT>
    _Out
    __write_padded(_Out __out, basic_string_view<_CharT> __str,
		   _Align __align, size_t __nfill, _CharT __fill_char)
    {
      const size_t __buflen = 0x20;
      _CharT __padding_chars[__buflen];
      basic_string_view<_CharT> __padding{__padding_chars, __buflen};

      auto __pad = [&__padding] (size_t __n, _Out& __o) {
	if (__n == 0)
	  return;
	while (__n > __padding.size())
	  {
	    __o = CXX20_FORMAT_DECORATE_NAME(__format)::__write(std::move(__o), __padding);
	    __n -= __padding.size();
	  }
	if (__n != 0)
	  __o = CXX20_FORMAT_DECORATE_NAME(__format)::__write(std::move(__o), __padding.substr(0, __n));
      };

      size_t __l, __r, __max;
      if (__align == _Align_centre)
	{
	  __l = __nfill / 2;
	  __r = __l + (__nfill & 1);
	  __max = __r;
	}
      else if (__align == _Align_right)
	{
	  __l = __nfill;
	  __r = 0;
	  __max = __l;
	}
      else
	{
	  __l = 0;
	  __r = __nfill;
	  __max = __r;
	}
      if (__max < __buflen)
	__padding.remove_suffix(__buflen - __max);
      else
	__max = __buflen;
      char_traits<_CharT>::assign(__padding_chars, __max, __fill_char);

      __pad(__l, __out);
      __out = CXX20_FORMAT_DECORATE_NAME(__format)::__write(std::move(__out), __str);
      __pad(__r, __out);

      return __out;
    }

  // Write STR to OUT, with alignment and padding as determined by SPEC.
  // pre: __spec._M_align != _Align_default || __align != _Align_default
  template<typename _CharT, typename _Out>
    _Out
    __write_padded_as_spec(basic_string_view<type_identity_t<_CharT>> __str,
			   size_t __estimated_width,
			   basic_format_context<_Out, _CharT>& __fc,
			   const _Spec<_CharT>& __spec,
			   _Align __align = _Align_left)
    {
      size_t __width = __spec._M_get_width(__fc);

      if (__width <= __estimated_width)
	return CXX20_FORMAT_DECORATE_NAME(__format)::__write(__fc.out(), __str);

      const size_t __nfill = __width - __estimated_width;

      if (__spec._M_align)
	__align = __spec._M_align;

      return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded(__fc.out(), __str, __align, __nfill,
				      __spec._M_fill);
    }

  // A lightweight optional<locale>.
  struct _Optional_locale
  {
    [[__gnu__::__always_inline__]]
    _Optional_locale() : _M_dummy(), _M_hasval(false) { }

    _Optional_locale(const locale& __loc) noexcept
    : _M_loc(__loc), _M_hasval(true)
    { }

    _Optional_locale(const _Optional_locale& __l) noexcept
    : _M_dummy(), _M_hasval(__l._M_hasval)
    {
      if (_M_hasval)
	std::construct_at(&_M_loc, __l._M_loc);
    }

    _Optional_locale&
    operator=(const _Optional_locale& __l) noexcept
    {
      if (_M_hasval)
	{
	  if (__l._M_hasval)
	    _M_loc = __l._M_loc;
	  else
	    {
	      _M_loc.~locale();
	      _M_hasval = false;
	    }
	}
      else if (__l._M_hasval)
	{
	  std::construct_at(&_M_loc, __l._M_loc);
	  _M_hasval = true;
	}
      return *this;
    }

    ~_Optional_locale() { if (_M_hasval) _M_loc.~locale(); }

    _Optional_locale&
    operator=(locale&& __loc) noexcept
    {
      if (_M_hasval)
	_M_loc = std::move(__loc);
      else
	{
	  std::construct_at(&_M_loc, std::move(__loc));
	  _M_hasval = true;
	}
      return *this;
    }

    const locale&
    value() noexcept
    {
      if (!_M_hasval)
	{
	  std::construct_at(&_M_loc);
	  _M_hasval = true;
	}
      return _M_loc;
    }

    bool has_value() const noexcept { return _M_hasval; }

    union {
      char _M_dummy = '\0';
      std::locale _M_loc;
    };
    bool _M_hasval = false;
  };

  template<typename _CharT>
    concept __char = same_as<_CharT, char> || same_as<_CharT, wchar_t>;

  template<__char _CharT>
    struct __formatter_str
    {
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	auto __first = __pc.begin();
	const auto __last = __pc.end();
	_Spec<_CharT> __spec{};

	auto __finalize = [this, &__spec] {
	  _M_spec = __spec;
	};

	auto __finished = [&] {
	  if (__first == __last || *__first == '}')
	    {
	      __finalize();
	      return true;
	    }
	  return false;
	};

	if (__finished())
	  return __first;

	__first = __spec._M_parse_fill_and_align(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_width(__first, __last, __pc);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_precision(__first, __last, __pc);
	if (__finished())
	  return __first;

	if (*__first == 's')
	  ++__first;
#if __cpp_lib_format_ranges
	else if (*__first == '?')
	  {
	    __spec._M_type = _Pres_esc;
	    ++__first;
	  }
#endif

	if (__finished())
	  return __first;

	CXX20_FORMAT_DECORATE_NAME(__format)::__failed_to_parse_format_spec();
      }

      template<typename _Out>
	_Out
	format(basic_string_view<_CharT> __s,
	       basic_format_context<_Out, _CharT>& __fc) const
	{
	  if (_M_spec._M_type == _Pres_esc)
	    {
	      // TODO: C++23 escaped string presentation
	    }

	  if (_M_spec._M_width_kind == _WP_none
		&& _M_spec._M_prec_kind == _WP_none)
	    return CXX20_FORMAT_DECORATE_NAME(__format)::__write(__fc.out(), __s);

	  size_t __estimated_width = __s.size(); // TODO: Unicode-aware estim.

	  if (_M_spec._M_prec_kind != _WP_none)
	    {
	      size_t __prec = _M_spec._M_get_precision(__fc);
	      if (__estimated_width > __prec)
		{
		  __s = __s.substr(0, __prec); // TODO: do not split code points
		  __estimated_width = __prec;
		}
	    }

	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded_as_spec(__s, __estimated_width,
						  __fc, _M_spec);
	}

#if __cpp_lib_format_ranges
      constexpr void
      set_debug_format() noexcept
      { _M_spec._M_type = _Pres_esc; }
#endif

    private:
      _Spec<_CharT> _M_spec{};
    };

  template<__char _CharT>
    struct __formatter_int
    {
      // If no presentation type is specified, meaning of "none" depends
      // whether we are formatting an integer or a char or a bool.
      static constexpr _Pres_type _AsInteger = _Pres_d;
      static constexpr _Pres_type _AsBool = _Pres_s;
      static constexpr _Pres_type _AsChar = _Pres_c;

      constexpr typename basic_format_parse_context<_CharT>::iterator
      _M_do_parse(basic_format_parse_context<_CharT>& __pc, _Pres_type __type)
      {
	_Spec<_CharT> __spec{};
	__spec._M_type = __type;

	const auto __last = __pc.end();
	auto __first = __pc.begin();

	auto __finalize = [this, &__spec] {
	  _M_spec = __spec;
	};

	auto __finished = [&] {
	  if (__first == __last || *__first == '}')
	    {
	      __finalize();
	      return true;
	    }
	  return false;
	};

	if (__finished())
	  return __first;

	__first = __spec._M_parse_fill_and_align(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_sign(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_alternate_form(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_zero_fill(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_width(__first, __last, __pc);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_locale(__first, __last);
	if (__finished())
	  return __first;

	switch (*__first)
	{
	  case 'b':
	    __spec._M_type = _Pres_b;
	    ++__first;
	    break;
	  case 'B':
	    __spec._M_type = _Pres_B;
	    ++__first;
	    break;
	  case 'c':
	    // _GLIBCXX_RESOLVE_LIB_DEFECTS
	    // 3586. format should not print bool with 'c'
	    if (__type != _AsBool)
	      {
		__spec._M_type = _Pres_c;
		++__first;
	      }
	    break;
	  case 'd':
	    __spec._M_type = _Pres_d;
	    ++__first;
	    break;
	  case 'o':
	    __spec._M_type = _Pres_o;
	    ++__first;
	    break;
	  case 'x':
	    __spec._M_type = _Pres_x;
	    ++__first;
	    break;
	  case 'X':
	    __spec._M_type = _Pres_X;
	    ++__first;
	    break;
	  case 's':
	    if (__type == _AsBool)
	      {
		__spec._M_type = _Pres_s; // same value (and meaning) as "none"
		++__first;
	      }
	    break;
#if __cpp_lib_format_ranges
	  case '?':
	    if (__type == _AsChar)
	      {
		__spec._M_type = _Pres_esc;
		++__first;
	      }
#endif
	    break;
	  }

	if (__finished())
	  return __first;

	CXX20_FORMAT_DECORATE_NAME(__format)::__failed_to_parse_format_spec();
      }

      template<typename _Tp>
	constexpr typename basic_format_parse_context<_CharT>::iterator
	_M_parse(basic_format_parse_context<_CharT>& __pc)
	{
	  if constexpr (is_same_v<_Tp, bool>)
	    {
	      auto __end = _M_do_parse(__pc, _AsBool);
	      if (_M_spec._M_type == _Pres_s)
		if (_M_spec._M_sign || _M_spec._M_alt || _M_spec._M_zero_fill)
		  __throw_format_error("format error: format-spec contains "
				       "invalid formatting options for "
				       "'bool'");
	      return __end;
	    }
	  else if constexpr (__char<_Tp>)
	    {
	      auto __end = _M_do_parse(__pc, _AsChar);
	      if (_M_spec._M_type == _Pres_c || _M_spec._M_type == _Pres_esc)
		if (_M_spec._M_sign || _M_spec._M_alt || _M_spec._M_zero_fill
		      /* XXX should be invalid? || _M_spec._M_localized */)
		  __throw_format_error("format error: format-spec contains "
				       "invalid formatting options for "
				       "'charT'");
	      return __end;
	    }
	  else
	    return _M_do_parse(__pc, _AsInteger);
	}

      template<typename _Int, typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_Int __i, basic_format_context<_Out, _CharT>& __fc) const
	{
	  if (_M_spec._M_type == _Pres_c)
	    return _M_format_character(_S_to_character(__i), __fc);

	  char __buf[sizeof(_Int) * __CHAR_BIT__ + 3];
	  to_chars_result __res{};

	  string_view __base_prefix;
	  make_unsigned_t<_Int> __u;
	  if (__i < 0)
	    __u = -static_cast<make_unsigned_t<_Int>>(__i);
	  else
	    __u = __i;

	  char* __start = __buf + 3;
	  char* const __end = __buf + sizeof(__buf);
	  char* const __start_digits = __start;

	  switch (_M_spec._M_type)
	  {
	    case _Pres_b:
	    case _Pres_B:
	      __base_prefix = _M_spec._M_type == _Pres_b ? "0b" : "0B";
	      __res = CXX20_FORMAT_DECORATE_NAME(__to_chars)(__start, __end, __u, 2);
	      break;
#if 0
	    case _Pres_c:
	      return _M_format_character(_S_to_character(__i), __fc);
#endif
	    case _Pres_none:
	      // Should not reach here with _Pres_none for bool or charT, so:
	      [[fallthrough]];
	    case _Pres_d:
	      __res = CXX20_FORMAT_DECORATE_NAME(__to_chars)(__start, __end, __u, 10);
	      break;
	    case _Pres_o:
	      if (__i != 0)
		__base_prefix = "0";
	      __res = CXX20_FORMAT_DECORATE_NAME(__to_chars)(__start, __end, __u, 8);
	      break;
	    case _Pres_x:
	    case _Pres_X:
	      __base_prefix = _M_spec._M_type == _Pres_x ? "0x" : "0X";
	      __res = CXX20_FORMAT_DECORATE_NAME(__to_chars)(__start, __end, __u, 16);
	      if (_M_spec._M_type == _Pres_X)
		for (auto __p = __start; __p != __res.ptr; ++__p)
#if __has_builtin(__builtin_toupper)
		  *__p = __builtin_toupper(*__p);
#else
		  *__p = std::toupper(*__p);
#endif
	      break;
	    default:
	      __builtin_unreachable();
	  }

	  if (_M_spec._M_alt && __base_prefix.size())
	    {
	      __start -= __base_prefix.size();
	      __builtin_memcpy(__start, __base_prefix.data(),
			       __base_prefix.size());
	    }
	  __start = CXX20_FORMAT_DECORATE_NAME(__format)::__put_sign(__i, _M_spec._M_sign, __start - 1);

	  return _M_format_int(string_view(__start, __res.ptr - __start),
			       __start_digits - __start, __fc);
	}

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(bool __i, basic_format_context<_Out, _CharT>& __fc) const
	{
	  if (_M_spec._M_type == _Pres_c)
	    return _M_format_character(static_cast<unsigned char>(__i), __fc);
	  if (_M_spec._M_type != _Pres_s)
	    return format(static_cast<unsigned char>(__i), __fc);

	  basic_string<_CharT> __s;
	  size_t __est_width;
	  if (_M_spec._M_localized) [[unlikely]]
	    {
	      auto& __np = std::use_facet<numpunct<_CharT>>(__fc.locale());
	      __s = __i ? __np.truename() : __np.falsename();
	      __est_width = __s.size(); // TODO Unicode-aware estimate
	    }
	  else
	    {
	      if constexpr (is_same_v<char, _CharT>)
		__s = __i ? "true" : "false";
	      else
		__s = __i ? L"true" : L"false";
	      __est_width = __s.size();
	    }

	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded_as_spec(__s, __est_width, __fc,
						  _M_spec);
	}

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	_M_format_character(_CharT __c,
		      basic_format_context<_Out, _CharT>& __fc) const
	{
	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded_as_spec({&__c, 1u}, 1, __fc, _M_spec);
	}

      template<typename _Int>
	static _CharT
	_S_to_character(_Int __i)
	{
	  using _Traits = __gnu_cxx::__int_traits<_CharT>;
	  if constexpr (is_signed_v<_Int> == is_signed_v<_CharT>)
	    {
	      if (_Traits::__min <= __i && __i <= _Traits::__max)
		return static_cast<_CharT>(__i);
	    }
	  else if constexpr (is_signed_v<_Int>)
	    {
	      if (__i >= 0 && make_unsigned_t<_Int>(__i) <= _Traits::__max)
		return static_cast<_CharT>(__i);
	    }
	  else if (__i <= make_unsigned_t<_CharT>(_Traits::__max))
	    return static_cast<_CharT>(__i);
	  __throw_format_error("format error: integer not representable as "
			       "character");
	}

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	_M_format_int(string_view __narrow_str, size_t __prefix_len,
		      basic_format_context<_Out, _CharT>& __fc) const
	{
	  size_t __width = _M_spec._M_get_width(__fc);

	  _Optional_locale __loc;

	  basic_string_view<_CharT> __str;
	  if constexpr (is_same_v<char, _CharT>)
	    __str = __narrow_str;
	  else
	    {
	      __loc = __fc.locale();
	      auto& __ct = use_facet<ctype<_CharT>>(__loc.value());
	      size_t __n = __narrow_str.size();
	      auto __p = (_CharT*)__builtin_alloca(__n * sizeof(_CharT));
	      __ct.widen(__narrow_str.data(), __narrow_str.data() + __n, __p);
	      __str = {__p, __n};
	    }

	  if (_M_spec._M_localized)
	    {
	      if constexpr (is_same_v<char, _CharT>)
		__loc = __fc.locale();
	      const auto& __l = __loc.value();
	      if (__l.name() != "C")
		{
		  auto& __np = use_facet<numpunct<_CharT>>(__l);
		  string __grp = __np.grouping();
		  if (!__grp.empty())
		    {
		      size_t __n = __str.size() - __prefix_len;
		      auto __p = (_CharT*)__builtin_alloca(2 * __n
							     * sizeof(_CharT)
							     + __prefix_len);
		      auto __s = __str.data();
		      char_traits<_CharT>::copy(__p, __s, __prefix_len);
		      __s += __prefix_len;
		      auto __end = std::__add_grouping(__p + __prefix_len,
						       __np.thousands_sep(),
						       __grp.data(),
						       __grp.size(),
						       __s, __s + __n);
		      __str = {__p, size_t(__end - __p)};
		    }
		}
	    }

	  if (__width <= __str.size())
	    return CXX20_FORMAT_DECORATE_NAME(__format)::__write(__fc.out(), __str);

	  _CharT __fill_char = _M_spec._M_fill;
	  _Align __align = _M_spec._M_align;

	  size_t __nfill = __width - __str.size();
	  auto __out = __fc.out();
	  if (__align == _Align_default)
	    {
	      __align = _Align_right;
	      if (_M_spec._M_zero_fill)
		{
		  __fill_char = _CharT('0');
		  // Write sign and base prefix before zero filling.
		  if (__prefix_len != 0)
		    {
		      __out = CXX20_FORMAT_DECORATE_NAME(__format)::__write(std::move(__out),
						__str.substr(0, __prefix_len));
		      __str.remove_prefix(__prefix_len);
		    }
		}
	      else
		__fill_char = _CharT(' ');
	    }
	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded(std::move(__out), __str,
					  __align, __nfill, __fill_char);
	}

#if defined __SIZEOF_INT128__ && defined __STRICT_ANSI__
      template<typename _Tp>
	using make_unsigned_t
	  = typename __conditional_t<(sizeof(_Tp) <= sizeof(long long)),
				     std::make_unsigned<_Tp>,
				     type_identity<unsigned __int128>>::type;

      // std::to_chars is not overloaded for int128 in strict mode.
      template<typename _Int>
	static to_chars_result
	CXX20_FORMAT_DECORATE_NAME(__to_chars)(char* __first, char* __last, _Int __value, int __base)
	{ return std::CXX20_FORMAT_NAMESPACE::__to_chars_i<_Int>(__first, __last, __value, __base); }
#endif

      _Spec<_CharT> _M_spec{};
    };

  // Decide how 128-bit floating-point types should be formatted (or not).
  // When supported, the typedef __format::__float128_t is the type that
  // format arguments should be converted to for storage in basic_format_arg.
  // Define the macro CXX20_FORMAT_FORMAT_F128 to say they're supported.
  // CXX20_FORMAT_FORMAT_F128=1 means __float128, _Float128 etc. will be formatted
  // by converting them to long double (or __ieee128 for powerpc64le).
  // CXX20_FORMAT_FORMAT_F128=2 means basic_format_arg needs to enable explicit
  // support for _Float128, rather than formatting it as another type.
#undef CXX20_FORMAT_FORMAT_F128

#ifdef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT

  // Format 128-bit floating-point types using __ieee128.
  using __float128_t = __ieee128;
# define CXX20_FORMAT_FORMAT_F128 1

#ifdef __LONG_DOUBLE_IEEE128__
  // These overloads exist in the library, but are not declared.
  // Make them available as std::__format::to_chars.
  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ibm128) noexcept
    __asm("_ZSt8to_charsPcS_e");

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ibm128, chars_format) noexcept
    __asm("_ZSt8to_charsPcS_eSt12chars_format");

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ibm128, chars_format, int) noexcept
    __asm("_ZSt8to_charsPcS_eSt12chars_formati");
#elif __cplusplus == 202002L
  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ieee128) noexcept
    __asm("_ZSt8to_charsPcS_u9__ieee128");

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ieee128, chars_format) noexcept
    __asm("_ZSt8to_charsPcS_u9__ieee128St12chars_format");

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, __ieee128, chars_format, int) noexcept
    __asm("_ZSt8to_charsPcS_u9__ieee128St12chars_formati");
#endif

#elif defined _GLIBCXX_LDOUBLE_IS_IEEE_BINARY128

  // Format 128-bit floating-point types using long double.
  using __float128_t = long double;
# define CXX20_FORMAT_FORMAT_F128 1

#elif __FLT128_DIG__ && defined(_GLIBCXX_HAVE_FLOAT128_MATH)

  // Format 128-bit floating-point types using _Float128.
  using __float128_t = _Float128;
# define CXX20_FORMAT_FORMAT_F128 2

# if __cplusplus == 202002L
  // These overloads exist in the library, but are not declared for C++20.
  // Make them available as std::__format::to_chars.
  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, _Float128) noexcept
#  if _GLIBCXX_INLINE_VERSION
    __asm("_ZNSt3__88to_charsEPcS0_DF128_");
#  else
    __asm("_ZSt8to_charsPcS_DF128_");
#  endif

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, _Float128, chars_format) noexcept
#  if _GLIBCXX_INLINE_VERSION
    __asm("_ZNSt3__88to_charsEPcS0_DF128_NS_12chars_formatE");
#  else
    __asm("_ZSt8to_charsPcS_DF128_St12chars_format");
#  endif

  to_chars_result
  CXX20_FORMAT_DECORATE_NAME(__to_chars)(char*, char*, _Float128, chars_format, int) noexcept
#  if _GLIBCXX_INLINE_VERSION
    __asm("_ZNSt3__88to_charsEPcS0_DF128_NS_12chars_formatEi");
#  else
    __asm("_ZSt8to_charsPcS_DF128_St12chars_formati");
#  endif
# endif
#endif

  using std::CXX20_FORMAT_NAMESPACE::CXX20_FORMAT_DECORATE_NAME(__to_chars);

  // We can format a floating-point type iff it is usable with to_chars.
  template<typename _Tp>
    concept __formattable_float = requires (_Tp __t, char* __p)
    { CXX20_FORMAT_DECORATE_NAME(__format)::CXX20_FORMAT_DECORATE_NAME(__to_chars)(__p, __p, __t, chars_format::scientific, 6); };

  template<__char _CharT>
    struct __formatter_fp
    {
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	_Spec<_CharT> __spec{};
	const auto __last = __pc.end();
	auto __first = __pc.begin();

	auto __finalize = [this, &__spec] {
	  _M_spec = __spec;
	};

	auto __finished = [&] {
	  if (__first == __last || *__first == '}')
	    {
	      __finalize();
	      return true;
	    }
	  return false;
	};

	if (__finished())
	  return __first;

	__first = __spec._M_parse_fill_and_align(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_sign(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_alternate_form(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_zero_fill(__first, __last);
	if (__finished())
	  return __first;

	if (__first[0] != '.')
	  {
	    __first = __spec._M_parse_width(__first, __last, __pc);
	    if (__finished())
	      return __first;
	  }

	__first = __spec._M_parse_precision(__first, __last, __pc);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_locale(__first, __last);
	if (__finished())
	  return __first;

	switch (*__first)
	{
	  case 'a':
	    __spec._M_type = _Pres_a;
	    ++__first;
	    break;
	  case 'A':
	    __spec._M_type = _Pres_A;
	    ++__first;
	    break;
	  case 'e':
	    __spec._M_type = _Pres_e;
	    ++__first;
	    break;
	  case 'E':
	    __spec._M_type = _Pres_E;
	    ++__first;
	    break;
	  case 'f':
	  case 'F':
	    __spec._M_type = _Pres_f;
	    ++__first;
	    break;
	  case 'g':
	    __spec._M_type = _Pres_g;
	    ++__first;
	    break;
	  case 'G':
	    __spec._M_type = _Pres_G;
	    ++__first;
	    break;
	  }

	if (__finished())
	  return __first;

	CXX20_FORMAT_DECORATE_NAME(__format)::__failed_to_parse_format_spec();
      }

      template<typename _Fp, typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_Fp __v, basic_format_context<_Out, _CharT>& __fc) const
	{
	  std::string __dynbuf;
	  char __buf[128];
	  to_chars_result __res{};

	  size_t __prec = 6;
	  bool __use_prec = _M_spec._M_prec_kind != _WP_none;
	  if (__use_prec)
	    __prec = _M_spec._M_get_precision(__fc);

	  char* __start = __buf + 1; // reserve space for sign
	  char* __end = __buf + sizeof(__buf);

	  chars_format __fmt{};
	  bool __upper = false;
	  bool __trailing_zeros = false;
	  char __expc = 0;

	  switch (_M_spec._M_type)
	  {
	    case _Pres_A:
	      __upper = true;
	      [[fallthrough]];
	    case _Pres_a:
	      __expc = 'p';
	      __fmt = chars_format::hex;
	      break;
	    case _Pres_E:
	      __upper = true;
	      [[fallthrough]];
	    case _Pres_e:
	      __expc = 'e';
	      __use_prec = true;
	      __fmt = chars_format::scientific;
	      break;
	    case _Pres_f:
	      __use_prec = true;
	      __fmt = chars_format::fixed;
	      break;
	    case _Pres_G:
	      __upper = true;
	      [[fallthrough]];
	    case _Pres_g:
	      __trailing_zeros = true;
	      __expc = 'e';
	      __use_prec = true;
	      __fmt = chars_format::general;
	      break;
	    case _Pres_none:
	      if (__use_prec)
		__fmt = chars_format::general;
	      break;
	  }

	  // Write value into buffer using std::to_chars.
	  auto __to_chars = [&](char* __b, char* __e) {
	    if (__use_prec)
	      return CXX20_FORMAT_DECORATE_NAME(__format)::CXX20_FORMAT_DECORATE_NAME(__to_chars)(__b, __e, __v, __fmt, __prec);
	    else if (__fmt != chars_format{})
	      return CXX20_FORMAT_DECORATE_NAME(__format)::CXX20_FORMAT_DECORATE_NAME(__to_chars)(__b, __e, __v, __fmt);
	    else
	      return CXX20_FORMAT_DECORATE_NAME(__format)::CXX20_FORMAT_DECORATE_NAME(__to_chars)(__b, __e, __v);
	  };

	  // First try using stack buffer.
	  __res = __to_chars(__start, __end);

	  if (__builtin_expect(__res.ec == errc::value_too_large, 0))
	    {
	      // If the buffer is too small it's probably because of a large
	      // precision, or a very large value in fixed format.
	      size_t __guess =  __prec + sizeof(__buf);
	      if (__fmt == chars_format::fixed)
		__guess += max((int)__builtin_log10(__builtin_abs(__v)) / 2, 1);
	      __dynbuf.reserve(__guess);

	      do
		{
		  auto __overwrite = [&__to_chars, &__res] (char* __p, size_t __n)
		  {
		    __res = __to_chars(__p + 1, __p + __n - 1);
		    return __res.ec == errc{} ? __res.ptr - __p : 0;
		  };

		  _S_resize_and_overwrite(__dynbuf, __dynbuf.capacity() * 2,
					  __overwrite);
		  __start = __dynbuf.data() + 1; // reserve space for sign
		  __end = __dynbuf.data() + __dynbuf.size();
		}
	      while (__builtin_expect(__res.ec == errc::value_too_large, 0));
	  }

	  // Use uppercase for 'A', 'E', and 'G' formats.
	  if (__upper)
	    {
	      for (char* __p = __start; __p != __res.ptr; ++__p)
		*__p = std::toupper(*__p);
	      __expc = std::toupper(__expc);
	    }

	  // Add sign for non-negative values.
	  if (!__builtin_signbit(__v))
	    {
	      if (_M_spec._M_sign == _Sign_plus)
		*--__start = '+';
	      else if (_M_spec._M_sign == _Sign_space)
		*--__start = ' ';
	    }

	  string_view __narrow_str(__start, __res.ptr - __start);

	  // Use alternate form.
	  if (_M_spec._M_alt && __builtin_isfinite(__v))
	    {
	      string_view __s = __narrow_str;
	      size_t __z = 0;
	      size_t __p;
	      size_t __d = __s.find('.');
	      size_t __sigfigs;
	      if (__d != __s.npos)
		{
		  __p = __s.find(__expc, __d + 1);
		  if (__p == __s.npos)
		    __p = __s.size();
		  __sigfigs = __p - 1;
		}
	      else
		{
		  __p = __s.find(__expc);
		  if (__p == __s.npos)
		    __p = __s.size();
		  __d = __p;
		  __sigfigs = __d;
		}

	      if (__trailing_zeros)
		{
		  if (!CXX20_FORMAT_DECORATE_NAME(__format)::__is_xdigit(__s[0]))
		    --__sigfigs;
		  __z = __prec - __sigfigs;
		}

	      if (size_t __extras = int(__d == __p) + __z)
		{
		  if (__dynbuf.empty() && __extras <= size_t(__end - __res.ptr))
		    {
		      // Move exponent to make space for extra chars.
		      __builtin_memmove(__start + __p + __extras,
					__start + __p,
					__s.size() - __p);

		      if (__d == __p)
			__start[__p++] = '.';
		      __builtin_memset(__start + __p, '0', __z);
		      __narrow_str = {__s.data(), __s.size() + __extras};
		    }
		  else
		    {
		      __dynbuf.reserve(__s.size() + __extras);
		      if (__dynbuf.empty())
			{
			  __dynbuf = __s.substr(0, __p);
			  if (__d == __p)
			    __dynbuf += '.';
			  if (__z)
			    __dynbuf.append(__z, '0');
			}
		      else
			{
			  __dynbuf.insert(__p, __extras, '0');
			  if (__d == __p)
			    __dynbuf[__p] = '.';
			}
		      __narrow_str = __dynbuf;
		    }
		}
	    }

	  // TODO move everything below to a new member function that
	  // doesn't depend on _Fp type.


	  _Optional_locale __loc;
	  basic_string_view<_CharT> __str;
	  basic_string<_CharT> __wstr;
	  if constexpr (is_same_v<_CharT, char>)
	    __str = __narrow_str;
	  else
	    {
	      __loc = __fc.locale();
	      auto& __ct = use_facet<ctype<_CharT>>(__loc.value());
	      const char* __data = __narrow_str.data();
	      auto __overwrite = [&__data, &__ct](_CharT* __p, size_t __n)
	      {
		__ct.widen(__data, __data + __n, __p);
		return __n;
	      };
	      _S_resize_and_overwrite(__wstr, __narrow_str.size(), __overwrite);
	      __str = __wstr;
	    }

	  if (_M_spec._M_localized)
	    {
	      if constexpr (is_same_v<char, _CharT>)
		__wstr = _M_localize(__str, __expc, __fc.locale());
	      else
		__wstr = _M_localize(__str, __expc, __loc.value());
	      __str = __wstr;
	    }

	  size_t __width = _M_spec._M_get_width(__fc);

	  if (__width <= __str.size())
	    return CXX20_FORMAT_DECORATE_NAME(__format)::__write(__fc.out(), __str);

	  _CharT __fill_char = _M_spec._M_fill;
	  _Align __align = _M_spec._M_align;

	  size_t __nfill = __width - __str.size();
	  auto __out = __fc.out();
	  if (__align == _Align_default)
	    {
	      __align = _Align_right;
	      if (_M_spec._M_zero_fill && __builtin_isfinite(__v))
		{
		  __fill_char = _CharT('0');
		  // Write sign before zero filling.
		  if (!CXX20_FORMAT_DECORATE_NAME(__format)::__is_xdigit(__narrow_str[0]))
		    {
		      *__out++ = __str[0];
		      __str.remove_prefix(1);
		    }
		}
	      else
		__fill_char = _CharT(' ');
	    }
	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded(std::move(__out), __str,
					  __align, __nfill, __fill_char);
	}

      // Locale-specific format.
      basic_string<_CharT>
      _M_localize(basic_string_view<_CharT> __str, char __expc,
		  const locale& __loc) const
      {
	basic_string<_CharT> __lstr;

	if (__loc == locale::classic())
	  return __lstr; // Nothing to do.

	const auto& __np = use_facet<numpunct<_CharT>>(__loc);
	const _CharT __point = __np.decimal_point();
	const string __grp = __np.grouping();

	_CharT __dot, __exp;
	if constexpr (is_same_v<_CharT, char>)
	  {
	    __dot = '.';
	    __exp = __expc;
	  }
	else
	  {
	    const auto& __ct = use_facet<ctype<_CharT>>(__loc);
	    __dot = __ct.widen('.');
	    __exp = __ct.widen(__expc);
	  }

	if (__grp.empty() && __point == __dot)
	  return __lstr; // Locale uses '.' and no grouping.

	size_t __d = __str.find(__dot);
	size_t __e = min(__d, __str.find(__exp));
	if (__e == __str.npos)
	  __e = __str.size();
	const size_t __r = __str.size() - __e;
	auto __overwrite = [&](_CharT* __p, size_t) {
	  auto __end = std::__add_grouping(__p, __np.thousands_sep(),
					   __grp.data(), __grp.size(),
					   __str.data(), __str.data() + __e);
	  if (__r)
	    {
	      if (__d != __str.npos)
		{
		  *__end = __point;
		  ++__end;
		  ++__e;
		}
	      if (__r > 1)
		__end += __str.copy(__end, __str.npos, __e);
	    }
	  return (__end - __p);
	};
	_S_resize_and_overwrite(__lstr, __e * 2 + __r, __overwrite);
	return __lstr;
      }

      template<typename _Ch, typename _Func>
	static void
	_S_resize_and_overwrite(basic_string<_Ch>& __str, size_t __n, _Func __f)
	{
#if __cpp_lib_string_resize_and_overwrite
	  __str.resize_and_overwrite(__n, __f);
#else
	  __str.resize(__n);
	  __str.resize(__f(__str.data(), __n));
#endif
	}

      _Spec<_CharT> _M_spec{};
    };

} // namespace __format
/// @endcond

  // Format a character.
  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<_CharT, _CharT>
    {
      formatter() = default;

      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	return _M_f.template _M_parse<_CharT>(__pc);
      }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_CharT __u, basic_format_context<_Out, _CharT>& __fc) const
	{
	  if (_M_f._M_spec._M_type == CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_none)
	    return _M_f._M_format_character(__u, __fc);
	  else if (_M_f._M_spec._M_type == CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_esc)
	    {
	      // TODO
	      return __fc.out();
	    }
	  else
	    return _M_f.format(__u, __fc);
	}

#if __cpp_lib_format_ranges
      constexpr void
      set_debug_format() noexcept
      { _M_f._M_spec._M_type = CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_esc; }
#endif

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_int<_CharT> _M_f;
    };

  // Format a char value for wide character output.
  template<>
    struct formatter<char, wchar_t>
    {
      formatter() = default;

      constexpr typename basic_format_parse_context<wchar_t>::iterator
      parse(basic_format_parse_context<wchar_t>& __pc)
      {
	return _M_f._M_parse<char>(__pc);
      }

      template<typename _Out>
	typename basic_format_context<_Out, wchar_t>::iterator
	format(char __u, basic_format_context<_Out, wchar_t>& __fc) const
	{
	  if (_M_f._M_spec._M_type == CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_none)
	    return _M_f._M_format_character(__u, __fc);
	  else if (_M_f._M_spec._M_type == CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_esc)
	    {
	      // TODO
	      return __fc.out();
	    }
	  else
	    return _M_f.format(__u, __fc);
	}

      constexpr void
      set_debug_format() noexcept
      { _M_f._M_spec._M_type = CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_esc; }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_int<wchar_t> _M_f;
    };

  /** Format a string.
   * @{
   */
  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<_CharT*, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	[[__gnu__::__nonnull__]]
	typename basic_format_context<_Out, _CharT>::iterator
	format(_CharT* __u, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__u, __fc); }

      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<_CharT> _M_f;
    };

  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<const _CharT*, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	[[__gnu__::__nonnull__]]
	typename basic_format_context<_Out, _CharT>::iterator
	format(const _CharT* __u,
	       basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__u, __fc); }

      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<_CharT> _M_f;
    };

  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT, size_t _Nm>
    struct formatter<_CharT[_Nm], _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(const _CharT (&__u)[_Nm],
	       basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format({__u, _Nm}, __fc); }

      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<_CharT> _M_f;
    };

  template<typename _Traits, typename _Alloc>
    struct formatter<basic_string<char, _Traits, _Alloc>, char>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<char>::iterator
      parse(basic_format_parse_context<char>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, char>::iterator
	format(const basic_string<char, _Traits, _Alloc>& __u,
	       basic_format_context<_Out, char>& __fc) const
	{ return _M_f.format(__u, __fc); }

#if __cpp_lib_format_ranges
      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }
#endif

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<char> _M_f;
    };

  template<typename _Traits, typename _Alloc>
    struct formatter<basic_string<wchar_t, _Traits, _Alloc>, wchar_t>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<wchar_t>::iterator
      parse(basic_format_parse_context<wchar_t>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, wchar_t>::iterator
	format(const basic_string<wchar_t, _Traits, _Alloc>& __u,
	       basic_format_context<_Out, wchar_t>& __fc) const
	{ return _M_f.format(__u, __fc); }

#if __cpp_lib_format_ranges
      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }
#endif

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<wchar_t> _M_f;
    };

  template<typename _Traits>
    struct formatter<basic_string_view<char, _Traits>, char>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<char>::iterator
      parse(basic_format_parse_context<char>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, char>::iterator
	format(basic_string_view<char, _Traits> __u,
	       basic_format_context<_Out, char>& __fc) const
	{ return _M_f.format(__u, __fc); }

#if __cpp_lib_format_ranges
      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }
#endif

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<char> _M_f;
    };

  template<typename _Traits>
    struct formatter<basic_string_view<wchar_t, _Traits>, wchar_t>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<wchar_t>::iterator
      parse(basic_format_parse_context<wchar_t>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, wchar_t>::iterator
	format(basic_string_view<wchar_t, _Traits> __u,
	       basic_format_context<_Out, wchar_t>& __fc) const
	{ return _M_f.format(__u, __fc); }

#if __cpp_lib_format_ranges
      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }
#endif

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_str<wchar_t> _M_f;
    };
  /// @}

  /// Format an integer.
  template<integral _Tp, CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    requires (!__is_one_of<_Tp, char, wchar_t, char16_t, char32_t>::value)
    struct formatter<_Tp, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	return _M_f.template _M_parse<_Tp>(__pc);
      }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_Tp __u, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__u, __fc); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_int<_CharT> _M_f;
    };

#if defined __SIZEOF_INT128__ && defined __STRICT_ANSI__
  template<typename _Tp, CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    requires (__is_one_of<_Tp, __int128, unsigned __int128>::value)
    struct formatter<_Tp, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	return _M_f.template _M_parse<_Tp>(__pc);
      }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_Tp __u, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__u, __fc); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_int<_CharT> _M_f;
    };
#endif

  /// Format a floating-point value.
  template<CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_float _Tp, CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<_Tp, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(_Tp __u, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__u, __fc); }

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::__formatter_fp<_CharT> _M_f;
    };

  /** Format a pointer.
   * @{
   */
  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<const void*, _CharT>
    {
      formatter() = default;

      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	CXX20_FORMAT_DECORATE_NAME(__format)::_Spec<_CharT> __spec{};
	const auto __last = __pc.end();
	auto __first = __pc.begin();

	auto __finalize = [this, &__spec] {
	  _M_spec = __spec;
	};

	auto __finished = [&] {
	  if (__first == __last || *__first == '}')
	    {
	      __finalize();
	      return true;
	    }
	  return false;
	};

	if (__finished())
	  return __first;

	__first = __spec._M_parse_fill_and_align(__first, __last);
	if (__finished())
	  return __first;

// _GLIBCXX_RESOLVE_LIB_DEFECTS
// P2510R3 Formatting pointers
#define CXX20_P2518R3 (__cplusplus > 202302L || ! defined __STRICT_ANSI__)

#if CXX20_P2518R3
	__first = __spec._M_parse_zero_fill(__first, __last);
	if (__finished())
	  return __first;
#endif

	__first = __spec._M_parse_width(__first, __last, __pc);

	if (__first != __last)
	  {
	    if (*__first == 'p')
	      ++__first;
#if CXX20_P2518R3
	    else if (*__first == 'P')
	    {
	      // _GLIBCXX_RESOLVE_LIB_DEFECTS
	      // P2510R3 Formatting pointers
	      __spec._M_type = CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_P;
	      ++__first;
	    }
#endif
	  }

	if (__finished())
	  return __first;

	CXX20_FORMAT_DECORATE_NAME(__format)::__failed_to_parse_format_spec();
      }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(const void* __v, basic_format_context<_Out, _CharT>& __fc) const
	{
	  auto __u = reinterpret_cast<__UINTPTR_TYPE__>(__v);
	  char __buf[2 + sizeof(__v) * 2];
	  auto [__ptr, __ec] = std::CXX20_FORMAT_NAMESPACE::CXX20_FORMAT_DECORATE_NAME(__to_chars)(__buf + 2, std::end(__buf),
					     __u, 16);
	  int __n = __ptr - __buf;
	  __buf[0] = '0';
	  __buf[1] = 'x';
#if CXX20_P2518R3
	  if (_M_spec._M_type == CXX20_FORMAT_DECORATE_NAME(__format)::_Pres_P)
	    {
	      __buf[1] = 'X';
	      for (auto __p = __buf + 2; __p != __ptr; ++__p)
#if __has_builtin(__builtin_toupper)
		*__p = __builtin_toupper(*__p);
#else
		*__p = std::toupper(*__p);
#endif
	    }
#endif

	  basic_string_view<_CharT> __str;
	  if constexpr (is_same_v<_CharT, char>)
	    __str = string_view(__buf, __n);
	  else
	    {
	      const std::locale& __loc = __fc.locale();
	      auto& __ct = use_facet<ctype<_CharT>>(__loc);
	      auto __p = (_CharT*)__builtin_alloca(__n * sizeof(_CharT));
	      __ct.widen(__buf, __buf + __n, __p);
	      __str = wstring_view(__p, __n);
	    }

#if CXX20_P2518R3
	  if (_M_spec._M_zero_fill)
	    {
	      size_t __width = _M_spec._M_get_width(__fc);
	      if (__width <= __str.size())
		return CXX20_FORMAT_DECORATE_NAME(__format)::__write(__fc.out(), __str);

	      auto __out = __fc.out();
	      // Write "0x" or "0X" prefix before zero-filling.
	      __out = CXX20_FORMAT_DECORATE_NAME(__format)::__write(std::move(__out), __str.substr(0, 2));
	      __str.remove_prefix(2);
	      size_t __nfill = __width - __n;
	      return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded(std::move(__out), __str,
					      CXX20_FORMAT_DECORATE_NAME(__format)::_Align_right,
					      __nfill, _CharT('0'));
	    }
#endif

	  return CXX20_FORMAT_DECORATE_NAME(__format)::__write_padded_as_spec(__str, __n, __fc, _M_spec,
						  CXX20_FORMAT_DECORATE_NAME(__format)::_Align_right);
	}

    private:
      CXX20_FORMAT_DECORATE_NAME(__format)::_Spec<_CharT> _M_spec{};
    };

  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<void*, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(void* __v, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__v, __fc); }

    private:
      formatter<const void*, _CharT> _M_f;
    };

  template<CXX20_FORMAT_DECORATE_NAME(__format)::__char _CharT>
    struct formatter<nullptr_t, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(nullptr_t, basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(nullptr, __fc); }

    private:
      formatter<const void*, _CharT> _M_f;
    };
  /// @}


/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  template<typename _Tp, typename _Context,
	   typename _Formatter
	     = typename _Context::template formatter_type<remove_const_t<_Tp>>,
	   typename _ParseContext
	     = basic_format_parse_context<typename _Context::char_type>>
    concept __parsable_with
      = semiregular<_Formatter>
	  && requires (_Formatter __f, _ParseContext __pc)
    {
      { __f.parse(__pc) } -> same_as<typename _ParseContext::iterator>;
    };

  template<typename _Tp, typename _Context,
	   typename _Formatter
	     = typename _Context::template formatter_type<remove_const_t<_Tp>>,
	   typename _ParseContext
	     = basic_format_parse_context<typename _Context::char_type>>
    concept __formattable_with
      = semiregular<_Formatter>
	  && requires (const _Formatter __cf, _Tp&& __t, _Context __fc)
    {
      { __cf.format(__t, __fc) } -> same_as<typename _Context::iterator>;
    };

  // An unspecified output iterator type used in the `formattable` concept.
  template<typename _CharT>
    using _Iter_for = back_insert_iterator<basic_string<_CharT>>;

  template<typename _Tp, typename _CharT,
	   typename _Context = basic_format_context<_Iter_for<_CharT>, _CharT>>
    concept __formattable_impl
      = __parsable_with<_Tp, _Context> && __formattable_with<_Tp, _Context>;

} // namespace __format
/// @endcond

#if __cplusplus > 202002L
  // [format.formattable], concept formattable
  template<typename _Tp, typename _CharT>
    concept formattable
      = CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_impl<remove_reference_t<_Tp>, _CharT>;
#endif

#if __cpp_lib_format_ranges
  /// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  template<typename _Rg, typename _CharT>
    concept __const_formattable_range
      = ranges::input_range<const _Rg>
	  && formattable<ranges::range_reference_t<const _Rg>, _CharT>;

  template<typename _Rg, typename _CharT>
    using __maybe_const_range
      = conditional_t<__const_formattable_range<_Rg, _CharT>, const _Rg, _Rg>;
} // namespace __format
  /// @endcond
#endif // format_ranges

  /// An iterator after the last character written, and the number of
  /// characters that would have been written.
  template<typename _Out>
    struct format_to_n_result
    {
      _Out out;
      iter_difference_t<_Out> size;
    };

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  template<typename _CharT>
    class _Sink_iter
    {
      _Sink<_CharT>* _M_sink = nullptr;

    public:
      using iterator_category = output_iterator_tag;
      using value_type = void;
      using difference_type = ptrdiff_t;
      using pointer = void;
      using reference = void;

      _Sink_iter() = default;
      _Sink_iter(const _Sink_iter&) = default;
      _Sink_iter& operator=(const _Sink_iter&) = default;

      [[__gnu__::__always_inline__]]
      explicit constexpr
      _Sink_iter(_Sink<_CharT>& __sink) : _M_sink(std::addressof(__sink)) { }

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter&
      operator=(_CharT __c)
      {
	_M_sink->_M_write(__c);
	return *this;
      }

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter&
      operator=(basic_string_view<_CharT> __s)
      {
	_M_sink->_M_write(__s);
	return *this;
      }

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter&
      operator*() { return *this; }

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter&
      operator++() { return *this; }

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter
      operator++(int) { return *this; }
    };

  // Abstract base class for type-erased character sinks.
  // All formatting and output is done via this type's iterator,
  // to reduce the number of different template instantiations.
  template<typename _CharT>
    class _Sink
    {
      friend class _Sink_iter<_CharT>;

      span<_CharT> _M_span;
      typename span<_CharT>::iterator _M_next;

      // Called when the span is full, to make more space available.
      // Precondition: _M_next != _M_span.begin()
      // Postcondition: _M_next != _M_span.end()
      virtual void _M_overflow() = 0;

    protected:
      // Precondition: __span.size() != 0
      [[__gnu__::__always_inline__]]
      explicit constexpr
      _Sink(span<_CharT> __span) noexcept
      : _M_span(__span), _M_next(__span.begin())
      { }

      // The portion of the span that has been written to.
      [[__gnu__::__always_inline__]]
      span<_CharT>
      _M_used() const noexcept
      { return _M_span.first(_M_next - _M_span.begin()); }

      // The portion of the span that has not been written to.
      [[__gnu__::__always_inline__]]
      constexpr span<_CharT>
      _M_unused() const noexcept
      { return _M_span.subspan(_M_next - _M_span.begin()); }

      // Use the start of the span as the next write position.
      [[__gnu__::__always_inline__]]
      constexpr void
      _M_rewind() noexcept
      { _M_next = _M_span.begin(); }

      // Replace the current output range.
      void
      _M_reset(span<_CharT> __s,
	       typename span<_CharT>::iterator __next) noexcept
      {
	_M_span = __s;
	_M_next = __next;
      }

      // Called by the iterator for *it++ = c
      constexpr void
      _M_write(_CharT __c)
      {
	*_M_next++ = __c;
	if (_M_next - _M_span.begin() == std::ssize(_M_span)) [[unlikely]]
	  _M_overflow();
      }

      constexpr void
      _M_write(basic_string_view<_CharT> __s)
      {
	span __to = _M_unused();
	while (__to.size() <= __s.size())
	  {
	    __s.copy(__to.data(), __to.size());
	    _M_next += __to.size();
	    __s.remove_prefix(__to.size());
	    _M_overflow();
	    __to = _M_unused();
	  }
	if (__s.size())
	  {
	    __s.copy(__to.data(), __s.size());
	    _M_next += __s.size();
	  }
      }

    public:
      _Sink(const _Sink&) = delete;
      _Sink& operator=(const _Sink&) = delete;

      [[__gnu__::__always_inline__]]
      constexpr _Sink_iter<_CharT>
      out() noexcept
      { return _Sink_iter<_CharT>(*this); }
    };

  // A sink with an internal buffer. This is used to implement concrete sinks.
  template<typename _CharT>
    class _Buf_sink : public _Sink<_CharT>
    {
    protected:
      _CharT _M_buf[32 * sizeof(void*) / sizeof(_CharT)];

      [[__gnu__::__always_inline__]]
      constexpr
      _Buf_sink() noexcept
      : _Sink<_CharT>(_M_buf)
      { }
    };

  // A sink that fills a sequence (e.g. std::string, std::vector, std::deque).
  // Writes to a buffer then appends that to the sequence when it fills up.
  template<typename _Seq>
    class _Seq_sink : public _Buf_sink<typename _Seq::value_type>
    {
      using _CharT = typename _Seq::value_type;

      _Seq _M_seq;

      // Transfer buffer contents to the sequence, so buffer can be refilled.
      void
      _M_overflow() override
      {
	auto __s = this->_M_used();
	if constexpr (__is_specialization_of<_Seq, basic_string>)
	  _M_seq.append(__s.data(), __s.size());
	else
	  _M_seq.insert(_M_seq.end(), __s.begin(), __s.end());
	this->_M_rewind();
      }

    public:
      [[__gnu__::__always_inline__]]
      _Seq_sink() noexcept(is_nothrow_default_constructible_v<_Seq>)
      { }

      _Seq_sink(_Seq&& __s) noexcept(is_nothrow_move_constructible_v<_Seq>)
      : _M_seq(std::move(__s))
      { }

      using _Sink<_CharT>::out;

      _Seq
      get() &&
      {
	_Seq_sink::_M_overflow();
	return std::move(_M_seq);
      }
    };

  template<typename _CharT, typename _Alloc = allocator<_CharT>>
    using _Str_sink
      = _Seq_sink<basic_string<_CharT, char_traits<_CharT>, _Alloc>>;

  // template<typename _CharT, typename _Alloc = allocator<_CharT>>
    // using _Vec_sink = _Seq_sink<vector<_CharT, _Alloc>>;

  // A sink that writes to an output iterator.
  // Writes to a fixed-size buffer and then flushes to the output iterator
  // when the buffer fills up.
  template<typename _CharT, typename _OutIter>
    class _Iter_sink : public _Buf_sink<_CharT>
    {
      _OutIter _M_out;
      iter_difference_t<_OutIter> _M_max;

    protected:
      size_t _M_count = 0;

      void
      _M_overflow() override
      {
	auto __s = this->_M_used();
	if (_M_max < 0) // No maximum.
	  _M_out = ranges::copy(__s, std::move(_M_out)).out;
	else if (_M_count < static_cast<size_t>(_M_max))
	  {
	    auto __max = _M_max - _M_count;
	    span<_CharT> __first;
	    if (__max < __s.size())
	      __first = __s.first(static_cast<size_t>(__max));
	    else
	      __first = __s;
	    _M_out = ranges::copy(__first, std::move(_M_out)).out;
	  }
	this->_M_rewind();
	_M_count += __s.size();
      }

    public:
      [[__gnu__::__always_inline__]]
      explicit
      _Iter_sink(_OutIter __out, iter_difference_t<_OutIter> __max = -1)
      : _M_out(std::move(__out)), _M_max(__max)
      { }

      using _Sink<_CharT>::out;

      format_to_n_result<_OutIter>
      _M_finish() &&
      {
	_Iter_sink::_M_overflow();
	iter_difference_t<_OutIter> __count(_M_count);
	return { std::move(_M_out), __count };
      }
    };

  // Partial specialization for contiguous iterators.
  // No buffer is used, characters are written straight to the iterator.
  // We do not know the size of the output range, so the span size just grows
  // as needed. The end of the span might be an invalid pointer outside the
  // valid range, but we never actually call _M_span.end(). This class does
  // not introduce any invalid pointer arithmetic or overflows that would not
  // have happened anyway.
  template<typename _CharT, contiguous_iterator _OutIter>
    class _Iter_sink<_CharT, _OutIter> : public _Sink<_CharT>
    {
      using uint64_t = __UINTPTR_TYPE__;
      _OutIter _M_first;
      iter_difference_t<_OutIter> _M_max = -1;
    protected:
      size_t _M_count = 0;
    private:
      _CharT _M_buf[64]; // Write here after outputting _M_max characters.

    protected:
      void
      _M_overflow()
      {
	auto __s = this->_M_used();
	_M_count += __s.size();

	if (_M_max >= 0)
	  {
	    // Span was already sized for the maximum character count,
	    // if it overflows then any further output must go to the
	    // internal buffer, to be discarded.
	    span<_CharT> __buf{_M_buf};
	    this->_M_reset(__buf, __buf.begin());
	  }
	else
	  {
	    // No maximum character count. Just extend the span to allow
	    // writing more characters to it.
	    this->_M_reset({__s.data(), __s.size() + 1024}, __s.end());
	  }
      }

    private:
      static span<_CharT>
      _S_make_span(_CharT* __ptr, iter_difference_t<_OutIter> __n,
		   span<_CharT> __buf) noexcept
      {
	if (__n == 0)
	  return __buf; // Only write to the internal buffer.

	if (__n > 0)
	  {
	    if constexpr (!is_integral_v<iter_difference_t<_OutIter>>
			    || sizeof(__n) > sizeof(size_t))
	      {
		// __int128 or __detail::__max_diff_type
		auto __m = iter_difference_t<_OutIter>((size_t)-1);
		if (__n > __m)
		  __n = __m;
	      }
	    return {__ptr, (size_t)__n};
	  }

#if __has_builtin(__builtin_dynamic_object_size)
	if (size_t __bytes = __builtin_dynamic_object_size(__ptr, 2))
	  return {__ptr, __bytes / sizeof(_CharT)};
#endif
	// Avoid forming a pointer to a different memory page.
	uint64_t __off = reinterpret_cast<uint64_t>(__ptr) % 1024;
	__n = (1024 - __off) / sizeof(_CharT);
	if (__n > 0) [[likely]]
	return {__ptr, static_cast<size_t>(__n)};
	else // Misaligned/packed buffer of wchar_t?
	  return {__ptr, 1};
      }

    public:
      explicit
      _Iter_sink(_OutIter __out, iter_difference_t<_OutIter> __n = -1) noexcept
      : _Sink<_CharT>(_S_make_span(std::to_address(__out), __n, _M_buf)),
	_M_first(__out), _M_max(__n)
      { }

      format_to_n_result<_OutIter>
      _M_finish() &&
      {
	_Iter_sink::_M_overflow();
	iter_difference_t<_OutIter> __count(_M_count);
	auto __s = this->_M_used();
	auto __last = _M_first;
	if (__s.data() == _M_buf) // Wrote at least _M_max characters.
	  __last += _M_max;
	else
	  __last += iter_difference_t<_OutIter>(__s.size());
	return { __last, __count };
      }
    };

  enum _Arg_t : unsigned char {
    _Arg_none, _Arg_bool, _Arg_c, _Arg_i, _Arg_u, _Arg_ll, _Arg_ull,
    _Arg_flt, _Arg_dbl, _Arg_ldbl, _Arg_str, _Arg_sv, _Arg_ptr, _Arg_handle,
    _Arg_i128, _Arg_u128,
    _Arg_bf16, _Arg_f16, _Arg_f32, _Arg_f64,
#ifdef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
    _Arg_next_value_,
    _Arg_f128 = _Arg_ldbl,
    _Arg_ibm128 = _Arg_next_value_,
#else
    _Arg_f128,
#endif
    _Arg_max_
  };

  template<typename _Context>
    struct _Arg_value
    {
      using _CharT = typename _Context::char_type;

      struct _HandleBase
      {
	const void* _M_ptr;
	void (*_M_func)();
      };

      union
      {
	monostate _M_none;
	bool _M_bool;
	_CharT _M_c;
	int _M_i;
	unsigned _M_u;
	long long _M_ll;
	unsigned long long _M_ull;
	float _M_flt;
	double _M_dbl;
#ifndef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT // No long double if it's ambiguous.
	long double _M_ldbl;
#endif
	const _CharT* _M_str;
	basic_string_view<_CharT> _M_sv;
	const void* _M_ptr;
	_HandleBase _M_handle;
#ifdef __SIZEOF_INT128__
	__int128 _M_i128;
	unsigned __int128 _M_u128;
#endif
	// TODO _Float16 etc.
#ifdef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
	__ieee128 _M_f128;
	__ibm128  _M_ibm128;
#elif CXX20_FORMAT_FORMAT_F128 == 2
	__float128_t _M_f128;
#endif
      };

      [[__gnu__::__always_inline__]]
      _Arg_value() : _M_none() { }

#if 0
      template<typename _Tp>
	_Arg_value(in_place_type_t<_Tp>, _Tp __val)
	{ _S_get<_Tp>() = __val; }
#endif

      template<typename _Tp, typename _Self>
	[[__gnu__::__always_inline__]]
	static auto&
	_S_get(_Self& __u) noexcept
	{
	  if constexpr (is_same_v<_Tp, bool>)
	    return __u._M_bool;
	  else if constexpr (is_same_v<_Tp, _CharT>)
	    return __u._M_c;
	  else if constexpr (is_same_v<_Tp, int>)
	    return __u._M_i;
	  else if constexpr (is_same_v<_Tp, unsigned>)
	    return __u._M_u;
	  else if constexpr (is_same_v<_Tp, long long>)
	    return __u._M_ll;
	  else if constexpr (is_same_v<_Tp, unsigned long long>)
	    return __u._M_ull;
	  else if constexpr (is_same_v<_Tp, float>)
	    return __u._M_flt;
	  else if constexpr (is_same_v<_Tp, double>)
	    return __u._M_dbl;
#ifndef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
	  else if constexpr (is_same_v<_Tp, long double>)
	    return __u._M_ldbl;
#else
	  else if constexpr (is_same_v<_Tp, __ieee128>)
	    return __u._M_f128;
	  else if constexpr (is_same_v<_Tp, __ibm128>)
	    return __u._M_ibm128;
#endif
	  else if constexpr (is_same_v<_Tp, const _CharT*>)
	    return __u._M_str;
	  else if constexpr (is_same_v<_Tp, basic_string_view<_CharT>>)
	    return __u._M_sv;
	  else if constexpr (is_same_v<_Tp, const void*>)
	    return __u._M_ptr;
#ifdef __SIZEOF_INT128__
	  else if constexpr (is_same_v<_Tp, __int128>)
	    return __u._M_i128;
	  else if constexpr (is_same_v<_Tp, unsigned __int128>)
	    return __u._M_u128;
#endif
#if CXX20_FORMAT_FORMAT_F128 == 2
	  else if constexpr (is_same_v<_Tp, __float128_t>)
	    return __u._M_f128;
#endif
	  else if constexpr (derived_from<_Tp, _HandleBase>)
	    return static_cast<_Tp&>(__u._M_handle);
	  // Otherwise, ill-formed.
	}

      template<typename _Tp>
	[[__gnu__::__always_inline__]]
	auto&
	_M_get() noexcept
	{ return _S_get<_Tp>(*this); }

      template<typename _Tp>
	[[__gnu__::__always_inline__]]
	const auto&
	_M_get() const noexcept
	{ return _S_get<_Tp>(*this); }

      template<typename _Tp>
	[[__gnu__::__always_inline__]]
	void
	_M_set(_Tp __v) noexcept
	{
	  if constexpr (derived_from<_Tp, _HandleBase>)
	    std::construct_at(&_M_handle, __v);
	  else
	    _S_get<_Tp>(*this) = __v;
	}
      };

  // [format.arg.store], class template format-arg-store
  template<typename _Context, typename... _Args>
    class _Arg_store;

} // namespace __format
/// @endcond

  template<typename _Context>
    class basic_format_arg
    {
      using _CharT = typename _Context::char_type;

      template<typename _Tp>
	static constexpr bool __formattable
	  = CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_with<_Tp, _Context>;

    public:
      class handle : public CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context>::_HandleBase
      {
	using _Base = typename CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context>::_HandleBase;

	// Format as const if possible, to reduce instantiations.
	template<typename _Tp>
	  using __maybe_const_t
	    = __conditional_t<CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_with<_Tp, _Context>,
			      const _Tp, _Tp>;

	template<typename _Tq>
	  static void
	  _S_format(basic_format_parse_context<_CharT>& __parse_ctx,
		    _Context& __format_ctx, const void* __ptr)
	  {
	    using _Td = remove_const_t<_Tq>;
	    typename _Context::template formatter_type<_Td> __f;
	    __parse_ctx.advance_to(__f.parse(__parse_ctx));
	    _Tq& __val = *const_cast<_Tq*>(static_cast<const _Td*>(__ptr));
	    __format_ctx.advance_to(__f.format(__val, __format_ctx));
	  }

	template<typename _Tp>
	  explicit
	  handle(_Tp& __val) noexcept
	  {
	    if constexpr (!CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_with<const _Tp, _Context>)
	      static_assert(!is_const_v<_Tp>, "std::format argument must be "
					      "non-const for this type");

	    this->_M_ptr = __builtin_addressof(__val);
	    auto __func = _S_format<__maybe_const_t<_Tp>>;
	    this->_M_func = reinterpret_cast<void(*)()>(__func);
	  }

	friend class basic_format_arg<_Context>;

      public:
	handle(const handle&) = default;
	handle& operator=(const handle&) = default;

	[[__gnu__::__always_inline__]]
	void
	format(basic_format_parse_context<_CharT>& __pc, _Context& __fc) const
	{
	  using _Func = void(*)(basic_format_parse_context<_CharT>&,
				_Context&, const void*);
	  auto __f = reinterpret_cast<_Func>(this->_M_func);
	  __f(__pc, __fc, this->_M_ptr);
	}
      };

      [[__gnu__::__always_inline__]]
      basic_format_arg() noexcept : _M_type(CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_none) { }

      [[nodiscard,__gnu__::__always_inline__]]
      explicit operator bool() const noexcept
      { return _M_type != CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_none; }

    private:
      template<typename _Ctx>
	friend class basic_format_args;

      template<typename _Ctx, typename... _Args>
	friend class CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store;

      static_assert(is_trivially_copyable_v<CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context>>);

      CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context> _M_val;
      CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t _M_type;

      // Transform incoming argument type to the type stored in _Arg_value.
      // e.g. short -> int, std::string -> std::string_view,
      // char[3] -> const char*.
      template<typename _Tp>
	static consteval auto
	_S_to_arg_type()
	{
	  using _Td = remove_const_t<_Tp>;
	  if constexpr (is_same_v<_Td, bool>)
	    return type_identity<bool>();
	  else if constexpr (is_same_v<_Td, _CharT>)
	    return type_identity<_CharT>();
	  else if constexpr (is_same_v<_Td, char> && is_same_v<_CharT, wchar_t>)
	    return type_identity<_CharT>();
#ifdef __SIZEOF_INT128__ // Check before signed/unsigned integer
	  else if constexpr (is_same_v<_Td, __int128>)
	    return type_identity<__int128>();
	  else if constexpr (is_same_v<_Td, unsigned __int128>)
	    return type_identity<unsigned __int128>();
#endif
	  else if constexpr (__is_signed_integer<_Td>::value)
	    {
	      if constexpr (sizeof(_Td) <= sizeof(int))
		return type_identity<int>();
	      else if constexpr (sizeof(_Td) <= sizeof(long long))
		return type_identity<long long>();
	    }
	  else if constexpr (__is_unsigned_integer<_Td>::value)
	    {
	      if constexpr (sizeof(_Td) <= sizeof(unsigned))
		return type_identity<unsigned>();
	      else if constexpr (sizeof(_Td) <= sizeof(unsigned long long))
		return type_identity<unsigned long long>();
	    }
	  else if constexpr (is_same_v<_Td, float>)
	    return type_identity<float>();
	  else if constexpr (is_same_v<_Td, double>)
	    return type_identity<double>();
#ifndef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
	  else if constexpr (is_same_v<_Td, long double>)
	    return type_identity<long double>();
#else
	  else if constexpr (is_same_v<_Td, __ibm128>)
	    return type_identity<__ibm128>();
	  else if constexpr (is_same_v<_Td, __ieee128>)
	    return type_identity<__ieee128>();
#endif

	  // TODO bfloat16 and float16

#ifdef __FLT32_DIG__
	  else if constexpr (is_same_v<_Td, _Float32>)
# ifdef _GLIBCXX_FLOAT_IS_IEEE_BINARY32
	    return type_identity<float>();
# else
	    return type_identity<_Float32>();
# endif
#endif
#ifdef __FLT64_DIG__
	  else if constexpr (is_same_v<_Td, _Float64>)
# ifdef _GLIBCXX_DOUBLE_IS_IEEE_BINARY64
	    return type_identity<double>();
# else
	    return type_identity<_Float64>();
# endif
#endif
#if CXX20_FORMAT_FORMAT_F128
# if __FLT128_DIG__
	  else if constexpr (is_same_v<_Td, _Float128>)
	    return type_identity<CXX20_FORMAT_DECORATE_NAME(__format)::__float128_t>();
# endif
# if __SIZEOF_FLOAT128__
	  else if constexpr (is_same_v<_Td, __float128>)
	    return type_identity<CXX20_FORMAT_DECORATE_NAME(__format)::__float128_t>();
# endif
#endif
	  else if constexpr (__is_specialization_of<_Td, basic_string_view>)
	    return type_identity<basic_string_view<_CharT>>();
	  else if constexpr (__is_specialization_of<_Td, basic_string>)
	    return type_identity<basic_string_view<_CharT>>();
	  else if constexpr (is_same_v<decay_t<_Td>, const _CharT*>)
	    return type_identity<const _CharT*>();
	  else if constexpr (is_same_v<decay_t<_Td>, _CharT*>)
	    return type_identity<const _CharT*>();
	  else if constexpr (is_void_v<remove_pointer_t<_Td>>)
	    return type_identity<const void*>();
	  else if constexpr (is_same_v<_Td, nullptr_t>)
	    return type_identity<const void*>();
	  else
	    return type_identity<handle>();
	}

      // Transform a formattable type to the appropriate storage type.
      template<typename _Tp>
	using _Normalize = typename decltype(_S_to_arg_type<_Tp>())::type;

      // Get the _Arg_t value corresponding to a normalized type.
      template<typename _Tp>
	static consteval CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t
	_S_to_enum()
	{
	  using namespace CXX20_FORMAT_DECORATE_NAME(__format);
	  if constexpr (is_same_v<_Tp, bool>)
	    return _Arg_bool;
	  else if constexpr (is_same_v<_Tp, _CharT>)
	    return _Arg_c;
	  else if constexpr (is_same_v<_Tp, int>)
	    return _Arg_i;
	  else if constexpr (is_same_v<_Tp, unsigned>)
	    return _Arg_u;
	  else if constexpr (is_same_v<_Tp, long long>)
	    return _Arg_ll;
	  else if constexpr (is_same_v<_Tp, unsigned long long>)
	    return _Arg_ull;
	  else if constexpr (is_same_v<_Tp, float>)
	    return _Arg_flt;
	  else if constexpr (is_same_v<_Tp, double>)
	    return _Arg_dbl;
#ifndef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
	  else if constexpr (is_same_v<_Tp, long double>)
	    return _Arg_ldbl;
#else
	  // Don't use _Arg_ldbl for this target, it's ambiguous.
	  else if constexpr (is_same_v<_Tp, __ibm128>)
	    return _Arg_ibm128;
	  else if constexpr (is_same_v<_Tp, __ieee128>)
	    return _Arg_f128;
#endif
	  else if constexpr (is_same_v<_Tp, const _CharT*>)
	    return _Arg_str;
	  else if constexpr (is_same_v<_Tp, basic_string_view<_CharT>>)
	    return _Arg_sv;
	  else if constexpr (is_same_v<_Tp, const void*>)
	    return _Arg_ptr;
#ifdef __SIZEOF_INT128__
	  else if constexpr (is_same_v<_Tp, __int128>)
	    return _Arg_i128;
	  else if constexpr (is_same_v<_Tp, unsigned __int128>)
	    return _Arg_u128;
#endif

	  // N.B. some of these types will never actually be used here,
	  // because they get normalized to a standard floating-point type.
#if defined __FLT32_DIG__ && ! _GLIBCXX_FLOAT_IS_IEEE_BINARY32
	  else if constexpr (is_same_v<_Tp, _Float32>)
	    return _Arg_f32;
#endif
#if defined __FLT64_DIG__ && ! _GLIBCXX_DOUBLE_IS_IEEE_BINARY64
	  else if constexpr (is_same_v<_Tp, _Float64>)
	    return _Arg_f64;
#endif
#if CXX20_FORMAT_FORMAT_F128 == 2
	  else if constexpr (is_same_v<_Tp, CXX20_FORMAT_DECORATE_NAME(__format)::__float128_t>)
	    return _Arg_f128;
#endif
	  else if constexpr (is_same_v<_Tp, handle>)
	    return _Arg_handle;
	}

      template<typename _Tp>
	void
	_M_set(_Tp __v) noexcept
	{
	  _M_type = _S_to_enum<_Tp>();
	  _M_val._M_set(__v);
	}

      template<typename _Tp>
	requires CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_with<_Tp, _Context>
	explicit
	basic_format_arg(_Tp& __v) noexcept
	{
	  using _Td = _Normalize<_Tp>;
	  if constexpr (is_same_v<_Td, basic_string_view<_CharT>>)
	    _M_set(_Td{__v.data(), __v.size()});
	  else
	    _M_set(static_cast<_Td>(__v));
	}

      template<typename _Ctx, typename... _Argz>
	friend auto
	make_format_args(_Argz&&...) noexcept;

      template<typename _Visitor, typename _Ctx>
	friend decltype(auto)
	visit_format_arg(_Visitor&& __vis, basic_format_arg<_Ctx>);

      template<typename _Visitor>
	decltype(auto)
	_M_visit(_Visitor&& __vis, CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t __type)
	{
	  using namespace CXX20_FORMAT_DECORATE_NAME(__format);
	  switch (__type)
	  {
	    case _Arg_none:
	      return std::forward<_Visitor>(__vis)(_M_val._M_none);
	    case _Arg_bool:
	      return std::forward<_Visitor>(__vis)(_M_val._M_bool);
	    case _Arg_c:
	      return std::forward<_Visitor>(__vis)(_M_val._M_c);
	    case _Arg_i:
	      return std::forward<_Visitor>(__vis)(_M_val._M_i);
	    case _Arg_u:
	      return std::forward<_Visitor>(__vis)(_M_val._M_u);
	    case _Arg_ll:
	      return std::forward<_Visitor>(__vis)(_M_val._M_ll);
	    case _Arg_ull:
	      return std::forward<_Visitor>(__vis)(_M_val._M_ull);
#if __cpp_lib_to_chars // FIXME: need to be able to format these types!
	    case _Arg_flt:
	      return std::forward<_Visitor>(__vis)(_M_val._M_flt);
	    case _Arg_dbl:
	      return std::forward<_Visitor>(__vis)(_M_val._M_dbl);
#ifndef _GLIBCXX_LONG_DOUBLE_ALT128_COMPAT
	    case _Arg_ldbl:
	      return std::forward<_Visitor>(__vis)(_M_val._M_ldbl);
#else
	    case _Arg_f128:
	      return std::forward<_Visitor>(__vis)(_M_val._M_f128);
	    case _Arg_ibm128:
	      return std::forward<_Visitor>(__vis)(_M_val._M_ibm128);
#endif
#endif
	    case _Arg_str:
	      return std::forward<_Visitor>(__vis)(_M_val._M_str);
	    case _Arg_sv:
	      return std::forward<_Visitor>(__vis)(_M_val._M_sv);
	    case _Arg_ptr:
	      return std::forward<_Visitor>(__vis)(_M_val._M_ptr);
	    case _Arg_handle:
	    {
	      auto& __h = static_cast<handle&>(_M_val._M_handle);
	      return std::forward<_Visitor>(__vis)(__h);
	    }
#ifdef __SIZEOF_INT128__
	    case _Arg_i128:
	      return std::forward<_Visitor>(__vis)(_M_val._M_i128);
	    case _Arg_u128:
	      return std::forward<_Visitor>(__vis)(_M_val._M_u128);
#endif
	      // TODO _Arg_f16 etc.

#if CXX20_FORMAT_FORMAT_F128 == 2
	    case _Arg_f128:
	      return std::forward<_Visitor>(__vis)(_M_val._M_f128);
#endif
	  }
	  __builtin_unreachable();
	}
    };

  template<typename _Visitor, typename _Context>
    inline decltype(auto)
    visit_format_arg(_Visitor&& __vis, basic_format_arg<_Context> __arg)
    {
      return __arg._M_visit(std::forward<_Visitor>(__vis), __arg._M_type);
    }

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  struct _WidthPrecVisitor
  {
    template<typename _Tp>
      size_t
      operator()(_Tp& __arg) const
      {
	if constexpr (is_same_v<_Tp, monostate>)
	  CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
	// _GLIBCXX_RESOLVE_LIB_DEFECTS
	// 3720. Restrict the valid types of arg-id for width and precision
	// 3721. Allow an arg-id with a value of zero for width
	else if constexpr (sizeof(_Tp) <= sizeof(long long))
	  {
	    // _GLIBCXX_RESOLVE_LIB_DEFECTS
	    // 3720. Restrict the valid types of arg-id for width and precision
	    if constexpr (__is_unsigned_integer<_Tp>::value)
	      return __arg;
	    else if constexpr (__is_signed_integer<_Tp>::value)
	      if (__arg >= 0)
		return __arg;
	  }
	__throw_format_error("format error: argument used for width or "
			     "precision must be a non-negative integer");
      }
  };

  template<typename _Context>
    inline size_t
    __int_from_arg(const basic_format_arg<_Context>& __arg)
    { return std::visit_format_arg(_WidthPrecVisitor(), __arg); }

  // Pack _Arg_t enum values into a single 60-bit integer.
  template<int _Bits, size_t _Nm>
    constexpr auto
    __pack_arg_types(const array<_Arg_t, _Nm>& __types)
    {
      __UINT64_TYPE__ __packed_types = 0;
      for (auto __i = __types.rbegin(); __i != __types.rend(); ++__i)
	__packed_types = (__packed_types << _Bits) | *__i;
      return __packed_types;
    }
} // namespace __format
/// @endcond

  template<typename _Context>
    class basic_format_args
    {
      static constexpr int _S_packed_type_bits = 5; // _Arg_t values [0,20]
      static constexpr int _S_packed_type_mask = 0b11111;
      static constexpr int _S_max_packed_args = 12;

      static_assert( CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_max_ <= (1 << _S_packed_type_bits) );

      template<typename... _Args>
	using _Store = CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store<_Context, _Args...>;

      template<typename _Ctx, typename... _Args>
	friend class CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store;

      using uint64_t = __UINT64_TYPE__;
      using _Format_arg = basic_format_arg<_Context>;
      using _Format_arg_val = CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context>;

      // If args are packed then the number of args is in _M_packed_size and
      // the packed types are in _M_unpacked_size, accessed via _M_type(i).
      // If args are not packed then the number of args is in _M_unpacked_size
      // and _M_packed_size is zero.
      uint64_t _M_packed_size : 4;
      uint64_t _M_unpacked_size : 60;

      union {
	const _Format_arg_val* _M_values; // Active when _M_packed_size != 0
	const _Format_arg* _M_args;       // Active when _M_packed_size == 0
      };

      size_t
      _M_size() const noexcept
      { return _M_packed_size ? _M_packed_size : _M_unpacked_size; }

      typename CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t
      _M_type(size_t __i) const noexcept
      {
	uint64_t __t = _M_unpacked_size >> (__i * _S_packed_type_bits);
	return static_cast<CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t>(__t & _S_packed_type_mask);
      }

      template<typename _Ctx, typename... _Args>
	friend auto
	make_format_args(_Args&&...) noexcept;

      // An array of _Arg_t enums corresponding to _Args...
      template<typename... _Args>
	static consteval array<CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_t, sizeof...(_Args)>
	_S_types_to_pack()
	{ return {_Format_arg::template _S_to_enum<_Args>()...}; }

    public:
      basic_format_args() noexcept = default;

      template<typename... _Args>
	basic_format_args(const _Store<_Args...>& __store) noexcept;

      [[nodiscard,__gnu__::__always_inline__]]
      basic_format_arg<_Context>
      get(size_t __i) const noexcept
      {
	basic_format_arg<_Context> __arg;
	if (__i < _M_packed_size)
	  {
	    __arg._M_type = _M_type(__i);
	    __arg._M_val = _M_values[__i];
	  }
	else if (_M_packed_size == 0 && __i < _M_unpacked_size)
	  __arg = _M_args[__i];
	return __arg;
      }
    };

  // _GLIBCXX_RESOLVE_LIB_DEFECTS
  // 3810. CTAD for std::basic_format_args
  template<typename _Context, typename... _Args>
    basic_format_args(CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store<_Context, _Args...>)
      -> basic_format_args<_Context>;

  template<typename _Context, typename... _Args>
    auto
    make_format_args(_Args&&... __fmt_args) noexcept;

  // An array of type-erased formatting arguments.
  template<typename _Context, typename... _Args>
    class CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store
    {
      friend std::basic_format_args<_Context>;

      template<typename _Ctx, typename... _Argz>
	friend auto
	std::make_format_args(_Argz&&...) noexcept;

      // For a sufficiently small number of arguments we only store values.
      // basic_format_args can get the types from the _Args pack.
      static constexpr bool _S_values_only
	= sizeof...(_Args) <= basic_format_args<_Context>::_S_max_packed_args;

      using _Element_t
	= __conditional_t<_S_values_only,
			  CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_value<_Context>,
			  basic_format_arg<_Context>>;

      _Element_t _M_args[sizeof...(_Args)];

      template<typename _Tp>
	static _Element_t
	_S_make_elt(_Tp& __v)
	{
	  basic_format_arg<_Context> __arg(__v);
	  if constexpr (_S_values_only)
	    return __arg._M_val;
	  else
	    return __arg;
	}

      template<typename... _Tp>
	requires (sizeof...(_Tp) == sizeof...(_Args))
	[[__gnu__::__always_inline__]]
	_Arg_store(_Tp&... __a) noexcept
	: _M_args{_S_make_elt(__a)...}
	{ }
    };

  template<typename _Context>
    class CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store<_Context>
    { };

  template<typename _Context>
    template<typename... _Args>
      basic_format_args<_Context>::
      basic_format_args(const _Store<_Args...>& __store) noexcept
      {
	if constexpr (sizeof...(_Args) == 0)
	  {
	    _M_packed_size = 0;
	    _M_unpacked_size = 0;
	    _M_args = nullptr;
	  }
	else if constexpr (sizeof...(_Args) <= _S_max_packed_args)
	  {
	    // The number of packed arguments:
	    _M_packed_size = sizeof...(_Args);
	    // The packed type enums:
	    _M_unpacked_size
	      = CXX20_FORMAT_DECORATE_NAME(__format)::__pack_arg_types<_S_packed_type_bits>(_S_types_to_pack<_Args...>());
	    // The _Arg_value objects.
	    _M_values = __store._M_args;
	  }
	else
	  {
	    // No packed arguments:
	    _M_packed_size = 0;
	    // The number of unpacked arguments:
	    _M_unpacked_size = sizeof...(_Args);
	    // The basic_format_arg objects:
	    _M_args = __store._M_args;
	  }
      }

  /// Capture formatting arguments for use by `std::vformat`.
  template<typename _Context = format_context, typename... _Args>
    [[nodiscard,__gnu__::__always_inline__]]
    inline auto
    make_format_args(_Args&&... __fmt_args) noexcept
    {
      using _Fmt_arg = basic_format_arg<_Context>;
      using _Store = CXX20_FORMAT_DECORATE_NAME(__format)::_Arg_store<_Context, typename _Fmt_arg::template
		     _Normalize<remove_reference_t<_Args>>...>;
      return _Store(__fmt_args...);
    }

  /// Capture formatting arguments for use by `std::vformat` (for wide output).
  template<typename... _Args>
    [[nodiscard,__gnu__::__always_inline__]]
    inline auto
    make_wformat_args(_Args&&... __args) noexcept
    { return std::make_format_args<wformat_context>(__args...); }

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  template<typename _Out, typename _CharT, typename _Context>
    _Out
    __do_vformat_to(_Out, basic_string_view<_CharT>,
		    const basic_format_args<_Context>&,
		    const locale* = nullptr);
} // namespace __format
/// @endcond

  /** Context for std::format and similar functions.
   *
   * A formatting context contains an output iterator and locale to use
   * for the formatting operations. Most programs will never need to use
   * this class template explicitly. For typical uses of `std::format` the
   * library will use the specializations `std::format_context` (for `char`)
   * and `std::wformat_context` (for `wchar_t`).
   */
  template<typename _Out, typename _CharT>
    class basic_format_context
    {
      static_assert( output_iterator<_Out, const _CharT&> );

      basic_format_args<basic_format_context> _M_args;
      _Out _M_out;
      CXX20_FORMAT_DECORATE_NAME(__format)::_Optional_locale _M_loc;

      basic_format_context(basic_format_args<basic_format_context> __args,
			   _Out __out)
      : _M_args(__args), _M_out(std::move(__out))
      { }

      basic_format_context(basic_format_args<basic_format_context> __args,
			   _Out __out, const std::locale& __loc)
      : _M_args(__args), _M_out(std::move(__out)), _M_loc(__loc)
      { }

      template<typename _Out2, typename _CharT2, typename _Context2>
	friend _Out2
	CXX20_FORMAT_DECORATE_NAME(__format)::__do_vformat_to(_Out2, basic_string_view<_CharT2>,
				  const basic_format_args<_Context2>&,
				  const locale*);

    public:
      basic_format_context() = default;
      ~basic_format_context() = default;

      using iterator = _Out;
      using char_type = _CharT;
      template<typename _Tp>
	using formatter_type = formatter<_Tp, _CharT>;

      [[nodiscard]]
      basic_format_arg<basic_format_context>
      arg(size_t __id) const noexcept
      { return _M_args.get(__id); }

      [[nodiscard]]
      std::locale locale() { return _M_loc.value(); }

      [[nodiscard]]
      iterator out() { return std::move(_M_out); }

      void advance_to(iterator __it) { _M_out = std::move(__it); }
    };


/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  template<typename _Ctx, typename _CharT>
    [[__gnu__::__always_inline__]]
    inline void
    __write(_Ctx& __ctx, basic_string_view<_CharT> __str)
    requires requires { { __ctx.out() } -> output_iterator<const _CharT&>; }
    {
      __ctx.advance_to(CXX20_FORMAT_DECORATE_NAME(__format)::__write(__ctx.out()));
    }

  // Abstract base class defining an interface for scanning format strings.
  // Scan the characters in a format string, dividing it up into strings of
  // ordinary characters, escape sequences, and replacement fields.
  // Call virtual functions for derived classes to parse format-specifiers
  // or write formatted output.
  template<typename _CharT>
    struct _Scanner
    {
      using iterator = typename basic_format_parse_context<_CharT>::iterator;

      basic_format_parse_context<_CharT> _M_pc;

      constexpr explicit
      _Scanner(basic_string_view<_CharT> __str, size_t __nargs = -1)
      : _M_pc(__str, __nargs)
      { }

      constexpr iterator begin() const noexcept { return _M_pc.begin(); }
      constexpr iterator end() const noexcept { return _M_pc.end(); }

      constexpr void
      _M_scan()
      {
	basic_string_view<_CharT> __fmt = _M_fmt_str();

	if (__fmt.size() == 2 && __fmt[0] == '{' && __fmt[1] == '}')
	  {
	    _M_pc.advance_to(begin() + 1);
	    _M_format_arg(_M_pc.next_arg_id());
	    return;
	  }

	size_t __lbr = __fmt.find('{');
	size_t __rbr = __fmt.find('}');

	while (__fmt.size())
	  {
	    auto __cmp = __lbr <=> __rbr;
	    if (__cmp == 0)
	      {
		_M_on_chars(end());
		_M_pc.advance_to(end());
		return;
	      }
	    else if (__cmp < 0)
	      {
		if (__lbr + 1 == __fmt.size()
		      || (__rbr == __fmt.npos && __fmt[__lbr + 1] != '{'))
		  CXX20_FORMAT_DECORATE_NAME(__format)::__unmatched_left_brace_in_format_string();
		const bool __is_escape = __fmt[__lbr + 1] == '{';
		iterator __last = begin() + __lbr + int(__is_escape);
		_M_on_chars(__last);
		_M_pc.advance_to(__last + 1);
		__fmt = _M_fmt_str();
		if (__is_escape)
		  {
		    if (__rbr != __fmt.npos)
		      __rbr -= __lbr + 2;
		    __lbr = __fmt.find('{');
		  }
		else
		  {
		    _M_on_replacement_field();
		    __fmt = _M_fmt_str();
		    __lbr = __fmt.find('{');
		    __rbr = __fmt.find('}');
		  }
	      }
	    else
	      {
		if (++__rbr == __fmt.size() || __fmt[__rbr] != '}')
		  CXX20_FORMAT_DECORATE_NAME(__format)::__unmatched_right_brace_in_format_string();
		iterator __last = begin() + __rbr;
		_M_on_chars(__last);
		_M_pc.advance_to(__last + 1);
		__fmt = _M_fmt_str();
		if (__lbr != __fmt.npos)
		  __lbr -= __rbr + 1;
		__rbr = __fmt.find('}');
	      }
	  }
      }

      constexpr basic_string_view<_CharT>
      _M_fmt_str() const noexcept
      { return {begin(), end()}; }

      constexpr virtual void _M_on_chars(iterator) { }

      constexpr void _M_on_replacement_field()
      {
	auto __next = begin();

	size_t __id;
	if (*__next == '}')
	  __id = _M_pc.next_arg_id();
	else if (*__next == ':')
	  {
	    __id = _M_pc.next_arg_id();
	    _M_pc.advance_to(++__next);
	  }
	else
	  {
	    auto [__i, __ptr] = CXX20_FORMAT_DECORATE_NAME(__format)::__parse_arg_id(begin(), end());
	    if (!__ptr || !(*__ptr == '}' || *__ptr == ':'))
	      CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
	    _M_pc.check_arg_id(__id = __i);
	    if (*__ptr == ':')
	      {
		_M_pc.advance_to(++__ptr);
	      }
	    else
	      _M_pc.advance_to(__ptr);
	  }
	_M_format_arg(__id);
	_M_pc.advance_to(_M_pc.begin() + 1); // Move past '}'
      }

      constexpr virtual void _M_format_arg(size_t __id) = 0;
    };

  // Process a format string and format the arguments in the context.
  template<typename _Out, typename _CharT>
    class _Formatting_scanner : public _Scanner<_CharT>
    {
    public:
      _Formatting_scanner(basic_format_context<_Out, _CharT>& __fc,
			  basic_string_view<_CharT> __str)
      : _Scanner<_CharT>(__str), _M_fc(__fc)
      { }

    private:
      basic_format_context<_Out, _CharT>& _M_fc;

      using iterator = typename _Scanner<_CharT>::iterator;

      void
      _M_on_chars(iterator __last) override
      {
	basic_string_view<_CharT> __str(this->begin(), __last);
	_M_fc.advance_to(CXX20_FORMAT_DECORATE_NAME(__format)::__write(_M_fc.out(), __str));
      }

      void
      _M_format_arg(size_t __id) override
      {
	using _Context = basic_format_context<_Out, _CharT>;
	using handle = typename basic_format_arg<_Context>::handle;

	std::visit_format_arg([this](auto& __arg) {
	  using _Type = remove_reference_t<decltype(__arg)>;
	  using _Formatter = typename _Context::template formatter_type<_Type>;
	  if constexpr (is_same_v<_Type, monostate>)
	    CXX20_FORMAT_DECORATE_NAME(__format)::__invalid_arg_id_in_format_string();
	  else if constexpr (is_same_v<_Type, handle>)
	    __arg.format(this->_M_pc, this->_M_fc);
	  else if constexpr (is_default_constructible_v<_Formatter>)
	    {
	      _Formatter __f;
	      this->_M_pc.advance_to(__f.parse(this->_M_pc));
	      this->_M_fc.advance_to(__f.format(__arg, this->_M_fc));
	    }
	  else
	    static_assert(CXX20_FORMAT_DECORATE_NAME(__format)::__formattable_with<_Type, _Context>);
	}, _M_fc.arg(__id));
      }
    };

  // Validate a format string for Args.
  template<typename _CharT, typename... _Args>
    class _Checking_scanner : public _Scanner<_CharT>
    {
      static_assert(
	(is_default_constructible_v<formatter<_Args, _CharT>> && ...),
	"std::formatter must be specialized for each type being formatted");

    public:
      constexpr
      _Checking_scanner(basic_string_view<_CharT> __str)
      : _Scanner<_CharT>(__str, sizeof...(_Args))
      { }

    private:
      constexpr void
      _M_format_arg(size_t __id) override
      {
	if constexpr (sizeof...(_Args) != 0)
	  {
	    if (__id < sizeof...(_Args))
	      {
		_M_parse_format_spec<_Args...>(__id);
		return;
	      }
	  }
	__builtin_unreachable();
      }

      template<typename _Tp, typename... _OtherArgs>
	constexpr void
	_M_parse_format_spec(size_t __id)
	{
	  if (__id == 0)
	    {
	      formatter<_Tp, _CharT> __f;
	      this->_M_pc.advance_to(__f.parse(this->_M_pc));
	    }
	  else if constexpr (sizeof...(_OtherArgs) != 0)
	    _M_parse_format_spec<_OtherArgs...>(__id - 1);
	  else
	    __builtin_unreachable();
	}
    };

  template<typename _Out, typename _CharT, typename _Context>
    inline _Out
    __do_vformat_to(_Out __out, basic_string_view<_CharT> __fmt,
		    const basic_format_args<_Context>& __args,
		    const locale* __loc)
    {
      _Iter_sink<_CharT, _Out> __sink(std::move(__out));
      _Sink_iter<_CharT> __sink_out;

      if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>)
	__sink_out = __out; // Already a sink iterator, safe to use post-move.
      else
	__sink_out = __sink.out();

      auto __ctx = __loc == nullptr
		     ? _Context(__args, __sink_out)
		     : _Context(__args, __sink_out, *__loc);
      _Formatting_scanner<_Sink_iter<_CharT>, _CharT> __scanner(__ctx, __fmt);
      __scanner._M_scan();

      if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>)
	return __ctx.out();
      else
	return std::move(__sink)._M_finish().out;
    }

} // namespace __format
/// @endcond

  template<typename _CharT, typename... _Args>
    template<typename _Tp>
      requires convertible_to<const _Tp&, basic_string_view<_CharT>>
      consteval
      basic_format_string<_CharT, _Args...>::
      basic_format_string(const _Tp& __s)
      : _M_str(__s)
      {
	CXX20_FORMAT_DECORATE_NAME(__format)::_Checking_scanner<_CharT, remove_cvref_t<_Args>...>
	  __scanner(_M_str);
	__scanner._M_scan();
      }

  // [format.functions], formatting functions

  template<typename _Out> requires output_iterator<_Out, const char&>
    [[__gnu__::__always_inline__]]
    inline _Out
    vformat_to(_Out __out, string_view __fmt, format_args __args)
    { return CXX20_FORMAT_DECORATE_NAME(__format)::__do_vformat_to(std::move(__out), __fmt, __args); }

  template<typename _Out> requires output_iterator<_Out, const wchar_t&>
    [[__gnu__::__always_inline__]]
    inline _Out
    vformat_to(_Out __out, wstring_view __fmt, wformat_args __args)
    { return CXX20_FORMAT_DECORATE_NAME(__format)::__do_vformat_to(std::move(__out), __fmt, __args); }

  template<typename _Out> requires output_iterator<_Out, const char&>
    [[__gnu__::__always_inline__]]
    inline _Out
    vformat_to(_Out __out, const locale& __loc, string_view __fmt,
	       format_args __args)
    { return CXX20_FORMAT_DECORATE_NAME(__format)::__do_vformat_to(std::move(__out), __fmt, __args, &__loc); }

  template<typename _Out> requires output_iterator<_Out, const wchar_t&>
    [[__gnu__::__always_inline__]]
    inline _Out
    vformat_to(_Out __out, const locale& __loc, wstring_view __fmt,
	       wformat_args __args)
    { return CXX20_FORMAT_DECORATE_NAME(__format)::__do_vformat_to(std::move(__out), __fmt, __args, &__loc); }

  [[nodiscard]]
  inline string
  vformat(string_view __fmt, format_args __args)
  {
    CXX20_FORMAT_DECORATE_NAME(__format)::_Str_sink<char> __buf;
    std::vformat_to(__buf.out(), __fmt, __args);
    return std::move(__buf).get();
  }

  [[nodiscard]]
  inline wstring
  vformat(wstring_view __fmt, wformat_args __args)
  {
    CXX20_FORMAT_DECORATE_NAME(__format)::_Str_sink<wchar_t> __buf;
    std::vformat_to(__buf.out(), __fmt, __args);
    return std::move(__buf).get();
  }

  [[nodiscard]]
  inline string
  vformat(const locale& __loc, string_view __fmt, format_args __args)
  {
    CXX20_FORMAT_DECORATE_NAME(__format)::_Str_sink<char> __buf;
    std::vformat_to(__buf.out(), __loc, __fmt, __args);
    return std::move(__buf).get();
  }

  [[nodiscard]]
  inline wstring
  vformat(const locale& __loc, wstring_view __fmt, wformat_args __args)
  {
    CXX20_FORMAT_DECORATE_NAME(__format)::_Str_sink<wchar_t> __buf;
    std::vformat_to(__buf.out(), __loc, __fmt, __args);
    return std::move(__buf).get();
  }

  template<typename... _Args>
    [[nodiscard]]
    inline string
    format(format_string<_Args...> __fmt, _Args&&... __args)
    { return std::vformat(__fmt.get(), std::make_format_args(__args...)); }

  template<typename... _Args>
    [[nodiscard]]
    inline wstring
    format(wformat_string<_Args...> __fmt, _Args&&... __args)
    { return std::vformat(__fmt.get(), std::make_wformat_args(__args...)); }

  template<typename... _Args>
    [[nodiscard]]
    inline string
    format(const locale& __loc, format_string<_Args...> __fmt,
	   _Args&&... __args)
    {
      return std::vformat(__loc, __fmt.get(),
			  std::make_format_args(__args...));
    }

  template<typename... _Args>
    [[nodiscard]]
    inline wstring
    format(const locale& __loc, wformat_string<_Args...> __fmt,
	   _Args&&... __args)
    {
      return std::vformat(__loc, __fmt.get(),
			  std::make_wformat_args(__args...));
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const char&>
    inline _Out
    format_to(_Out __out, format_string<_Args...> __fmt, _Args&&... __args)
    {
      return std::vformat_to(std::move(__out), __fmt.get(),
			     std::make_format_args(std::forward<_Args>(__args)...));
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const wchar_t&>
    inline _Out
    format_to(_Out __out, wformat_string<_Args...> __fmt, _Args&&... __args)
    {
      return std::vformat_to(std::move(__out), __fmt.get(),
			     std::make_wformat_args(std::forward<_Args>(__args)...));
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const char&>
    inline _Out
    format_to(_Out __out, const locale& __loc, format_string<_Args...> __fmt,
	      _Args&&... __args)
    {
      return std::vformat_to(std::move(__out), __loc, __fmt.get(),
			     std::make_format_args(std::forward<_Args>(__args)...));
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const wchar_t&>
    inline _Out
    format_to(_Out __out, const locale& __loc, wformat_string<_Args...> __fmt,
	      _Args&&... __args)
    {
      return std::vformat_to(std::move(__out), __loc, __fmt.get(),
			     std::make_wformat_args(std::forward<_Args>(__args)...));
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const char&>
    inline format_to_n_result<_Out>
    format_to_n(_Out __out, iter_difference_t<_Out> __n,
		format_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Iter_sink<char, _Out> __sink(std::move(__out), __n);
      std::vformat_to(__sink.out(), __fmt.get(),
		      std::make_format_args(__args...));
      return std::move(__sink)._M_finish();
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const wchar_t&>
    inline format_to_n_result<_Out>
    format_to_n(_Out __out, iter_difference_t<_Out> __n,
		wformat_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Iter_sink<wchar_t, _Out> __sink(std::move(__out), __n);
      std::vformat_to(__sink.out(), __fmt.get(),
		      std::make_wformat_args(__args...));
      return std::move(__sink)._M_finish();
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const char&>
    inline format_to_n_result<_Out>
    format_to_n(_Out __out, iter_difference_t<_Out> __n, const locale& __loc,
		format_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Iter_sink<char, _Out> __sink(std::move(__out), __n);
      std::vformat_to(__sink.out(), __loc, __fmt.get(),
		      std::make_format_args(__args...));
      return std::move(__sink)._M_finish();
    }

  template<typename _Out, typename... _Args>
    requires output_iterator<_Out, const wchar_t&>
    inline format_to_n_result<_Out>
    format_to_n(_Out __out, iter_difference_t<_Out> __n, const locale& __loc,
		wformat_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Iter_sink<wchar_t, _Out> __sink(std::move(__out), __n);
      std::vformat_to(__sink.out(), __loc, __fmt.get(),
		      std::make_wformat_args(__args...));
      return std::move(__sink)._M_finish();
    }

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
#if 1
  template<typename _CharT>
    class _Counting_sink : public _Iter_sink<_CharT, _CharT*>
    {
    public:
      _Counting_sink() : _Iter_sink<_CharT, _CharT*>(nullptr, 0) { }

      [[__gnu__::__always_inline__]]
      size_t
      count()
      {
	_Counting_sink::_M_overflow();
	return this->_M_count;
      }
    };
#else
  template<typename _CharT>
    class _Counting_sink : public _Buf_sink<_CharT>
    {
      size_t _M_count = 0;

      void
      _M_overflow() override
      {
	if (!std::is_constant_evaluated())
	  _M_count += this->_M_used().size();
	this->_M_rewind();
      }

    public:
      _Counting_sink() = default;

      [[__gnu__::__always_inline__]]
      size_t
      count() noexcept
      {
	_Counting_sink::_M_overflow();
	return _M_count;
      }
    };
#endif
} // namespace __format
/// @endcond

  template<typename... _Args>
    [[nodiscard]]
    inline size_t
    formatted_size(format_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Counting_sink<char> __buf;
      std::vformat_to(__buf.out(), __fmt.get(),
		      std::make_format_args(std::forward<_Args>(__args)...));
      return __buf.count();
    }

  template<typename... _Args>
    [[nodiscard]]
    inline size_t
    formatted_size(wformat_string<_Args...> __fmt, _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Counting_sink<wchar_t> __buf;
      std::vformat_to(__buf.out(), __fmt.get(),
		      std::make_wformat_args(std::forward<_Args>(__args)...));
      return __buf.count();
    }

  template<typename... _Args>
    [[nodiscard]]
    inline size_t
    formatted_size(const locale& __loc, format_string<_Args...> __fmt,
		   _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Counting_sink<char> __buf;
      std::vformat_to(__buf.out(), __loc, __fmt.get(),
		      std::make_format_args(std::forward<_Args>(__args)...));
      return __buf.count();
    }

  template<typename... _Args>
    [[nodiscard]]
    inline size_t
    formatted_size(const locale& __loc, wformat_string<_Args...> __fmt,
		   _Args&&... __args)
    {
      CXX20_FORMAT_DECORATE_NAME(__format)::_Counting_sink<wchar_t> __buf;
      std::vformat_to(__buf.out(), __loc, __fmt.get(),
		      std::make_wformat_args(std::forward<_Args>(__args)...));
      return __buf.count();
    }

#if __cpp_lib_format_ranges
  // [format.range], formatting of ranges
  // [format.range.fmtkind], variable template format_kind
  enum class range_format {
    disabled,
    map,
    set,
    sequence,
    string,
    debug_string
  };

  /// @cond undocumented
  template<typename _Rg>
    constexpr auto format_kind = not defined(format_kind<_Rg>);

  template<typename _Tp>
    consteval range_format
    __fmt_kind()
    {
      using _Ref = ranges::range_reference_t<_Tp>;
      if constexpr (is_same_v<remove_cvref_t<_Ref>, _Tp>)
	return range_format::disabled;
      else if constexpr (requires { typename _Tp::key_type; })
	{
	  if constexpr (requires { typename _Tp::mapped_type; })
	    {
	      using _Up = remove_cvref_t<_Ref>;
	      if constexpr (__is_pair<_Up>)
		return range_format::map;
	      else if constexpr (__is_specialization_of<_Up, tuple>)
		if constexpr (tuple_size_v<_Up> == 2)
		  return range_format::map;
	    }
	  return range_format::set;
	}
      else
	return range_format::sequence;
    }
  /// @endcond

  /// A constant determining how a range should be formatted.
  template<ranges::input_range _Rg> requires same_as<_Rg, remove_cvref_t<_Rg>>
    constexpr range_format format_kind<_Rg> = __fmt_kind<_Rg>();

  // [format.range.formatter], class template range_formatter
  template<typename _Tp, typename _CharT = char>
    requires same_as<remove_cvref_t<_Tp>, _Tp> && formattable<_Tp, _CharT>
    class range_formatter; // TODO

/// @cond undocumented
namespace CXX20_FORMAT_DECORATE_NAME(__format)
{
  // [format.range.fmtdef], class template range-default-formatter
  template<range_format _Kind, ranges::input_range _Rg, typename _CharT>
    struct __range_default_formatter; // TODO
} // namespace __format
/// @endcond

  // [format.range.fmtmap], [format.range.fmtset], [format.range.fmtstr],
  // specializations for maps, sets, and strings
  template<ranges::input_range _Rg, typename _CharT>
    requires (format_kind<_Rg> != range_format::disabled)
      && formattable<ranges::range_reference_t<_Rg>, _CharT>
    struct formatter<_Rg, _CharT>
    : CXX20_FORMAT_DECORATE_NAME(__format)::__range_default_formatter<format_kind<_Rg>, _Rg, _CharT>
    { };
#endif // C++23 formatting ranges

} // namespace CXX20_FORMAT_NAMESPACE
} // namespace std
#endif // C++20
#endif // CXX20_FORMAT_H
