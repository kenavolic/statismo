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

#ifndef __STATIMO_ITK_INTERPOLATING_STATISTICAL_DEFORMATION_MODEL_TRANSFORM_H_
#define __STATIMO_ITK_INTERPOLATING_STATISTICAL_DEFORMATION_MODEL_TRANSFORM_H_

#include "statismo/core/Representer.h"
#include "statismo/ITK/itkStandardImageRepresenter.h"
#include "statismo/ITK/itkStatisticalModel.h"
#include "statismo/ITK/itkStatisticalModelTransformBase.h"

#include <itkImage.h>
#include <itkVector.h>
#include <itkVectorLinearInterpolateImageFunction.h>

namespace itk
{

/**
 * \brief SSM deformation transform with interpolation
 *
 * In contrast to the standard itk::StatisticalDeformationModelTransform, this transform performs a linear interpolation
 * of the PCABasis. This has the advantage that a model can be fitted which has a much lower resolution that the image
 * to be explained.
 *
 * \sa itk::StatisticalDeformationModelTransform
 * \ingroup Transforms
 * \ingroup ITK
 */
template <typename Dataset, typename ScalarType, unsigned int DIMENSION>
class ITK_EXPORT InterpolatingStatisticalDeformationModelTransform
  : public itk::StatisticalModelTransformBase<Dataset, ScalarType, DIMENSION>
{
public:
  using Self = InterpolatingStatisticalDeformationModelTransform;
  using Superclass = itk::StatisticalModelTransformBase<Dataset, ScalarType, DIMENSION>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkSimpleNewMacro(Self);
  /** Run-time type information (and related methods). */
  itkTypeMacro(InterpolatingStatisticalDeformationModelTransform, Superclass);

  using InputPointType = typename Superclass::InputPointType;
  using OutputPointType = typename Superclass::OutputPointType;
  using RepresenterType = typename Superclass::RepresenterType;
  using StatisticalModelType = typename Superclass::StatisticalModelType;
  using JacobianType = typename Superclass::JacobianType;
  using DeformationFieldType = typename RepresenterType::DatasetType;
  using InterpolatorType = VectorLinearInterpolateImageFunction<DeformationFieldType, ScalarType>;

  ::itk::LightObject::Pointer
  CreateAnother() const override
  {
    ::itk::LightObject::Pointer smartPtr;
    Pointer                     another = Self::New().GetPointer();
    this->CopyBaseMembers(another);
    another->m_meanDeformation = this->m_meanDeformation;
    another->m_pcaBasisDeformations = this->m_pcaBasisDeformations;
    smartPtr = static_cast<Pointer>(another);
    return smartPtr;
  }

  /**
   * \brief Set the statistical shape model
   */
  virtual void
  SetStatisticalModel(const StatisticalModelType * model)
  {
    this->Superclass::SetStatisticalModel(model);

    m_meanDeformation = InterpolatorType::New();

    auto meanDf = model->DrawMean();
    m_meanDeformation->SetInputImage(meanDf);
    for (unsigned i = 0; i < model->GetNumberOfPrincipalComponents(); ++i)
    {
      auto deformationField = model->DrawPCABasisSample(i);
      auto basisI = InterpolatorType::New();
      basisI->SetInputImage(deformationField);
      m_pcaBasisDeformations.push_back(basisI);
    }
  }


  void
  ComputeJacobianWithRespectToParameters(const InputPointType & pt, JacobianType & jacobian) const override
  {
    jacobian.SetSize(DIMENSION, m_pcaBasisDeformations.size());
    jacobian.Fill(0);
    if (!m_meanDeformation->IsInsideBuffer(pt))
    {
      return;
    }

    for (unsigned j = 0; j < m_pcaBasisDeformations.size(); ++j)
    {
      auto d = m_pcaBasisDeformations[j]->Evaluate(pt);
      for (unsigned i = 0; i < DIMENSION; ++i)
      {
        jacobian(i, j) += d[i];
      }
    }

    itkDebugMacro(<< "Jacobian with MM:\n" << jacobian);
    itkDebugMacro(<< "After GetMorphableModelJacobian:"
                  << "\nJacobian = \n"
                  << jacobian);
  }

  /**
   * \brief Transform a given point according to the deformation induced by the StatisticalModel,
   * given the current parameters.
   *
   * \param pt point to tranform
   * \return transformed point
   */
  OutputPointType
  TransformPoint(const InputPointType & pt) const override
  {
    if (!m_meanDeformation->IsInsideBuffer(pt))
    {
      return pt;
    }

    assert(this->m_coeffVector.size() == m_pcaBasisDeformations.size());
    auto def = m_meanDeformation->Evaluate(pt);

    for (unsigned i = 0; i < m_pcaBasisDeformations.size(); ++i)
    {
      auto defBasisI = m_pcaBasisDeformations[i]->Evaluate(pt);
      def += (defBasisI * this->m_coeffVector[i]);
    }

    OutputPointType transformedPoint;
    for (unsigned i = 0; i < pt.GetPointDimension(); ++i)
    {
      transformedPoint[i] = pt[i] + def[i];
    }

    return transformedPoint;
  }

private:
  typename InterpolatorType::Pointer              m_meanDeformation;
  std::vector<typename InterpolatorType::Pointer> m_pcaBasisDeformations;
};


} // namespace itk

#endif
