/*
 * Copyright (c) 2015 University of Basel
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
 */

#ifndef __STATISMO_ITK_PENALIZING_MEAN_SQUARES_IMAGE_TO_IMAGE_METRIC_H_
#define __STATISMO_ITK_PENALIZING_MEAN_SQUARES_IMAGE_TO_IMAGE_METRIC_H_

#include <itkMeanSquaresImageToImageMetric.h>
/**
 * This is a simple extension of the MeanSquaresImageToImageMetric, which adds a regularization
 * of the type regWeight * ||p||^2, where p denote the parameters of the model transformation.
 */
namespace itk
{
template <class TFixedImage, class TMovingImage>
class ITK_EXPORT PenalizingMeanSquaresImageToImageMetric
  : public MeanSquaresImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  // Standard class typedefs.
  using Self = PenalizingMeanSquaresImageToImageMetric;
  using Superclass = MeanSquaresImageToImageMetric<TFixedImage, TMovingImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  // Method for creation through the object factory.
  itkNewMacro(Self);
  ITK_DISALLOW_COPY_AND_ASSIGN(PenalizingMeanSquaresImageToImageMetric);
  // Run-time type information (and related methods).
  itkTypeMacro(PenalizingMeanSquaresImageToImageMetric, Object);

  // Types transferred from the base class
  using ParametersType = typename Superclass::ParametersType;
  using MeasureType = typename Superclass::MeasureType;
  using DerivativeType = typename Superclass::DerivativeType;

  // Get the derivatives of the match measure.
  void
  GetDerivative(const ParametersType & parameters, DerivativeType & derivative) const override
  {
    Superclass::GetDerivative(parameters, derivative);
    CalculateDerivativePenalty(parameters, derivative);
  }

  // Get the value for single valued optimizers.
  MeasureType
  GetValue(const ParametersType & parameters) const override
  {
    MeasureType value = Superclass::GetValue(parameters);
    return value + CalculateValuePenalty(parameters);
  }

  // Get value and derivatives for multiple valued optimizers.
  void
  GetValueAndDerivative(const ParametersType & parameters,
                        MeasureType &          value,
                        DerivativeType &       derivative) const override
  {
    Superclass::GetValueAndDerivative(parameters, value, derivative);
    value += CalculateValuePenalty(parameters);
    CalculateDerivativePenalty(parameters, derivative);
  }

  itkSetMacro(RegularizationParameter, MeasureType);
  itkGetMacro(RegularizationParameter, MeasureType);

  itkSetMacro(NumberOfModelComponents, unsigned);
  itkGetMacro(NumberOfModelComponents, unsigned);

protected:
  PenalizingMeanSquaresImageToImageMetric() = default;
  virtual ~PenalizingMeanSquaresImageToImageMetric() = default;

  MeasureType m_RegularizationParameter;
  unsigned    m_NumberOfModelComponents{ 0 };

private:
  MeasureType
  CalculateValuePenalty(const ParametersType & parameters) const
  {
    MeasureType regValue = 0;
    for (unsigned int par = 0; par < m_NumberOfModelComponents; ++par)
    {
      regValue += parameters[par] * parameters[par];
    }
    return regValue * m_RegularizationParameter;
  }
  void
  CalculateDerivativePenalty(const ParametersType & parameters, DerivativeType & derivative) const
  {
    for (unsigned int i = 0; i < m_NumberOfModelComponents; ++i)
    {
      derivative[i] += parameters[i] * 2 * m_RegularizationParameter;
    }
  }
};
} // end namespace itk
#endif