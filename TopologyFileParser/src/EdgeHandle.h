#ifndef EDGEHANDLE_H
#define EDGEHANDLE_H

#include <fstream>
#include "TalassConfig.h"
#include "FileHandle.h"
#include "ComputeTracking.h"

//Need to declare operators before defining Data<> (in DataHandle.h).
namespace TopologyFileFormat 
{
//! Typedef to declare a edge
typedef std::pair<LocalIndexType,LocalIndexType> EdgeType;

std::ostream& operator<<(std::ostream& out, const EdgeType& edge);
std::istream& operator>>(std::istream& in, EdgeType& edge);



}


namespace TopologyFileFormat {

//! A class encapsulating a list of edges stored as pairs of local indices
class EdgeHandle : public FileHandle {

public:

  //! Friend declaration to enable access to functions of the baseclass
  friend class AssociationHandle;

  //! Default constructor
  EdgeHandle();

  //! Constructor
  EdgeHandle(const char* filename);
    //! Copy constructor
  EdgeHandle(const EdgeHandle& handle);

  //! Destructor
  virtual ~EdgeHandle();


  //! Assignment operator
  EdgeHandle& operator=(const EdgeHandle& handle);

  //! Set offset and neighbor vectors
  void setData(ComputeFamilyCorrelation* data);


  //! Read Offset Data
  int readOffsets(std::vector<LocalIndexType>& _offsets);

  //!Read neighbor Data
  int readNeighbors(std::vector<LocalIndexType>& _neighbors);

  //!Read weight Data
  int readWeights(std::vector<double>& _weights);

  //! Return the handle's type
  HandleType type() const { return mType;}

  //! Return whether this handle is ascii or binary
  bool encoding() const {return mASCIIFlag;}

  //! Switch the encoding type
  void encoding(bool ascii_flag) {mASCIIFlag=ascii_flag;}

  void setTrackingType(TrackingType t) {
      tracking_type = t;
  }

  LocalIndexType featureCount() const {return mFeatureCount;}

    std::string getFileName() const {return mFileName;}

	std::vector<LocalIndexType>* getNeighbors() { return mNeighbors; }
	std::vector<LocalIndexType>* getOffsets() { return arrOffsets; }

   

protected:
    TrackingType tracking_type;

  //vector containing offesets to neighbors for source feature id
  std::vector<LocalIndexType>* arrOffsets;

  //vector containing destination ids for a tracking edge
  std::vector<LocalIndexType>* mNeighbors;

  std::vector<double>* mWeights;


  //! The number of features should be identical to mSegmentation->size()
  LocalIndexType mFeatureCount;
  
  //! Flag to indicate whether we should encode binary=false or ascii=true
  bool mASCIIFlag;

  

  //! Parse the information from the xml tree
 virtual void topHandle(FileHandle* top) {mTopHandle = top;}

  //! Parse the xml tree
  virtual int parseXML(const XMLNode& node);

  //! Parse the information from the xml tree
  virtual int parseXMLInternal(const XMLNode& node);

  //! Add the local attribute to the node
  virtual int attachXMLInternal(XMLNode& node) const;

  //! Write the local data
  virtual int writeData(std::ofstream& output, const std::string& filename);

};

} // end of namespace


#endif /* EDGEHANDLE_H */
