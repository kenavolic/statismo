/*
 * This file is part of the statismo library.
 *
 * Author: Marcel Luethi (marcel.luethi@unibas.ch)
 *
 * Copyright (c) 2011 University of Basel
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

#include "statismo/core/Utils.h"
#include "statismo/core/RandUtils.h"

#include <random>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include <random>

namespace statismo::utils
{
VectorType
GenerateNormalVector(unsigned n)
{
  // we make the random generate static, to ensure that the seed is only set once, and not with
  // every call
  static std::normal_distribution<> dist(0, 1);
  static auto                       r = std::bind(dist, rand::RandGen());

  VectorType v = VectorType::Zero(n);
  for (unsigned i = 0; i < n; i++)
  {
    v(i) = r();
  }
  return v;
}

VectorType
ReadVectorFromTxtFile(const char * name)
{
  using ListType = std::list<statismo::ScalarType>;
  ListType      values;
  std::ifstream inFile(name, std::ios::in);
  if (inFile.good())
  {
    std::copy(std::istream_iterator<statismo::ScalarType>(inFile),
              std::istream_iterator<statismo::ScalarType>(),
              std::back_insert_iterator<ListType>(values));
  }
  else
  {
    throw StatisticalModelException((std::string("Could not read text file ") + name).c_str(), Status::BAD_INPUT_ERROR);
  }

  VectorType v = VectorType::Zero(values.size());
  unsigned   i = 0;
  for (auto it = std::cbegin(values); it != std::cend(values); ++it)
  {
    v(i) = *it;
    i++;
  }
  return v;
}

std::string
CreateTmpName(const std::string & extension)
{
  // https://github.com/statismo/statismo/pull/268/files
  // \note replace by std::filesystem in the future

  // imitates the path that was generated by boost::filesystem::unique_path to make sure we don't break anything
  static const char                      pathChars[] = "0123456789abcdefghiklmnopqrstuvwxyz";
  static std::uniform_int_distribution<> randIndex(
    0, sizeof(pathChars) - 2); //-1 for the \0 terminator and -1 because the index starts at 0

  std::string mask = "%%%%-%%%%-%%%%-%%%%";
  for (auto & c : mask)
  {
    if (c == '%')
    {
      c = pathChars[randIndex(rand::RandGen())];
    }
  }

  return mask + extension;
}

void
RemoveFile(const std::string & str)
{
  std::remove(str.c_str());
}

void
ToLower(std::string & str)
{
  std::transform(std::begin(str), std::end(str), std::begin(str), [](unsigned char c) { return std::tolower(c); });
}

std::string
ToLowerCopy(std::string str)
{
  ToLower(str);
  return str;
}
} // namespace statismo::utils