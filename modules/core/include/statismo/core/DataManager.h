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
 * PROFITS; OR BUSINESS addINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __STATIMO_CORE_DATA_MANAGER_H_
#define __STATIMO_CORE_DATA_MANAGER_H_

#include "statismo/core/CommonTypes.h"
#include "statismo/core/GenericFactory.h"
#include "statismo/core/NonCopyable.h"
#include "statismo/core/Representer.h"
#include "statismo/core/DataItem.h"
#include "statismo/core/Logger.h"

#include <list>

/**
 * \defgroup DataManagers Data management classes and routines
 */

namespace statismo
{

/**
 * \brief Wrapper to hold training and test data used for
 * Crossvalidation
 *
 * \ingroup DataManagers
 * \ingroup Core
 */
template <typename T>
class CrossValidationFold
{
public:
  using DataItemType = DataItem<T>;
  using DataItemListType = std::list<SharedPtrType<DataItemType>>;

  /**
   * \brief Create an empty fold
   */
  CrossValidationFold() = default;

  /**
   * \brief Create a fold with the given \a trainingData and \a testingData
   */
  CrossValidationFold(DataItemListType trainingData, DataItemListType testingData)
    : m_trainingData(std::move(trainingData))
    , m_testingData(std::move(testingData))
  {}

  /**
   * \brief Get training data
   */
  DataItemListType
  GetTrainingData() const
  {
    return m_trainingData;
  }

  /**
   * \brief Get testing data
   */
  DataItemListType
  GetTestingData() const
  {
    return m_testingData;
  }

private:
  DataItemListType m_trainingData;
  DataItemListType m_testingData;
};

/**
 * \brief Base abstract class for data managers
 *
 * Managers manage training and test Data for building Statistical Models and provides
 * functionality for Crossvalidation.
 *
 * The DataManager class provides functionality for loading and managing data sets to be used in the
 * statistical model. Datasets are loaded either by using DataManager::AddDataset or directly from a hdf5 File using
 * the Load function. Per default all the datasets are marked as training data. It is, however, often useful
 * to leave a few datasets out to validate the model. For this purpose, the DataManager class implements basic
 * crossvalidation features.
 *
 * For efficiency purposes, the data is internally stored as a large matrix, using the internal SampleVector
 * representation. Furthermore, Statismo emphasizes on traceability, and ties information with the datasets, such as the
 * original filename. This means that when accessing the data stored in the DataManager, one gets a DataItem structure.
 *
 * \sa Representer
 * \sa DataItem
 *
 * \ingroup DataManagers
 * \ingroup Core
 */
template <typename T>
class DataManager : public NonCopyable
{
public:
  using RepresenterType = Representer<T>;
  using DatasetPointerType = typename RepresenterType::DatasetPointerType;
  using DatasetConstPointerType = typename RepresenterType::DatasetConstPointerType;
  using DataItemType = DataItem<T>;
  using DataItemListType = std::list<SharedPtrType<DataItemType>>;
  using CrossValidationFoldType = CrossValidationFold<T>;
  using CrossValidationFoldListType = std::list<CrossValidationFoldType>;

  /**
   * \brief Add a dataset to the manager
   * \param dataset the dataset to be added
   * \param uri string containing the URI of the given dataset (added as a metadata)
   *
   * While it is not strictly necessary, and sometimes not even possible, to specify a URI for the given dataset,
   * it is strongly encouraged to add a description. The string will be added to the metadata and stored with the model.
   * Having this information stored with the model may prove valuable at a later point in time.
   */
  virtual void
  AddDataset(DatasetConstPointerType dataset, const std::string & uri) = 0;

  /**
   * \brief Save the data matrix and all URIs into an HDF5 file.
   * \param filename
   */
  virtual void
  Save(const std::string & filename) const = 0;

  /**
   * \brief Get all sample data objects managed by the data manager
   * \sa DataItem
   */
  virtual DataItemListType
  GetData() const = 0;

  /**
   * \brief Get the number of samples managed by the data manager
   */
  virtual std::size_t
  GetNumberOfSamples() const = 0;

