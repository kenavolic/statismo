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


#ifndef __STATIMO_ITK_DATAMANAGER_WITH_SURROGATES_H_
#define __STATIMO_ITK_DATAMANAGER_WITH_SURROGATES_H_

#include "statismo/core/DataManagerWithSurrogates.h"
#include "statismo/core/ImplWrapper.h"
#include "statismo/ITK/itkConfig.h"
#include "statismo/ITK/itkUtils.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <functional>
#include <utility>

namespace itk
{


/**
 * \brief ITK Wrapper for statismo::DataManagerWithSurrogates class.
 * \see statismo::DataManagerWithSurrogates for detailed documentation.
 */
template <class Representer>
class DataManagerWithSurrogates
  : public statismo::DataManager<Representer>
  , public statismo::ImplWrapper<statismo::DataManagerWithSurrogates<Representer>>
{
public:
  using Self = DataManagerWithSurrogates;
  using Superclass = statismo::DataManager<Representer>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using ImplType = typename statismo::ImplWrapper<statismo::DataManagerWithSurrogates<Representer>>::ImplType;

  itkNewMacro(Self);
  itkTypeMacro(DataManagerWithSurrogates, Object);

  void
  SetRepresenterAndSurrogateFilename(const Representer * representer, const char * surrogTypeFilename)
  {
    this->SetStatismoImplObj(ImplType::SafeCreate(representer, surrogTypeFilename));
  }

  void
  SetRepresenter(const Representer * representer)
  {
    itkExceptionMacro(<< "Please call SetRepresenterAndSurrogateFilename to initialize the object");
  }


  void
  AddDatasetWithSurrogates(typename Representer::DatasetConstPointerType ds,
                           const char *                                  datasetURI,
                           const char *                                  surrogateFilename)
  {
    this->CallForwardImplTrans(
      statismo::itk::ExceptionHandler{ *this }, &ImplType::AddDasetWithSurrogates, ds, datasetURI, surrogateFilename);
  }
};


} // namespace itk

#endif
