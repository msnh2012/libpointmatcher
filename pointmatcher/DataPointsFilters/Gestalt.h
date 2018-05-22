#pragma once

#include "PointMatcher.h"

//! Gestalt descriptors filter as described in Bosse & Zlot ICRA 2013
template<typename T>
struct GestaltDataPointsFilter: public PointMatcher<T>::DataPointsFilter
{
	typedef PointMatcherSupport::Parametrizable Parametrizable;
	typedef PointMatcherSupport::Parametrizable P;
	typedef Parametrizable::Parameters Parameters;
	typedef Parametrizable::ParameterDoc ParameterDoc;
	typedef Parametrizable::ParametersDoc ParametersDoc;
	typedef Parametrizable::InvalidParameter InvalidParameter;
	
	typedef typename PointMatcher<T>::Vector Vector;
	typedef typename PointMatcher<T>::Matrix Matrix;	
	typedef typename PointMatcher<T>::DataPoints DataPoints;
	typedef typename PointMatcher<T>::DataPoints::InvalidField InvalidField;
	
  inline static const std::string description()
  {
    return "Gestalt descriptors filter.";
  }
  inline static const ParametersDoc availableParameters()
  {
    return boost::assign::list_of<ParameterDoc>
    ( "ratio", "ratio of points to keep with random subsampling. Matrix (normal, density, etc.) will be associated to all points in the same bin.", "0.1", "0.0000001", "0.9999999", &P::Comp<T> )
    ( "radius", "is the radius of the gestalt descriptor, will be divided into 4 circular and 8 radial bins = 32 bins", "5", "0.1", "2147483647", &P::Comp<T> )
    ( "knn", "determined how many points are used to compute the normals. Direct link with the rapidity of the computation (large = fast). Technically, limit over which a box is splitted in two", "7", "3", "2147483647", &P::Comp<unsigned> )
    ( "vSizeX", "Dimension of each voxel cell in x direction", "1.0", "-inf", "inf", &P::Comp<T> )
    ( "vSizeY", "Dimension of each voxel cell in y direction", "1.0", "-inf", "inf", &P::Comp<T> )
    ( "vSizeZ", "Dimension of each voxel cell in z direction", "1.0", "-inf", "inf", &P::Comp<T> )
    ( "keepMeans", "whether the means should be added as descriptors to the resulting cloud", "0" )
    ( "maxBoxDim", "maximum length of a box above which the box is discarded", "inf" )
    ( "averageExistingDescriptors", "whether the filter keep the existing point descriptors and average them or should it drop them", "1" )
    ( "maxTimeWindow", "maximum spread of times in a surfel", "inf" )
    ( "keepNormals", "whether the normals should be added as descriptors to the resulting cloud", "1" )
    ( "keepEigenValues", "whether the eigen values should be added as descriptors to the resulting cloud", "0" )
    ( "keepEigenVectors", "whether the eigen vectors should be added as descriptors to the resulting cloud", "0" )
    ( "keepCovariances", "whether the covariances should be added as descriptors to the resulting cloud", "0" )
    ( "keepGestaltFeatures", "whether the Gestalt features shall be added to the resulting cloud", "1" )
    ;
  }

  const T ratio;
  const T radius;
  const unsigned knn;
  const T vSizeX;
  const T vSizeY;
  const T vSizeZ;
  const T maxBoxDim;
  const T maxTimeWindow;
  const bool keepMeans;
  const bool averageExistingDescriptors;
  const bool keepNormals;
  const bool keepEigenValues;
  const bool keepEigenVectors;
  const bool keepCovariances;
  const bool keepGestaltFeatures;


 public:
  GestaltDataPointsFilter(const Parameters& params = Parameters());
  virtual ~GestaltDataPointsFilter() {}
  virtual DataPoints filter(const DataPoints& input);
  virtual void inPlaceFilter(DataPoints& cloud);
  
  typename PointMatcher<T>::Vector serializeGestaltMatrix(const Matrix& gestaltFeatures) const;
  typename PointMatcher<T>::Vector calculateAngles(const Matrix& points, const Eigen::Matrix<T,3,1>&) const;
  typename PointMatcher<T>::Vector calculateRadii(const Matrix& points, const Eigen::Matrix<T,3,1>&) const;


 protected:
  struct BuildData
  {
    typedef std::vector<int> Indices;
    typedef typename DataPoints::View View;
    typedef typename Eigen::Matrix<boost::int64_t, Eigen::Dynamic, Eigen::Dynamic> Int64Matrix;
    typedef typename Eigen::Matrix<boost::int64_t, 1, Eigen::Dynamic> Int64Vector;

    Indices indices;
    Indices indicesToKeep;
    Matrix& features;
    Matrix& descriptors;
    Int64Matrix& times;
    boost::optional<View> normals;
    boost::optional<View> means;
    boost::optional<View> eigenValues;
    boost::optional<View> eigenVectors;
    boost::optional<View> covariance;
    boost::optional<View> gestaltMeans;
    boost::optional<View> gestaltVariances;
    boost::optional<View> gestaltShapes;
    boost::optional<View> warpedXYZ;
    int outputInsertionPoint;
    int unfitPointsCount;

    BuildData(Matrix& features, Matrix& descriptors, Int64Matrix& times):
      features(features),
      descriptors(descriptors),
      times(times),
      unfitPointsCount(0)
    {
      const int pointsCount(features.cols());
      indices.reserve(pointsCount);
      for (int i = 0; i < pointsCount; ++i)
        indices.push_back(i);
    }
  };

  struct CompareDim
  {
    const int dim;
    const BuildData& buildData;
    CompareDim(const int dim, const BuildData& buildData):dim(dim),buildData(buildData){}
    bool operator() (const int& p0, const int& p1)
    {
      return buildData.features(dim, p0) <
          buildData.features(dim, p1);
    }
  };

 protected:
  void buildNew(BuildData& data, const int first, const int last, Vector&& minValues, Vector&& maxValues) const;
  void fuseRange(BuildData& data, DataPoints& input, const int first, const int last) const;

};
