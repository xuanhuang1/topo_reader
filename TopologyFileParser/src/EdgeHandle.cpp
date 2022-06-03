#include "EdgeHandle.h"

namespace TopologyFileFormat {

///////////////////////////////////////////////////////////////////////////////
 std::ostream& operator<<(std::ostream& out, const TrackingType& edge)
 {
   out << edge;

   return out;
 }


 ///////////////////////////////////////////////////////////////////////////////
 std::istream& operator>>(std::istream& in, TrackingType& edge)
 {
	 in >> edge;

   return in;
 }

///////////////////////////////////////////////////////////////////////////////
EdgeHandle::EdgeHandle() : FileHandle(H_EDGE), mASCIIFlag(false), mFeatureCount(0), arrOffsets(NULL), mNeighbors(NULL), mWeights(NULL){
	
}

///////////////////////////////////////////////////////////////////////////////
EdgeHandle::EdgeHandle(const char* filename) : FileHandle(filename,H_EDGE), mASCIIFlag(false), mFeatureCount(0),arrOffsets(NULL), mNeighbors(NULL), mWeights(NULL) {
	
}


EdgeHandle::EdgeHandle(const EdgeHandle& handle) : FileHandle(handle) {
	mFeatureCount=handle.mFeatureCount;
	mASCIIFlag=handle.mASCIIFlag;
	mNeighbors = handle.mNeighbors;
	mWeights=handle.mWeights;
	arrOffsets = handle.arrOffsets;
	tracking_type = handle.tracking_type;
}


///////////////////////////////////////////////////////////////////////////////
EdgeHandle::~EdgeHandle() {}


EdgeHandle& EdgeHandle::operator=(const EdgeHandle& handle)
{
  FileHandle::operator=(handle);

  mASCIIFlag = handle.mASCIIFlag;
  mFeatureCount = handle.mFeatureCount;
  mNeighbors = handle.mNeighbors;
  arrOffsets = handle.arrOffsets;
  mWeights=handle.mWeights;
  tracking_type = handle.tracking_type;


  return *this;
}


///////////////////////////////////////////////////////////////////////////////
void EdgeHandle::setData(ComputeFamilyCorrelation* data) {
	//std::cout<<"setting edge data, with feat size "<<data->getFirstHierarchySize()+1<<" and offset size is "<<arrOffsets->size()<<std::endl;
	tracking_type = data->tracking_type;
	arrOffsets=new std::vector<LocalIndexType>;
	std::vector<LocalIndexType>* n= new std::vector<LocalIndexType>;
	std::vector<double>* w= new std::vector<double>;

	arrOffsets->resize(data->getFirstHierarchySize()+1);
	LocalIndexType offset_count=0;
	for(const auto& src: data->mEdges){
		// std::cout<<"setting "<<src.first<<"to "<<offset_count<<std::endl;
		arrOffsets->at(src.first)=offset_count;
		for(const auto& edge: src.second){
		//	 std::cout<<"for "<<src.first<< " adding "<<edge<<std::endl;
			n->push_back(edge);
			w->push_back(data->mWeights[src.first][edge]);
			offset_count++;
		}
  	}
	mFeatureCount = arrOffsets->size()-1;
	arrOffsets->at(mFeatureCount)=offset_count;
	//arrOffsets = new std::vector<LocalIndexType>;
	mNeighbors = new std::vector<LocalIndexType>;
	mNeighbors->resize(n->size());

	mWeights = new std::vector<double>;
	mWeights->resize(w->size());

	 offset_count = 0;
	for (const auto& src : data->mEdges) {
		// std::cout<<"setting "<<src.first<<"to "<<offset_count<<std::endl;
		//arrOffsets->at(src.first) = offset_count
		for (const auto& edge : src.second) {
			//	 std::cout<<"for "<<src.first<< " adding "<<edge<<std::endl;
			mNeighbors->at(offset_count)=edge;
			mWeights->at(offset_count)=data->mWeights[src.first][edge];
			offset_count++;
		}
	}
	std::cout<<" neighbor size is "<<mNeighbors->size()<<" and offset size is "<<arrOffsets->size()<<" and offest_count is " <<offset_count<<std::endl;	
}

///////////////////////////////////////////////////////////////////////////////
int EdgeHandle::writeData(std::ofstream& output, const std::string& filename) 
{
	LocalIndexType count = 0;
	LocalIndexType i;
	this->mFileName = filename;
	mOffset = static_cast<FileOffsetType>(output.tellp());
	std::cout << "offset at write is " << mOffset << std::endl;
	if ((mNeighbors != NULL) && (arrOffsets != NULL)) {

	    if (mASCIIFlag) {
			Data<LocalIndexType> n(mNeighbors);
			Data<double> w(mWeights);
			Data<LocalIndexType> o(arrOffsets);
			o.writeASCII(output);
			n.writeASCII(output);
			w.writeASCII(output);




	     /* for (i=0;i<=mFeatureCount;i++)
	        output << (*arrOffsets)[i] << std::endl;

	      for (i=0;i<mFeatureCount;i++) {
	        for (LocalIndexType k=(*arrOffsets)[i];k<(*arrOffsets)[i+1];k++)
	          output << (*mNeighbors)[k] << " ";
	        output << std::endl;
	      }
		  for (i = 0; i < mFeatureCount; i++) {
			  for (LocalIndexType k = (*arrOffsets)[i]; k < (*arrOffsets)[i + 1]; k++)
				  output << (*mWeights)[k] << " ";
			  output << std::endl;
		  }*/
	    }
	    else {
			//std::cout << "writing binary data: " << sizeof(LocalIndexType) << " * " << mFeatureCount + 1 << " = " << sizeof(LocalIndexType) * (mFeatureCount + 1) << std::endl;
			      //rite((const char*)&(*mOffsets)[0], sizeof(LocalIndexType) * (mFkeatureCount + 1));
		 auto local = static_cast<FileOffsetType>(output.tellp());
	      output.write((char*)&(*arrOffsets)[0],sizeof(LocalIndexType)*(arrOffsets->size()));
		   local = static_cast<FileOffsetType>(output.tellp());
		   std::cout << local << std::endl;
	      output.write((char*)&(*mNeighbors)[0],sizeof(LocalIndexType)*mNeighbors->size());
		   local = static_cast<FileOffsetType>(output.tellp());
		   std::cout << local << std::endl;

		  output.write((const char*)&(*mWeights)[0],sizeof(double)*mWeights->size());
		   local = static_cast<FileOffsetType>(output.tellp());
		   std::cout << local << std::endl;

    	}
  	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
int EdgeHandle::parseXML(const XMLNode& node) {
	FileHandle* handle;

	parseXMLInternal(node);

	return 1;

}


///////////////////////////////////////////////////////////////////////////////
int EdgeHandle::readOffsets(std::vector<LocalIndexType>& offsets){
	std::ifstream file;
	openInputFile(mFileName,file,!mASCIIFlag);
	rewind(file);

	offsets.resize(mFeatureCount+1);

	Data<LocalIndexType> off(&offsets);

	if (mASCIIFlag)
		off.readASCII(file);
	else
		file.read((char*)&(offsets)[0], sizeof(LocalIndexType) * offsets.size());
		//off.readBinary(file);

	file.close();
	return 1;
}

int EdgeHandle::readNeighbors(std::vector<LocalIndexType>& _neighbors){
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
	//size=mFeatureCount;
	_neighbors.resize(size);
	Data<LocalIndexType> n(&_neighbors);

	// Read the rest of the data
	if (mASCIIFlag)
		n.readASCII(file);
	else
		n.readBinary(file);

	file.close();
	return 1;
}


int EdgeHandle::readWeights(std::vector<double>& _weights){
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
	//size=mFeatureCount;
	_weights.resize(size);

	Data<double> w(&_weights);
	
	if(mASCIIFlag)
		for (LocalIndexType i = 0; i < _weights.size(); i++)
			file >> size;
	else
		file.seekg(size *sizeof(LocalIndexType),std::ios_base::cur);

	// Read the rest of the data
	if (mASCIIFlag)
		w.readASCII(file);
	else
		w.readBinary(file);

	file.close();
	return 1;
}

int EdgeHandle::parseXMLInternal(const XMLNode& node){
	FileHandle::parseXMLInternal(node);

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
	if (node.getAttribute("trackingtype", 0) == NULL) {
		fprintf(stderr, "Warning: no \"trackingtype\" attribute found in segmentation handle.");
		//tracking_type = NULL;
	}
	else {
		std::stringstream input(std::string(node.getAttribute("trackingtype", 0)));
		std::string temp;
		input >> temp;
		if (temp == "VERTEX_OVERLAP")
			tracking_type = VERTEX_OVERLAP;
		if (temp == "DISTANCE")
			tracking_type = DISTANCE;
		if(temp=="GIVEN")
			tracking_type=GIVEN;
	}
  return 1;
}

//! Add the local attribute to the node
int EdgeHandle::attachXMLInternal(XMLNode& node) const{
	XMLNode child;
	FileHandle::attachXMLInternal(node);


	if (mASCIIFlag)
		node.addAttribute("encoding","ascii");
	else
		node.addAttribute("encoding","binary");

	node.addAttribute("featurecount",mFeatureCount);
	switch (tracking_type) {
		case VERTEX_OVERLAP:
			node.addAttribute("trackingtype", "VERTEX_OVERLAP");
			break;
		case DISTANCE:
			node.addAttribute("trackingtype", "DISTANCE");
			break;
		case GIVEN:
			node.addAttribute("trackingtype", "GIVEN");
			break;
		default:
			node.addAttribute("trackingtype", "");

	}


  return 1;
}











} //namespace TopologyFileFormat