#include "DataHandle.h"

namespace TopologyFileFormat {

DataHandle::DataHandle(HandleType t) 
  : FileHandle(t), mData(NULL), mElementCount(0), mASCIIFlag(true)
{
}

DataHandle::DataHandle(const char* filename, HandleType t) 
  :  FileHandle(filename,t), mData(NULL), mElementCount(0),
     mASCIIFlag(true)
{
}

DataHandle::DataHandle(const DataHandle& handle)
  : FileHandle(handle), mData(handle.mData), mElementCount(handle.mElementCount),
    mASCIIFlag(handle.mASCIIFlag)
{
}
 
DataHandle::~DataHandle()
{
}

DataHandle& DataHandle::operator=(const DataHandle& handle)
{
  FileHandle::operator=(handle);

  mData = handle.mData;
  mElementCount = handle.mElementCount;
  mASCIIFlag = handle.mASCIIFlag;

  return *this;
}

void DataHandle::setData(FileData* data)
{
  mData = data;
  mElementCount = data->size();
}


void DataHandle::readData(FileData* data) const
{
  std::ifstream file;

  openInputFile(mFileName,file,!mASCIIFlag);
  rewind(file);

  if (mASCIIFlag)
    data->readASCII(file);
  else {
    data->readBinary(file);
    //std::cout << "read binary" << std::endl;
  }
 


  file.close();
}



void DataHandle::clear()
{
  FileHandle::clear();

  mData = NULL;
  mElementCount = 0;
  mASCIIFlag = true;
}

int DataHandle::parseXMLInternal(const XMLNode& node)
{
  FileHandle::parseXMLInternal(node);

  if (node.getAttribute("encoding",0) == NULL)
    fprintf(stderr,"Could not find required \"encoding\" attribute for file handle.\n");
  else {
    if (strcmp(node.getAttribute("encoding",0),"binary") == 0)
      mASCIIFlag = false;
    else if (strcmp(node.getAttribute("encoding",0),"ascii") == 0)
      mASCIIFlag = true;
    else 
      fprintf(stderr,"Warning: \"encoding\" attribute should be either \"ascii\" or \"float\".\n");
  }

  if (node.getAttribute("elementcount",0) == NULL)
    fprintf(stderr,"Could not find required \"elementcount\" attribute for file handle.\n");
  else {
    mElementCount = (LocalIndexType) atoi(node.getAttribute("elementcount",0));
  }

  if (node.getAttribute("offset",0) == NULL)
    fprintf(stderr,"Could not find required \"offset\" attribute for file handle.\n");
  else {
    mOffset = (FileOffsetType) atoi(node.getAttribute("offset",0));
  }
  

  return 1;
}

int DataHandle::attachXMLInternal(XMLNode& node) const
{   
  FileHandle::attachXMLInternal(node);

  if (mASCIIFlag)
    node.addAttribute("encoding","ascii");
  else
    node.addAttribute("encoding","binary");

  node.addAttribute("elementcount",mElementCount);

  return 1;
}


int DataHandle::writeData(std::ofstream& output,const std::string& filename)
{
  this->mFileName = filename;

  mOffset = static_cast<FileOffsetType>(output.tellp());

  // If there is no data to write
  if (mData == NULL)
    return 1; // There is nothing to do
  
   if (mASCIIFlag)
    mData->writeASCII(output);
  else
    mData->writeBinary(output);
  return 1;
}

}

