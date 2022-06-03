#include <errno.h>
#include <iostream>
#include <string.h>

#include "FileHandle.h"
#include "SimplificationHandle.h"
#include "FamilyHandle.h"
#include "IndexHandle.h"
#include "AssociationHandle.h"
#include "EdgeHandle.h"
#include "SegmentationHandle.h"

namespace TopologyFileFormat {

const char* gHandleNames[HANDLE_COUNT] = {
  "Clan",
  "Family",
  "Simplification",
  "Index",
  "Statistic",
  "Association",
  "Edge",
  "Segmentation",
  "Geometry",
  "Annotation",
  "Location"
};

const std::string FileHandle::sEmptyString = std::string("");

const char* FileHandle::name(HandleType t)
{
  sterror(t==H_UNDEFINED,"Handle type not set cannot determine name.");

  return gHandleNames[t];
}


FileHandle* FileHandle::constructHandle(const char* name, const std::string& filename)
{
  return constructHandle(name,filename.c_str());
}


FileHandle* FileHandle::constructHandle(const char* name,const char* filename)
{
  //fprintf(stderr,"FileHandle::constructHandle \"%s\"\n",name);
  if (strcmp(name,gHandleNames[H_FAMILY]) == 0)
    return new FamilyHandle(filename);
  else if (strcmp(name,gHandleNames[H_SIMPLIFICATION]) == 0)
    return new SimplificationHandle(filename);
  else if (strcmp(name,gHandleNames[H_STAT]) == 0)
    return new StatHandle(filename);
  else if (strcmp(name,gHandleNames[H_INDEX]) == 0)
    return new IndexHandle(filename);
  else if (strcmp(name,gHandleNames[H_ASSOC]) == 0)
    return new AssociationHandle(filename);
  else if (strcmp(name,gHandleNames[H_EDGE]) == 0)
    return new EdgeHandle(filename);
  else if (strcmp(name,gHandleNames[H_SEGMENTATION]) == 0)
    return new SegmentationHandle(filename);
  else if (strcmp(name,gHandleNames[H_GEOMETRY]) == 0)
    return new GeometryHandle(filename);
  else if (strcmp(name,gHandleNames[H_ANNOTATION]) == 0)
    return new AnnotationHandle(filename);
  else if (strcmp(name,gHandleNames[H_LOCATION]) == 0)
    return new LocationHandle(filename);

  sterror(true,"Unkown handle name \"%s\".",name);
  
  return NULL;
}


FileHandle::FileHandle(HandleType t)
  : mType(t), mFileName(""), mOffset(0), mTopHandle(NULL)
{
}

FileHandle::FileHandle(const char* filename, HandleType t)
  :  mType(t), mFileName(filename), mOffset(0), mTopHandle(NULL)
{
}

FileHandle::FileHandle(const FileHandle& handle) 
  :  mType(handle.mType), mFileName(handle.mFileName), mOffset(handle.mOffset),
     mTopHandle(handle.mTopHandle)
{
}
 
FileHandle::~FileHandle()
{
}

FileHandle& FileHandle::operator=(const FileHandle& handle)
{
  sterror(mType!=handle.mType,"Assignment between incompatible types.");
  mFileName = handle.mFileName;
  
  mOffset = handle.mOffset;

  mTopHandle = handle.mTopHandle;
  return *this;
}

const char* FileHandle::name() const
{
  sterror(mType==H_UNDEFINED,"Handle type not set cannot determine name.");

  return gHandleNames[mType];
}

FileHandle& FileHandle::add(const FileHandle& handle)
{
  sterror(true,"Unexpected node type.");

  return *this;
}

void FileHandle::clear()
{
  mFileName = std::string("");
  
  mOffset = 0;
  mTopHandle = NULL;
}

void FileHandle::appendData(FileHandle& handle)
{
  sterror(mType!=H_CLAN,"Sorry, append can only be called from a ClanHandle");
}


int FileHandle::parseXMLInternal(const XMLNode& node)
{
  if (node.getAttribute("offset",0) == NULL)
    fprintf(stderr,"Could not find required \"offset\" attribute for file handle.\n");
  else {
    mOffset = static_cast<std::streamoff>(atoi(node.getAttribute("offset",0)));
  }
  

  return 1;
}

int FileHandle::attachXMLInternal(XMLNode& node) const
{   
  node.addAttribute("offset",static_cast<uint64_t>(mOffset));

  return 1;
}

int FileHandle::rewind(std::ifstream& file) const
{
  if (!file.good()) {
    fprintf(stderr,"Cannot rewind NULL file pointer.");
    return 0;
  }
  
  file.seekg(mOffset,std::ios_base::beg);

  if (!file.good()) {
    fprintf(stderr,"Error rewinding the file.");
    return 0;
  }

  return 1;
}


int FileHandle::openOutputFile(const std::string& filename, std::ofstream& file,
                               bool binary) const
{
  return openOutputFile(filename.c_str(),file,binary);
}

int FileHandle::openOutputFile(const char* filename, std::ofstream& file,
                               bool binary) const
{
  if (strcmp(filename,"")==0) {
    fprintf(stderr,"Cannot open an empty file name \"\"");
    return 0;
  }
  
  if (binary)
    file.open(filename,std::ios::out | std::ios::binary);
  else
    file.open(filename,std::ios::out);


  if (file.fail()) {
    fprintf(stderr,"Could not open file \"%s\". Got errno %d = \"%s\".\n",filename,errno,strerror(errno));
    return 0;
  }

  return 1;
}

int FileHandle::openInputFile(const std::string& filename, std::ifstream& file, bool binary) const
{
  return openInputFile(filename.c_str(),file,binary);
}

int FileHandle::openInputFile(const char* filename, std::ifstream& file, bool binary) const
{
  if (strcmp(filename,"") == 0) {
    fprintf(stderr,"Cannot open an empty file name \"\"");
    return 0;
  }

  if (binary)
    file.open(filename, std::ios::in | std::ios::binary);
  else
    file.open(filename, std::ios::in);


  //std::cout << "file.fail() = " << file.fail() << std::endl;

  // JCB: crashes when you try to print mode as a string
  //sterror(true,"Could not open file \"%s\" with mode \"%s\". Got errno %d = \"%s\".\n",filename,mode,errno,strerror(errno));
  if (file.fail()) {
    fprintf(stderr,"Could not open file \"%s\" . Got errno %d = \"%s\".\n",filename,errno,strerror(errno));
    return 0;
  }

  return 1;
}
 
}

