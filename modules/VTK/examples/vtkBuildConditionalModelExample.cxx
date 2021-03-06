/*
 * This file is part of the statismo library.
 *
 * Author: Remi Blanc
 *
 * Copyright (c) 2011, ETH Zurich
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

#include "statismo/core/ConditionalModelBuilder.h"
#include "statismo/core/DataManager.h"
#include "statismo/core/StatisticalModel.h"
#include "statismo/core/IO.h"
#include "statismo/VTK/vtkStandardImageRepresenter.h"

#include <vtkStructuredPointsReader.h>
#include <vtkNew.h>

#include <memory>
#include <iostream>

using namespace statismo;

namespace
{

vtkSmartPointer<vtkStructuredPoints>
LoadVTKStructuredPointsData(const std::string & filename)
{
  vtkNew<vtkStructuredPointsReader> reader;
  reader->SetFileName(filename.c_str());
  reader->Update();

  return reader->GetOutput();
}
} // namespace

//
// This example shows the use of the classes for
// building an intensity model (appearance model) from
// a number of images.
//
int
main(int argc, char ** argv)
{
  if (argc < 3)
  {
    std::cerr << "Usage " << argv[0] << " datadir modelname" << std::endl;
    return 1;
  }
  std::string datadir(argv[1]);
  std::string modelname(argv[2]);


  // All the statismo classes have to be parameterized with the RepresenterType.
  // For building a intensity model with vtk, we use the vtkStructuredPointsRepresenter.
  // Here, we work with unsigned character images. The second template parameter specifies
  // the pixel dimension (1 means scalar image, whereas 3 is a 3D vector image).
  using RepresenterType = vtkStandardImageRepresenter<unsigned char, 1>;
  using DataManagerWithSurrogatesType = DataManagerWithSurrogates<vtkStructuredPoints>;
  using ConditionalModelBuilderType = ConditionalModelBuilder<vtkStructuredPoints>;

  try
  {
    auto reference = LoadVTKStructuredPointsData(datadir + "/hand-0.vtk");
    auto representer = RepresenterType::SafeCreate(reference);

    // We use the SurrogateDataManager, as we need to specify surrogate data in addition to the images.
    // We provide in addition to the representer also a file that contains a description of the surrogate
    // variables (e.g. whether they are categorical or continuous). See the API doc for more details.
    auto dataManager =
      DataManagerWithSurrogatesType::SafeCreate(representer.get(), datadir + "/surrogates/hand_surrogates_types.txt");

    // add the data information. The first argument is the dataset, the second the surrogate information
    // and the 3rd the surrogate type

    // load the data and add it to the data manager. We take the first 4 hand images that we find in the data folder
    for (unsigned i = 0; i < 4; i++)
    {
      std::ostringstream ssFilename;
      ssFilename << datadir << "/hand-" << i << ".vtk";
      std::string datasetFilename = ssFilename.str();

      std::ostringstream ssSurrogateFilename;
      ssSurrogateFilename << datadir << "/surrogates/hand-" << i << "_surrogates.txt";

      // We provde the filename as a second argument.
      // It will be written as metadata, and allows us to more easily figure out what we did later.
      dataManager->AddDatasetWithSurrogates(
        LoadVTKStructuredPointsData(datasetFilename), datasetFilename, ssSurrogateFilename.str());
    }

    // Build up a list holding the conditioning information.
    ConditionalModelBuilderType::CondVariableValueVectorType conditioningInfo;
    conditioningInfo.emplace_back(true, 1);
    conditioningInfo.emplace_back(false, 65);
    conditioningInfo.emplace_back(true, 86.1);
    conditioningInfo.emplace_back(true, 162.0);

    // Create the model builder and build the model
    auto modelBuilder = ConditionalModelBuilderType::SafeCreate();

    auto model =
      modelBuilder->BuildNewModel(dataManager->GetData(), dataManager->GetSurrogateTypeInfo(), conditioningInfo, 0.1);

    // The resulting model is a normal statistical model, from which we could for example sample examples.
    // Here we simply  save it to disk for later use.
    statismo::IO<vtkStructuredPoints>::SaveStatisticalModel(model.get(), modelname);
    std::cout << "save model as " << modelname << std::endl;
  }
  catch (const std::exception & e)
  {
    std::cerr << "Exception occured while building the conditional model" << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
