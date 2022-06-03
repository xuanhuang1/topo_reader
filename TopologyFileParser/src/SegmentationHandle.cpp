/*
 * SegmentationHandle.cpp
 *
 *  Created on: Feb 16, 2012
 *      Author: bremer5
 */

#include "SegmentationHandle.h"

namespace TopologyFileFormat {

SegmentationHandle::SegmentationHandle() : FileHandle(H_SEGMENTATION), mDomainType(UNDEFINED_DOMAIN),
    mDomainDescription(""), mASCIIFlag(false), mFeatureCount(0), mSegmentation(NULL), 
    mFlatSegmentation(NULL), mOffsets(NULL)
{
}

SegmentationHandle::SegmentationHandle(const char* filename) : FileHandle(filename,H_SEGMENTATION),
    mDomainType(UNDEFINED_DOMAIN), mDomainDescription(""), mASCIIFlag(false), mFeatureCount(0), mSegmentation(NULL),
    mFlatSegmentation(NULL), mOffsets(NULL)
{
}

SegmentationHandle::SegmentationHandle(const SegmentationHandle& handle) : FileHandle(handle)
{
  mDomainType = handle.mDomainType;
  mDomainDescription = handle.mDomainDescription;
  mASCIIFlag = handle.mASCIIFlag;
  mIndex = handle.mIndex;
  mLocation = handle.mLocation;
  mGeometry = handle.mGeometry;
  mAnnotation = handle.mAnnotation;
  mFeatureCount = handle.mFeatureCount;
  mSegmentation = handle.mSegmentation;
  mFlatSegmentation = handle.mFlatSegmentation;
  mOffsets = handle.mOffsets;
}

SegmentationHandle::~SegmentationHandle()
{
}

SegmentationHandle& SegmentationHandle::operator=(const SegmentationHandle& handle)
{
  FileHandle::operator=(handle);

  mDomainType = handle.mDomainType;
  mDomainDescription = handle.mDomainDescription;
  mASCIIFlag = handle.mASCIIFlag;
  mIndex = handle.mIndex;
  mLocation = handle.mLocation;
  mGeometry = handle.mGeometry;
  mAnnotation = handle.mAnnotation;
  mFeatureCount = handle.mFeatureCount;
  mSegmentation = handle.mSegmentation;
  mFlatSegmentation = handle.mFlatSegmentation;
  mOffsets = handle.mOffsets;

  return *this;
}

void SegmentationHandle::setSegmentation(std::vector<std::vector<GlobalIndexType> >* segmentation)
{
  mSegmentation = segmentation;
  mFeatureCount = mSegmentation->size();
}

void SegmentationHandle::setOffsets(std::vector<LocalIndexType>* offsets)
{
  mOffsets = offsets;
  mFeatureCount = mOffsets->size()-1;
}

void SegmentationHandle::setSegmentation(std::vector<GlobalIndexType>* segmentation)
{
  mFlatSegmentation = segmentation;
}

FileHandle& SegmentationHandle::add(const FileHandle& handle)
{
  switch (handle.type()) {
    case H_INDEX:
      mIndex = dynamic_cast<const IndexHandle&>(handle);
      mIndex.topHandle(clanHandle());
      return mIndex;
      break;
    case H_GEOMETRY:
      mGeometry = dynamic_cast<const GeometryHandle&>(handle);
      mGeometry.topHandle(clanHandle());
      return mGeometry;
      break;
    case H_ANNOTATION:
      mAnnotation = dynamic_cast<const AnnotationHandle&>(handle);
      mAnnotation.topHandle(clanHandle());
      return mAnnotation;
      break;
    case H_LOCATION:
      mLocation = dynamic_cast<const LocationHandle&>(handle);
      mLocation.topHandle(clanHandle());
      return mLocation;
      break;
    default:
      sterror(true,"Unknown handle type cannot attach a %d to a clan",handle.type());
      break;
  }

  // Note that this is a bogus return to satisfy the compiler. You should hit
  // the error before coming to here
  return mIndex;
}



int SegmentationHandle::readOffsets(std::vector<LocalIndexType>& offsets)
{
  std::ifstream file;

  openInputFile(mFileName,file,!mASCIIFlag);
  rewind(file);

  offsets.resize(mFeatureCount+1);
  Data<LocalIndexType> off(&offsets);

  if (mASCIIFlag)
    off.readASCII(file);
  else
    off.readBinary(file);

  file.close();
  return 1;
}

int SegmentationHandle::readSegmentation(std::vector<GlobalIndexType>& segmentation)
{
  std::ifstream file;
  LocalIndexType size;

  openInputFile(mFileName,file,!mASCIIFlag);
  rewind(file);

  // Read over the offset information except the last offset
  if (mASCIIFlag) {
    for (LocalIndexType i=0;i<mFeatureCount;i++)
      file >> size;
  }
  else {
    file.seekg(mFeatureCount*sizeof(LocalIndexType),std::ios_base::cur);
  }

  // Read the last offset which will contain the number of samples
  if (mASCIIFlag)
    file >> size;
  else
    file.read((char*)&size,sizeof(LocalIndexType));

  segmentation.resize(size);
  Data<GlobalIndexType> seg(&segmentation);

  // Read the rest of the data
  if (mASCIIFlag)
    seg.readASCII(file);
  else
    seg.readBinary(file);

  file.close();
  return 1;
}

int SegmentationHandle::parseXML(const XMLNode& node)
{
  FileHandle* handle;

  parseXMLInternal(node);

  for (int i=0;i<node.nChildNode();i++) {

    handle = this->constructHandle(node.getChildNode(i).getName(),mFileName);
    handle->topHandle(clanHandle());
    handle->parseXML(node.getChildNode(i));

    switch (handle->type()){

      case H_INDEX:
        mIndex = *dynamic_cast<IndexHandle*>(handle);
        break;
      case H_GEOMETRY:
        mGeometry = *dynamic_cast<GeometryHandle*>(handle);
        break;
      case H_ANNOTATION:
        mAnnotation = *dynamic_cast<AnnotationHandle*>(handle);
        break;
      case H_LOCATION:
        mLocation = *dynamic_cast<LocationHandle*>(handle);
        break;
      default:
        sterror(true,"Unexpected node type.");
        break;
    }

    delete handle;
  }

  return 1;
}

int SegmentationHandle::parseXMLInternal(const XMLNode& node)
{
  FileHandle::parseXMLInternal(node);

  if (node.getAttribute("domain",0) == NULL) {
    fprintf(stderr,"Warning: no \"domain\" attribute found in segmentation handle.");
    mDomainType = UNDEFINED_DOMAIN;
  }
  else {
    mDomainType = (DomainType)atoi(node.getAttribute("domain",0));
  }

  if (node.getAttribute("domain-description",0) == NULL)
    fprintf(stderr,"Warning: no \"domain-description\" attribute found in segmentation handle.");
  else
    mDomainDescription = std::string(node.getAttribute("domain-description",0));

  if (node.getAttribute("encoding",0) == NULL)
    fprintf(stderr,"Could not find required \"encoding\" attribute for segmentation handle.\n");
  else {
    if (strcmp(node.getAttribute("encoding",0),"binary") == 0)
      mASCIIFlag = false;
    else if (strcmp(node.getAttribute("encoding",0),"ascii") == 0)
      mASCIIFlag = true;
    else
      fprintf(stderr,"Warning: \"encoding\" attribute should be either \"ascii\" or \"float\".\n");
  }

  if (node.getAttribute("featurecount",0) == NULL) {
    fprintf(stderr,"Warning: no \"featurecount\" attribute found in segmentation handle.");
    mFeatureCount = 0;
  }
  else {
    std::stringstream input(std::string(node.getAttribute("featurecount",0)));
    input >> mFeatureCount;
  }

  return 1;
}

int SegmentationHandle::attachXMLInternal(XMLNode& node) const
{
  XMLNode child;
  FileHandle::attachXMLInternal(node);

  node.addAttribute("domain",mDomainType);
  node.addAttribute("domain-description",mDomainDescription.c_str());

  if (mASCIIFlag)
    node.addAttribute("encoding","ascii");
  else
    node.addAttribute("encoding","binary");

  node.addAttribute("featurecount",mFeatureCount);

  if (mIndex.elementCount() > 0) {
     XMLNode child;

     child = node.addChild(mIndex.name());
     mIndex.attachXMLInternal(child);
  }

  if (mGeometry.elementCount() > 0) {

     child = node.addChild(mGeometry.name());
     mGeometry.attachXMLInternal(child);
  }

  if (mAnnotation.elementCount() > 0) {

     child = node.addChild(mAnnotation.name());
     mAnnotation.attachXMLInternal(child);
  }

  if (mLocation.elementCount() > 0) {

     child = node.addChild(mLocation.name());
     mLocation.attachXMLInternal(child);
  }

  return 1;
}

int SegmentationHandle::writeData(std::ofstream& output, const std::string& filename)
{
  LocalIndexType count = 0;
  LocalIndexType i;
  std::vector<GlobalIndexType>::iterator it;

  this->mFileName = filename;
  this->mOffset = static_cast<FileOffsetType>(output.tellp());

  //! If there is no data to write
  if (mSegmentation != NULL) {

    if (mASCIIFlag) {
      for (i=0;i<mSegmentation->size();i++) {

        output << count << std::endl;
        count += (*mSegmentation)[i].size();
      }
      output << count << std::endl;

      for (i=0;i<mSegmentation->size();i++) {
        for (it=(*mSegmentation)[i].begin();it!=(*mSegmentation)[i].end();it++) {
          output << *it << " ";
        }
        output << std::endl;
      }
    }
    else {
      for (i=0;i<mSegmentation->size();i++) {

        output.write((const char*)&count,sizeof(LocalIndexType));
        count += (*mSegmentation)[i].size();
      }
      output.write((const char*)&count,sizeof(LocalIndexType));
    

      for (i=0;i<mSegmentation->size();i++)
        output.write((const char*)&(*mSegmentation)[i][0],
                     sizeof(GlobalIndexType)*(*mSegmentation)[i].size());

      //output.write((const char*)(mCoordinates->size()/3),sizeof(LocalIndexType));
      //output.write((const char*)&(*mCoordinates)[0], sizeof(FunctionType)*mCoordinates->size());
    }
  }
  else if ((mFlatSegmentation != NULL) && (mOffsets != NULL)) {

    if (mASCIIFlag) {
      for (i=0;i<=mFeatureCount;i++)
        output << (*mOffsets)[i] << std::endl;

      for (i=0;i<mFeatureCount;i++) {
        for (LocalIndexType k=(*mOffsets)[i];k<(*mOffsets)[i+1];k++)
          output << (*mFlatSegmentation)[k] << " ";
        output << std::endl;
      }
    }
    else {
      output.write((const char*)&(*mOffsets)[0],sizeof(LocalIndexType)*(mFeatureCount+1));
      output.write((const char*)&(*mFlatSegmentation)[0],sizeof(GlobalIndexType)*mFlatSegmentation->size());
    }
  }

  // If necessary write the geometry data
  if (mGeometry.elementCount() > 0)
    mGeometry.writeData(output,this->mFileName);

  // If necessary write the annotation data
  if (mAnnotation.elementCount() > 0)
    mAnnotation.writeData(output,this->mFileName);

  // If necessary write the location data
  if (mLocation.elementCount() > 0)
    mLocation.writeData(output,this->mFileName);

  return 1;
}

}
