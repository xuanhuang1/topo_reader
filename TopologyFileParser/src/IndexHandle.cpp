#include "IndexHandle.h"

namespace TopologyFileFormat {

/*
void IndexHandle::setData(LocalIndexType* indices, LocalIndexType count)
{
  mData = (char*)indices;
  mElementCount = count;
}


void IndexHandle::readData(LocalIndexType* data)
{
  std::ifstream file;

  openInputFile(this->mFileName,file);

    // Reset the file pointer to start reading
  rewind(file);

  LocalIndexType i; 
 
  if (mASCIIFlag) {
    for (i=0;i<mElementCount;i++) 
      file >> data[i];
  }
  else {
    file.read((char *)data,sizeof(LocalIndexType)*mElementCount);
  }    

  file.close();
}
*/
int IndexHandle::parseXML(const XMLNode& node)
{
  return parseXMLInternal(node);
}

}