  /**
   * \brief Assigns the data to one of n Folds to be used for cross validation.
   * \warning This method has to be called before cross validation can be started.
   *
   * \param nFolds number of folds used in the crossvalidation
   * \param isRandomized if true, the data will be randomly assigned to the nfolds, otherwise the order with which it
   * was added is preserved
   */
  virtual CrossValidationFoldListType
  GetCrossValidationFolds(unsigned nFolds, bool isRandomized) const = 0;

  /**
   * \brief Generates Leave-one-out cross validation folds
   */
  virtual CrossValidationFoldListType
  GetLeaveOneOutCrossValidationFolds() const = 0;

  /**
   * \brief Generic delete function
   */
  virtual void
  Delete() = 0;

  /**
   * \brief Get logger
   */
  virtual Logger *
  GetLogger() const = 0;
};

/**
 * \brief Base implementation for data managers
 *
 * \ingroup DataManagers
 * \ingroup Core
 */
template <typename T, typename Derived>
class DataManagerBase
  : public DataManager<T>
  , public GenericFactory<Derived>
{
public:
  using Superclass = DataManager<T>;
  using RepresenterType = typename Superclass::RepresenterType;
  using DatasetPointerType = typename Superclass::DatasetPointerType;
  using DatasetConstPointerType = typename Superclass::DatasetConstPointerType;
  using DataItemType = typename Superclass::DataItemType;
  using DataItemListType = typename Superclass::DataItemListType;
  using CrossValidationFoldType = typename Superclass::CrossValidationFoldType;
  using CrossValidationFoldListType = typename Superclass::CrossValidationFoldListType;
  using ObjectFactoryType = GenericFactory<Derived>;

  friend ObjectFactoryType;

public:
  // virtual ~DataManagerBase() = default;

  void
  Save(const std::string & filename) const override;

  DataItemListType
  GetData() const override;

  std::size_t
  GetNumberOfSamples() const override
  {
    return m_dataItemList.size();
  }

  CrossValidationFoldListType
  GetCrossValidationFolds(unsigned nFolds, bool isRandomized = true) const override;

  CrossValidationFoldListType
  GetLeaveOneOutCrossValidationFolds() const override;

  virtual void
  SetLogger(Logger * logger)
  {
    m_logger = logger;
  }

  void
  Delete() override
  {
    delete this;
  }

protected:
  explicit DataManagerBase(const RepresenterType * representer);

  template <typename ConcreteDataItemType, typename... Args>
  static UniquePtrType<DataManagerBase>
  Load(RepresenterType * representer, const std::string & filename, Args &&... args);

  Logger *
  GetLogger() const override
  {
    return m_logger;
  }

  UniquePtrType<RepresenterType> m_representer;
  DataItemListType               m_dataItemList;

private:
  Logger * m_logger{ nullptr };
};

/**
 * \brief Standard data manager
 *
 * \ingroup DataManagers
 * \ingroup Core
 */
template <typename T>
class BasicDataManager : public DataManagerBase<T, BasicDataManager<T>>
{
public:
  using Superclass = DataManagerBase<T, BasicDataManager<T>>;
  using RepresenterType = typename Superclass::RepresenterType;
  using DataItemType = typename Superclass::DataItemType;
  using BasicDataItemType = BasicDataItem<T>;
  using DatasetConstPointerType = typename Superclass::DatasetConstPointerType;
  using ObjectFactoryType = typename Superclass::ObjectFactoryType;

  friend ObjectFactoryType;

  /**
   * Create a new dataManager, with the data stored in the given hdf5 file
   */
  static UniquePtrType<Superclass>
  Load(RepresenterType * representer, const std::string & filename)
  {
    return Superclass::template Load<BasicDataItem<T>>(representer, filename);
  }

  void
  AddDataset(DatasetConstPointerType dataset, const std::string & uri) override;

private:
  explicit BasicDataManager(const RepresenterType * representer)
    : Superclass(representer)
  {}
};

} // namespace statismo

#include "DataManager.hxx"

#endif
