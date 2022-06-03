#include "AssociationHandle.h"
#include "SimplificationHandle.h"

namespace TopologyFileFormat {

AssociationHandle::AssociationHandle(const AssociationHandle& handle) : FileHandle(handle)
{
  mSource = handle.mSource;
  mDestination = handle.mDestination;
  mEdges = handle.mEdges;
  mValues = handle.mValues;
}


AssociationHandle& AssociationHandle::operator=(const AssociationHandle& handle)
{
  FileHandle::operator=(handle);

  mSource = handle.mSource;
  mDestination = handle.mDestination;
  mEdges = handle.mEdges;
  mValues = handle.mValues;

  return *this;
}

FileHandle& AssociationHandle::add(const FileHandle& handle)
{
  switch (handle.type()) {
    case H_EDGE:

      mEdges.push_back( dynamic_cast<const EdgeHandle&>(handle));
      mEdges.back().topHandle(clanHandle());

      return mEdges.back();
    case H_STAT:
      mValues.push_back(dynamic_cast<const StatHandle&>(handle));
      return mValues.back();
    default:
      sterror(true,"Unexpected node type.");
  }

  return *this;
}

bool AssociationHandle::providesValue(const std::string& value_name) const
{
  std::vector<StatHandle>::const_iterator it;

  for (it=mValues.begin();it!=mValues.end();it++) {
    if (it->species() == value_name)
      return true;
  }

  return false;
}

std::vector<std::string> AssociationHandle::values() const
{
  std::vector<StatHandle>::const_iterator it;
  std::vector<std::string> list;

  for (it=mValues.begin();it!=mValues.end();it++)
    list.push_back(it->species());

  return list;
}

StatHandle AssociationHandle::value(const std::string& value) const
{
  std::vector<StatHandle>::const_iterator it;

  for (it=mValues.begin();it!=mValues.end();it++)
    if (it->species() == value)
      return *it;

  return StatHandle();
}

void AssociationHandle::clear()
{
  mEdges.clear();

  mSource = SimplificationKey();
  mDestination = SimplificationKey();

  mValues.clear();
}

void AssociationHandle::topHandle(FileHandle* top)
{
  std::vector<StatHandle>::iterator it;
  mTopHandle = top;
  std::vector<EdgeHandle>::iterator eIt;

  for (eIt = mEdges.begin(); eIt != mEdges.end(); eIt++)
      eIt->topHandle(clanHandle());


  for (it=mValues.begin();it!=mValues.end();it++)
    it->topHandle(clanHandle());
}

int AssociationHandle::parseXML(const XMLNode& association)
{
  FileHandle* handle;

  parseXMLInternal(association);

  for (int i=0;i<association.nChildNode();i++) {

    if (strcmp(association.getChildNode(i).getName(),"source") == 0) {
      mSource.parseXML(association.getChildNode(i));
    }
    else if (strcmp(association.getChildNode(i).getName(),"destination")==0) {
      mDestination.parseXML(association.getChildNode(i));
    }
    else {

      handle = this->constructHandle(association.getChildNode(i).getName(),mFileName);
      handle->topHandle(clanHandle());

      handle->parseXML(association.getChildNode(i));

      switch (handle->type()) {
        case H_EDGE:
            mEdges.push_back(*dynamic_cast<EdgeHandle*>(handle));
            //  mEdges.topHandle(clanHandle());
          break;
        case H_STAT:
          mValues.push_back(*dynamic_cast<StatHandle*>(handle));
          mValues.back().topHandle(clanHandle());
          break;
        default:
          sterror(true,"Unexpected node type.");
      }

      delete handle;
    }
  }

  return 1;
}

int AssociationHandle::attachXMLInternal(XMLNode& node) const
{
  XMLNode child;
  std::vector<StatHandle>::const_iterator it;

  FileHandle::attachXMLInternal(node);

  child = node.addChild("source");
  mSource.attachXML(child);

  child = node.addChild("destination");
  mDestination.attachXML(child);

  for (uint16_t i = 0; i < mEdges.size(); i++) {

      child = node.addChild(mEdges[i].name());
      mEdges[i].attachXMLInternal(child);
  }
  //child = node.addChild(mEdges.name());
  //mEdges.attachXMLInternal(child);

  for (it=mValues.begin();it!=mValues.end();it++) {
    child = node.addChild(it->name());
    it->attachXMLInternal(child);
  }

  return 1;
}

int AssociationHandle::writeData(std::ofstream& output, const std::string& filename)
{
  std::vector<StatHandle>::iterator it;

  this->mFileName = filename;
  this->mOffset = static_cast<FileOffsetType>(output.tellp());
  for (uint16_t i = 0; i < mEdges.size(); i++) {
      mEdges[i].writeData(output, this->mFileName);
  }

  //for (it=this->mValues.begin();it!=this->mValues.end();it++)
  //  it->writeData(output,filename);

  return 1;
}

}




