#ifndef ASSOCIATIONHANDLE_H
#define ASSOCIATIONHANDLE_H

#include <vector>
#include <string>
#include "xmlParser.h"
#include "FileHandle.h"
#include "StatHandle.h"
#include "EdgeHandle.h"
#include "FamilyKey.h"
#include "SimplificationKey.h"

namespace TopologyFileFormat {

//! A class encapsulating the associations between two feature families
/*! An AssociationHandle store the associations between to hierarchies
 *  of two (potentially) different feature families. In particular, it
 *  stores edges between features in form of index pairs as well as an
 *  arbitrary number of statistics per edge
 */
class AssociationHandle : public FileHandle
{
public:

  friend class ClanHandle;

  //! Default Constructor
  AssociationHandle() : FileHandle(H_ASSOC) {}

  //! Constructor
  AssociationHandle(const char* filename) : FileHandle(filename,H_ASSOC) {}

  //! Copy constructor
  AssociationHandle(const AssociationHandle& handle);

  //! Destructor
  virtual ~AssociationHandle() {}

  //! Assignment operator
  AssociationHandle& operator=(const AssociationHandle& handle);

  /*******************************************************************************************
   ****************************  Interface to internal handles *******************************
   ******************************************************************************************/

  //! Add the given handle to the internal data structure but don't write
  FileHandle& add(const FileHandle& handle);

  //! Indicate whether the association contains a specific attribute handle
  bool providesValue(const std::string& value_name) const;

  //! Provide a list of all available values
  std::vector<std::string> values() const;

  //! Return the edge handle if one is present. Otherwise return an empty handle
  const EdgeHandle edges() const {return mEdges[0];}

  //! Return a stat handle for the given value
  StatHandle value(const std::string& value) const;

  //! Return the source information
  SimplificationKey source() const {return mSource;}

  //! Return the destination information
  SimplificationKey destination() const {return mDestination;}

  //! Set the source information
  void source(SimplificationKey& info) {mSource = info;}

  //! Set th destination information
  void destination(SimplificationKey& info) {mDestination = info;}

  std::string getFileName() const {return mFileName;}

  bool providesOverlapTracking() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == VERTEX_OVERLAP)
              return true;
      }
      return false;
  }

  bool providesDistanceTracking() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == DISTANCE)
              return true;
      }
      return false;
  }

  bool providesGivenTracking() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == GIVEN)
              return true;
      }
      return false;
  }
  EdgeHandle getOverlapEdgeHandle() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == VERTEX_OVERLAP)
              return edge;
      }
      return NULL;
  }

  EdgeHandle getDistanceEdgeHandle() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == DISTANCE)
              return edge;
      }
      return NULL;
  }
  EdgeHandle getGivenEdgeHandle() {
      bool val = false;
      for (auto& edge : mEdges) {
          if (edge.tracking_type == GIVEN)
              return edge;
      }
      return NULL;

  }
protected:

  //! The information about the source hierarchy
  SimplificationKey mSource;

  //! The information about the destination hierarchy
  SimplificationKey mDestination;

  //! The handle to the edge information
  std::vector<EdgeHandle> mEdges;

  //! The list of values per edge
  std::vector<StatHandle> mValues;

  //! Reset all values to their default uninitialized values
  void clear();

  //! Set the top handle
  void topHandle(FileHandle* top);

  //! Parse the xml tree
  int parseXML(const XMLNode& family);

  //! Attach the local variables to the given node
  int attachXMLInternal(XMLNode& node) const;

  //! Write the data of children to the given file stream
  int writeData(std::ofstream& output, const std::string& filename);

};

} // End of namespace

#endif /* ASSOCIATIONHANDLE_H */
