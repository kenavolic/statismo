/*
 * This file is part of the statismo library.
 *
 * Copyright (c) 2019 Laboratory of Medical Information Processing
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __REFLECT_H_
#define __REFLECT_H_

#include <type_traits>

// Compile-time reflection utilities
namespace statismo
{
namespace details
{
// general compile-time validator (ref: C++ template the complete guide)
template <typename F, typename... Args, typename = decltype(std::declval<F>()(std::declval<Args &&>()...))>
auto
is_valid_impl(void *) -> std::true_type;

template <typename F, typename... Args>
auto
is_valid_impl(...) -> std::false_type;
} // namespace details

inline constexpr auto is_valid = [](auto f) {
  using input_type = decltype(f);
  return [](auto &&... args) { return decltype(details::is_valid_impl<input_type, decltype(args) &&...>(nullptr)){}; };
};

template <typename T>
struct type_t
{
  using type = T;
};

template <typename T>
constexpr auto type = type_t<T>{};

template <typename T>
T value_t(type_t<T>);

} // namespace statismo

#endif