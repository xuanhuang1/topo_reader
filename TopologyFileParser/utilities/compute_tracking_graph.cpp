#include "FamilyCorrelation.h"
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <sstream>

#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SimplificationHandle.h"
#include "FeatureElement.h"
#include "FileData.h"
#include "EdgeHandle.h"
#include "HandleCollection.h"

using namespace TopologyFileFormat;

std::string filename;

int main(int argc, const char *argv[])
{

if (argc < 2) {
    fprintf(stderr,"Usage: %s <family0> <seg0>.... <familyN> <segN>\n",argv[0]);
    exit(0);
  }



 //! The meta data store
  TopologyFileFormat::HandleCollection collection;

  // The set of all clan keys we have found
  std::vector<TopologyFileFormat::ClanKey> clans;

  //! The set of available time indices for each clan
  std::map<TopologyFileFormat::ClanKey, std::set<int> > timesteps;
  collection.initialize(&argv[1], argc-1);

  collection.getClanKeys(clans);

  std::vector<TopologyFileFormat::FamilyKey> keys;
  std::vector<TopologyFileFormat::FamilyKey>::iterator kIt;
  std::vector<TopologyFileFormat::ClanKey>::iterator it;

  std::vector<TopologyFileFormat::AssociationKey> aKeys;

    
    
  for (it=clans.begin();it!=clans.end();it++) {

    collection.getFamilyKeys(*it,keys);
    collection.getAssociationKeys(*it,aKeys);
    //break;

    // AssociationHandle association =

    for (kIt=keys.begin();kIt!=keys.end()-1;kIt++){
      	//t.insert(kIt->timeIndex());
  		//std::cout<<kIt->timeIndex()<<std::endl;

  		FamilyHandle family1 = collection.findFamilyHandle(*kIt);

  		//kIt++; //get next family
  		FamilyHandle family2 = collection.findFamilyHandle(*(kIt+1));

  		//note for monday: get the heigherarchy/segmentation and then pass that into the compute correlation. then attatch edge handle to asscociation handle and update clan+
		for (int i=0;i<family1.numSimplifications();i++) {

			filename=collection.getFileFromFamily(*kIt);
			
			//std::cout<<"dataset is "<<dataset<<std::endl;
			filename=filename.substr(0,filename.find(".fam"))+ std::string(".family");

  			// std::string filename= family1.mFileName;
  			std::cout<<"attatching clan to "<<filename<<std::endl;
  			ClanHandle clan;
  			clan.attach(filename);

  			//clan.dataset(dataset);
  			//clan.write();


      		SimplificationHandle handle1 = family1.simplification(i);
      		//std::cout<<"num simps is "<<family1.numSimplifications()<<std::endl;
			Data<FeatureElement> data;
			handle1.readData(data);
	  		// Instantiate the corresponding hierarchy
			TopologyFileFormat::FeatureHierarchy* hierarchy1 = new TopologyFileFormat::FeatureHierarchy();
			hierarchy1->initialize(handle1);

			SegmentationHandle seg1 = family1.segmentation();
			TopologyFileFormat::FeatureSegmentation* segmentation1 = new TopologyFileFormat::FeatureSegmentation();
			segmentation1->initialize(seg1);
		  	
		  	for (int j=0;j<family2.numSimplifications();j++) {
			  		SimplificationHandle handle2 = family2.simplification(j);

		  		// Instantiate the corresponding hierarchy
				TopologyFileFormat::FeatureHierarchy* hierarchy2 = new TopologyFileFormat::FeatureHierarchy();
				hierarchy2->initialize(handle2);

				SegmentationHandle seg2 = family2.segmentation();
				TopologyFileFormat::FeatureSegmentation* segmentation2 = new TopologyFileFormat::FeatureSegmentation();
				segmentation2->initialize(seg2);

				//now construct association handle

				AssociationHandle association;

				SimplificationKey sk1(handle1, *kIt);
				SimplificationKey sk2(handle2, *(kIt+1));

				association.source(sk1);
				association.destination(sk2);

				
				AssociationKey a_key(sk1,sk2);
				std::cout<<"for association key "<<a_key.source().familyKey().timeIndex()<<" "<<a_key.destination().familyKey().timeIndex()<<"there are this many associations: "<<clan.numAssociations()<<std::endl;
    			if(clan.numAssociations()==0) {


					ComputeFamilyCorrelation* correlation= new ComputeFamilyCorrelation(hierarchy1,hierarchy2,segmentation1,segmentation2, VERTEX_OVERLAP);
					correlation->computeCorrelations();
									//edge handle
					EdgeHandle edge;
					edge.setData(correlation);
					edge.encoding(true);
					//edge.setTrackingType(VERTEX_OVERLAP);
				
					ComputeFamilyCorrelation* correlationdist = new ComputeFamilyCorrelation(hierarchy1, hierarchy2, segmentation1, segmentation2, DISTANCE,3);
					correlationdist->computeCorrelations();
					//edge handle
					EdgeHandle edgedist;
					edgedist.setData(correlationdist);
					edgedist.encoding(true);
					//edge.setTrackingType(DISTANCE);


					std::cout<<"writing to file"<<std::endl;
					association.add(edge);
					association.add(edgedist);
					//clan.append(association);

					std::ofstream ofs;
					ofs.open(filename, std::ofstream::out | std::ofstream::trunc);
					ofs.close();
					ClanHandle clanw(filename);
					clanw.dataset("Rebika-SPT");

					// Create the family which is the internal name for the collection
					// of all the information related to a time step
					FamilyHandle family;

					// The index of this time step
					family.timeIndex(family1.timeIndex());

					// The actual floating point real time. Often this is identical
					// to the index but it does not have to be. Also this allows to
					// accept time series with unevenly distributed time steps
					family.time(family1.time());

					// The name of the threshold parameter we are using
					family.variableName("ParameterName");

					// The range of the parameter
					family.range(0, 1);
					SimplificationHandle simplification;
					simplification.setData(&data);
					simplification.metric("Threshold");
					simplification.fileType(SINGLE_REPRESENTATIVE);
					simplification.setRange(0, 1);
					simplification.encoding(true); // For testing purposes we write this in ascii
					clanw.add(family);
					clanw.family(0).add(simplification);
					clanw.add(association);
					clanw.write();
					
					//ClanHandle tclan;
					//tclan.attach(filename);


				}
		  	}

		  }
	//////////////////////uncomment for double checking written data//////////////////////////////
	 		//ClanHandle handle;
	 		//handle.attach(filename);
	 		//std::cout << " there are this many associations for this file " << handle.numAssociations() << std::endl;

	 		//if (handle.numAssociations() > 0) {
	 		//	AssociationHandle a = handle.association(0);
	 		//	std::cout << "for association key " << a.source().familyKey().timeIndex() << " " << a.destination().familyKey().timeIndex() << " and filename is " << a.getFileName() << std::endl;
	 		//	std::cout << "loaded association handle" << std::endl;
	 		//	EdgeHandle e;
	 		//	e = a.edges();
				///*TopologyFileFormat::FamilyCorrelation* corr = new TopologyFileFormat::FamilyCorrelation();
				//corr->initialize(e);*/
	 		//	std::cout << "loaded edge handle with feat count " << e.featureCount() << " and filename " << e.getFileName() << std::endl;
	 		//	std::vector<LocalIndexType> offsets;
	 		//	e.readOffsets(offsets);
	 		//	std::cout << "offset size is " << offsets.size() << " "<< offsets[offsets.size()-1]<<std::endl;
	 		//	std::vector<LocalIndexType> neighbors;
	 		//	e.readNeighbors(neighbors);
	 		//	std::cout << "neighbor size is " << neighbors.size() << std::endl;
				//std::vector<double> weights;

				//e.readWeights(weights);
				//std::cout << "weights size is " << weights.size() << std::endl;
				//std::cout << "//////////////////////////////" << std::endl;
	 		//}
		}
	}
	  		
  return 1;
}