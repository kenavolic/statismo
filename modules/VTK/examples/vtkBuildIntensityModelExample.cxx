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

#include "statismo/core/DataManager.h"
#include "statismo/core/PCAModelBuilder.h"
#include "statismo/core/StatisticalModel.h"
#include "statismo/core/IO.h"
#include "statismo/VTK/vtkStandardImageRepresenter.h"

#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkNew.h>

#include <iostream>
#include <ostream>

#include <memory>

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
// Build a new shape model from vtkPolyData, given in datadir.
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
  using DataManagerType = BasicDataManager<vtkStructuredPoints>;
  using ModelBuilderType = PCAModelBuilder<vtkStructuredPoints>;

  try
  {

    // Model building is exactly the same as for shape models (see BuildShapeModelExample for detailed explanation)
    auto reference = LoadVTKStructuredPointsData(datadir + "/hand-0.vtk");
    auto representer = RepresenterType::SafeCreate(reference);
    auto dataManager = DataManagerType::SafeCreate(representer.get());

    // load the data and add it to the data manager. We take the first 4 hand shapes that we find in the data folder
    for (unsigned i = 0; i < 4; i++)
    {
      std::ostringstream ss;
      ss << datadir + "/hand-" << i << ".vtk";
      std::string datasetFilename = ss.str();
      auto        dataset = LoadVTKStructuredPointsData(datasetFilename);

      std::cout << "adding " << datasetFilename << std::endl;

      // We provde the filename as a second argument.
      // It will be written as metadata, and allows us to more easily figure out what we did later.
      dataManager->AddDataset(dataset, datasetFilename);
    }

    auto modelBuilder = ModelBuilderType::SafeCreate();
    auto model = modelBuilder->BuildNewModel(dataManager->GetData(), 0.01);
    statismo::IO<vtkStructuredPoints>::SaveStatisticalModel(model.get(), modelname);

    std::cout << "Successfully saved model as " << modelname << std::endl;
  }
  catch (const StatisticalModelException & e)
  {
    std::cerr << "Exception occured while building the intensity model" << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
