#include <string>
#include <iostream>
#include <errno.h>

#include "ClanHandle.h"

namespace TopologyFileFormat {

const std::string ClanHandle::sDefaultName = "Simulation";

ClanHandle::ClanHandle() : FileHandle(H_CLAN),
    mDataset(sDefaultName), mMajor(sMajorVersion),mMinor(sMinorVersion)
{
}

ClanHandle::ClanHandle(const char* filename) : FileHandle(filename,H_CLAN), mDataset(sDefaultName), mMajor(sMajorVersion),mMinor(sMinorVersion)
{
}

ClanHandle::ClanHandle(std::string& filename) : FileHandle(filename.c_str(),H_CLAN), mDataset(sDefaultName), mMajor(sMajorVersion),mMinor(sMinorVersion)
{
}

ClanHandle::ClanHandle(const ClanHandle& handle) 
  : FileHandle(handle), mDataset(handle.mDataset), mMajor(handle.mMajor),
    mMinor(handle.mMinor)
{
  std::vector<FamilyHandle>::iterator it;
  std::vector<AssociationHandle>::iterator it2;

  mFamilies = handle.mFamilies;
  for (it=mFamilies.begin();it!=mFamilies.end();it++)
    it->topHandle(this);

  mAssociations = handle.mAssociations;
  for (it2=mAssociations.begin();it2!=mAssociations.end();it2++)
    it2->topHandle(this);
}


ClanHandle& ClanHandle::operator=(const ClanHandle& handle)
{
  std::vector<FamilyHandle>::iterator it;
  std::vector<AssociationHandle>::iterator it2;

  FileHandle::operator=(handle);

  mFamilies = handle.mFamilies;
  for (it=mFamilies.begin();it!=mFamilies.end();it++)
    it->topHandle(this);

  mAssociations = handle.mAssociations;
  for (it2=mAssociations.begin();it2!=mAssociations.end();it2++)
    it2->topHandle(this);

  mMajor = handle.mMajor;
  mMinor = handle.mMinor;
  mDataset = handle.mDataset;

  return *this;
}

int ClanHandle::attach(const char* filename)
{
  FileOffsetType header_size;
  char* header;
  std::ifstream file;

  // Remove all the old information
  clear();

  mFileName = std::string(filename);

  if (!openInputFile(mFileName,file, std::ios_base::binary))
    return 0;

  // the cast is necessary as sizeof returns size_t and negating it results
  // in SIZE_MAX - sizeof(FileOffsetType) + 1
  file.seekg(-(std::streamoff)sizeof(FileOffsetType),std::ios_base::end);

  if (file.fail())
    return 0;

  // Read the offset
  file.read((char *)&this->mOffset,sizeof(FileOffsetType));
  
  if (file.fail())
    return 0;

  // Allocate a buffer to read the header
  header_size = static_cast<FileOffsetType>((FileOffsetType)file.tellg() - this->mOffset - sizeof(FileOffsetType));
  header = new char[header_size+1];

  // Point the file stream to the start of the header
  if (!rewind(file))
    return 0;
 
  // Read the header
  file.read((char *)header,header_size);


  if (file.fail())
    return 0;

  // Close the file
  file.close();

  // Make the header into a valid string
  header[header_size] = '\0';

  //fprintf(stderr,"Header:\nOffset: %d\n%s##########",mOffset,header);
  XMLNode root;
  XMLCSTR tag = NULL;
  XMLResults *results = NULL;


  root = XMLNode::parseString(header,tag,results);
  
  if (results != NULL) {
    delete[] header;
    return 0;
  }

  std::string str = root.getName();
  std::string str1 = name();
  sterror((strcmp(root.getName(),name()) != 0),"The topmost node of a topology file should be a clan node.");
  sterror(root.nChildNode(name(H_FAMILY)) > 1,"Feature clan with more than one family not yet supported.");

  parseXML(root);

  delete[] header;

  return 1;
}

void ClanHandle::write(const char* filename)
{
  std::vector<FamilyHandle>::iterator fIt;
  std::vector<AssociationHandle>::iterator aIt;

  if (filename != NULL)
    this->mFileName = std::string(filename);
  else {
    sterror(this->mFileName==sEmptyString,"No internal file name set. Need a file name to write to");
  }

  std::ofstream file;
  openOutputFile(this->mFileName,file,true); // For now we are not using std::ios::binary

  // Make sure that our ascii values have the desired precision
  file.precision(sPrecision);

  // Make sure that we use scientific notation
  std::scientific(file);

  // First we write all the data to the file
  for (fIt=mFamilies.begin();fIt!=mFamilies.end();fIt++) {
    fIt->writeData(file,this->mFileName);
  }
  
  // First we write all the data to the file
  for (aIt=mAssociations.begin();aIt!=mAssociations.end();aIt++) {
    aIt->writeData(file,this->mFileName);
  }

  // Rewrite the xml-footer with the new info included
  attachXMLFooter(file);

  file.close();
}

FileHandle& ClanHandle::add(const FileHandle& handle)
{
  switch (handle.type()) {
    case H_FAMILY:
      mFamilies.push_back(dynamic_cast<const FamilyHandle&>(handle));
      mFamilies.back().topHandle(this);
      return mFamilies.back();
      break;
    case H_ASSOC:
      mAssociations.push_back(dynamic_cast<const AssociationHandle&>(handle));
      mAssociations.back().topHandle(this);
      return mAssociations.back();
      break;
    default:
      sterror(true,"Unknown handle type cannot attach a %d to a clan",handle.type());
      break;
  }

  // Note that this is a bogus return to satisfy the compiler. You should hit
  // the error before coming to here
  return mFamilies.back();
}

void ClanHandle::updateMetaData(const char* filename)
{
  if (filename != NULL)
    this->mFileName = std::string(filename);
  else {
    sterror(this->mFileName==sEmptyString,"No internal file name set. Need a file name to write to");
  }

  std::ofstream file(this->mFileName.c_str(),std::ios::in | std::ios::out);
  sterror(file.fail(),"Could not open file \"%s\" with mode \"%s\". Got errno %d = \"%s\".\n",this->mFileName.c_str(),
          std::ios::in | std::ios::out,errno,strerror(errno));

  // Point the file to the current start of the header
  file.seekp(mOffset,std::ios_base::beg);

  // Rewrite the xml-footer with the new info included
  attachXMLFooter(file);

  file.close();
}



void ClanHandle::clear()
{
  FileHandle::clear();
 
  mFamilies.clear();
  mAssociations.clear();
  mMajor = sMajorVersion;
  mMinor = sMinorVersion;
}

void ClanHandle::appendData(FileHandle& handle)
{
  if (mFileName == sEmptyString)
    fprintf(stderr,"Cannot append data to file since ClanHandle is not attached yet.");

  std::ofstream file(this->mFileName.c_str(),std::ios::in | std::ios::out);
  sterror(file.fail(),"Could not open file \"%s\" with mode \"%s\". Got errno %d = \"%s\".\n",this->mFileName.c_str(),
          std::ios::in | std::ios::out,errno,strerror(errno));

  // Make sure that our ascii values have the desired precision
  file.precision(sPrecision);

  // Make sure that we use scientific notation
  std::scientific(file);

  // Point the file to the current start of the header
  file.seekp(mOffset,std::ios_base::beg);

  // Write the data of the new handle. This implicitly update the offset values
  handle.writeData(file,this->mFileName);

  // Rewrite the xml-footer with the new info included
  attachXMLFooter(file);

  file.close();
}

int ClanHandle::parseXML(const XMLNode& node)
{
  FileHandle* handle;
 
  parseXMLInternal(node);
 
  for (int i=0;i<node.nChildNode();i++) {
    
    handle = this->constructHandle(node.getChildNode(i).getName(),mFileName);
    handle->topHandle(this);

    switch (handle->type()) {
      case H_FAMILY:
        handle->parseXML(node.getChildNode(i));
        mFamilies.push_back(*dynamic_cast<FamilyHandle*>(handle));
        break;
      case H_ASSOC:
        handle->parseXML(node.getChildNode(i));
        mAssociations.push_back(*dynamic_cast<AssociationHandle*>(handle));
        break;
      default:
        sterror(true,"Invalid xml structure. A clan should not have a child of type %d",handle->type());
        break;
    }
    delete handle;
  }
  return 1;
}

int ClanHandle::parseXMLInternal(const XMLNode& node)
{
  FileHandle::parseXMLInternal(node);

  if (node.getAttribute("localsize",0) == NULL)
    fprintf(stderr,"Could not find \"localsize\" attribute for file handle.\n");
  else {
    int local = atoi(node.getAttribute("localsize",0));
    if (local != sizeof(LocalIndexType)) {
      fprintf(stderr,"Warning: localsize found in file is %d but the binary is compiled with %lu.\n",
              local,sizeof(LocalIndexType));
      assert(false);
    }
  }

  if (node.getAttribute("globalsize",0) == NULL)
    fprintf(stderr,"Could not find \"globalsize\" attribute for file handle.\n");
  else {
    int global = atoi(node.getAttribute("globalsize",0));
    if (global != sizeof(GlobalIndexType)) {
      fprintf(stderr,"Warning: globalsize found in file is %d but the binary is compiled with %lu.\n",
              global,sizeof(GlobalIndexType));
      assert(false);
    }
  }


  if (node.getAttribute("precision",0) == NULL)
    fprintf(stderr,"Could not find \"precision\" attribute for file handle.\n");
  else {
    if (strcmp(node.getAttribute("precision",0),"float") == 0) {
      if (sizeof(FunctionType) != 4) 
        fprintf(stderr,"Warning: File assumes precision 4 but binary is compiled with %lu\n",
                sizeof(FunctionType));   
    }
    else if (strcmp(node.getAttribute("precision",0),"double") == 0) {
      if (sizeof(FunctionType) != 8) 
        fprintf(stderr,"Warning: File assumes precision 8 but binary is compiled with %lu\n",
                sizeof(FunctionType));   
    }
    else {
      fprintf(stderr,"Warning: \"precision\" attribute should be either \"float\" or \"double\"\n");
    }
  }

  if (node.getAttribute("major",0) == NULL)
    fprintf(stderr,"Could not find required \"major\" attribute for file handle.\n");
  else {
    mMajor = (uint16_t) atoi(node.getAttribute("minor",0));
  }

  if (node.getAttribute("minor",0) == NULL)
    fprintf(stderr,"Could not find required \"minor\" attribute for file handle.\n");
  else {
    mMinor = (uint16_t) atoi(node.getAttribute("minor",0));
  }

  sterror(mMajor > sMajorVersion,"Version number missmatch. File needs version %d.%d but code is version %d.%d",mMajor,mMinor,
          sMajorVersion,sMinorVersion);
 
  sterror((mMajor==sMajorVersion) && (mMinor > sMinorVersion),"Version number missmatch. File needs version %d.%d but code is version %d.%d",mMajor,mMinor,
          sMajorVersion,sMinorVersion);


  if (node.getAttribute("Dataset",0) == NULL)
    mDataset = sDefaultName;
  else {
    mDataset = std::string(node.getAttribute("Dataset",0));
  }

  return 1;
}



int ClanHandle::attachXMLInternal(XMLNode& node) const
{
  XMLNode child;

  FileHandle::attachXMLInternal(node);

  node.addAttribute("globalsize",(uint32_t)sizeof(GlobalIndexType));
  node.addAttribute("localsize",(uint32_t)sizeof(LocalIndexType));
  
  if (sizeof(FunctionType) == 4)
    node.addAttribute("precision","float");
  else
    node.addAttribute("precision","double");
    
  node.addAttribute("major",mMajor);
  node.addAttribute("minor",mMinor);
  node.addAttribute("Dataset",mDataset.c_str());

  for (uint8_t i=0;i<mFamilies.size();i++) {

    child = node.addChild(mFamilies[i].name());
    mFamilies[i].attachXMLInternal(child);
  }

  for (uint32_t i=0;i<mAssociations.size();i++) {

    child = node.addChild(mAssociations[i].name());
    mAssociations[i].attachXMLInternal(child);
  }

  return 1;
}

int ClanHandle::attachXMLFooter(std::ofstream& file)
{
  // Record the offset as start of the header
  mOffset = file.tellp();

  // Then we create the xml hierarchy
  XMLNode clan;

  clan = XMLNode::createXMLTopNode(name());
  attachXMLInternal(clan);

  // Create the corresponding header string
  XMLSTR header = clan.createXMLString();

  file << header;

  //fprintf(stderr,"\nOffset: %d\n%s#########\n",mOffset,header);

  FileOffsetType o = static_cast<uint64_t>(mOffset);

  file.write((char*)&o,sizeof(FileOffsetType));

  return 1;
}

}
