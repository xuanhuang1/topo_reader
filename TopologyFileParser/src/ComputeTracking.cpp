#include "ComputeTracking.h"

namespace TopologyFileFormat {

void ComputeFamilyCorrelation::insertEdge(LocalIndexType srcId, LocalIndexType desId, double edgeWeight, double edgeWeightAcc, bool accu){
 	std::map<LocalIndexType,double>::iterator it;
 
 	mEdges[srcId].insert(desId);

	it= mWeights.at(srcId).find(desId);

	if(it == mWeights.at(srcId).end()) 
	{
		mWeights.at(srcId).insert(std::pair<LocalIndexType,double>(desId,edgeWeight));
		// mEdges.push_back(edge);
	}
	else
	{
		if(mAccumType==SUM) mWeights.at(srcId).at(desId)+=edgeWeight;

		else if(mAccumType==MIN) mWeights.at(srcId).at(desId) = std::min(mWeights.at(srcId).at(desId),edgeWeight);

		else if (mAccumType == MAX) mWeights.at(srcId).at(desId) = std::max(mWeights.at(srcId).at(desId), edgeWeight);

	}
	// it = edgesOfNodes.at(edge.srcId).find(edge.desId);
	
}

//! Accumulate edge weight over the hierarchy
void ComputeFamilyCorrelation::accumulateEdgeWeight(Feature* pSrc, Feature* pDes, bool bDir1, bool bDir2, double weight){
	if(bDir1) // merge tree
	{
	    while(pSrc->repSize()!=0 || pDes->repSize()!=0)
	    {
	      if(pSrc->lifeTime()[0] > pDes->lifeTime()[0]) // move up in merge tree for srcNode
	      {
	        if(pSrc->repSize()==0) break;
	        pSrc = pSrc->rep(0);
	      }
	      else if(pSrc->lifeTime()[0] < pDes->lifeTime()[0]) // move up in merge tree for desNode
	      {
	        if(pDes->repSize()==0) break;
	        pDes = pDes->rep(0);
	      }
	      else
	      {
	        if(pSrc->repSize()==0 && pDes->repSize()==0) break;
	        if(pSrc->repSize()>0) pSrc = pSrc->rep(0);
	        if(pDes->repSize()>0) pDes = pDes->rep(0);
	      }
	      
	      insertEdge(pSrc->id(), pDes->id(), weight, weight, true); 
	    }
	  }
	  else // split tree
	  {
	    while(pSrc->repSize()!=0 || pDes->repSize()!=0)
	    {  
	      if(pSrc->lifeTime()[1] > pDes->lifeTime()[1]) // move up in split tree for desNode
	      {
	        if(pDes->repSize()==0) break;
	        pDes = pDes->rep(0);
	      }
	      else if(pSrc->lifeTime()[1] < pDes->lifeTime()[1]) // move up in merge tree for srcNode
	      {
	        if(pSrc->repSize()==0) break;
	        pSrc = pSrc->rep(0);
	      }
	      else
	      {
	        if(pSrc->repSize()==0 && pDes->repSize()==0) break;
	        if(pSrc->repSize()>0) pSrc = pSrc->rep(0);
	        if(pDes->repSize()>0) pDes = pDes->rep(0);
	      }
	      insertEdge(pSrc->id(), pDes->id(), weight, weight, true); 
	    }
	 }
}

//! Get alive feature details for the given src and des
void ComputeFamilyCorrelation::getAliveIds(Feature* &prSrc, Feature* &prDes, bool bDir1, bool bDir2){ //TODO: take a closer look at this
	// TIMO : is representative always one?? can it have multiple representatives? 

	FunctionType birth_src = prSrc->lifeTime()[0];
	FunctionType death_src = prSrc->lifeTime()[1];

	FunctionType birth_des = prDes->lifeTime()[0];      
	FunctionType death_des = prDes->lifeTime()[1];

	if(bDir1) // merge tree
	{
		if(death_des<birth_src && death_des<death_src) // desNode before the lifetime of srcNode
		{
		  // go up the merge tree for srcNode
		  while(death_des<prSrc->lifeTime()[0] && death_des<prSrc->lifeTime()[1] && prSrc->repSize()>0)
		  { prSrc = prSrc->rep(0); }
		}
		else if(death_src<birth_des && death_src<death_des) // desNode after the lifetime of srcNode
		{
		  // go up the merge tree for desNode
		  while(death_src<prDes->lifeTime()[0] && death_src<prDes->lifeTime()[1] && prDes->repSize()>0)
		  { prDes = prDes->rep(0); }
		}
		}
		else // split tree
		{
		if(birth_src>birth_des && birth_src>death_des) // desNode before the lifetime of srcNode
		{
		  // go up the split tree for desNode
		  while(birth_src>prDes->lifeTime()[0] && birth_src>prDes->lifeTime()[1] && prDes->repSize()>0)
		  { prDes = prDes->rep(0); }
		}
		else if(birth_des>birth_src && birth_des>death_src) // desNode after the lifetime of srcNode
		{
		  // go up the split tree for srcNode
		  while(birth_des>prSrc->lifeTime()[0] && birth_des>prSrc->lifeTime()[1] && prSrc->repSize()>0)
		  { prSrc = prSrc->rep(0); }
		}
	}
}

bool getDirection(FeatureHierarchy*  &hierarchy){
	return (hierarchy->featureCount()>0) ? hierarchy->feature(0)->direction() : false;
}

//! Combine correlations to a edge list
void ComputeFamilyCorrelation::combineCorrelations(std::vector<HashTable> &rCorrelationMap){
	std::vector<Feature> features1 = hierarchy1->allFeatures(); 
	std::vector<Feature> features2 = hierarchy2->allFeatures();

	bool b_dir1 = getDirection(hierarchy1);
	std::cout<<"FEAT SIZE IS "<<features1.size();
	bool b_dir2 = getDirection(hierarchy2);

	// insert & accumulate edges 
	for(int i=0; i<(signed int)rCorrelationMap.size(); i++)
	{
		for(HashTable::iterator it = rCorrelationMap.at(i).begin(); it != rCorrelationMap.at(i).end(); it++) 
		{        
		  int src_id = i;
		  int des_id = it->first;    

		  Feature* p_src_feat = &features1.at(src_id);
		  Feature* p_des_feat = &features2.at(des_id);
		  
		  getAliveIds(p_src_feat, p_des_feat, b_dir1, b_dir2);
		  
		  src_id = p_src_feat->id();
		  des_id = p_des_feat->id();
		  
		  insertEdge(src_id, des_id, it->second, it->second, false);    
		  
		  accumulateEdgeWeight(p_src_feat, p_des_feat, b_dir1, b_dir2, it->second);      
		}
	}
}

HashTable ComputeFamilyCorrelation::generateHashTable(FeatureHierarchy* &rHierarchy, FeatureSegmentation* &rSegmentation){
	HashTable hash_table;

	std::vector<Feature> features =  rHierarchy->allFeatures();
	for(int i=0; i<(signed int)features.size(); i++)
	{
		Feature feat = features.at(i);

		TopologyFileFormat::Segment seg = rSegmentation->elementSegmentation(feat.id());
		for(LocalIndexType j=0; j<seg.size; j++)
		{
		  if(hash_table.find(seg.samples[j])==hash_table.end()) 
		  { hash_table[seg.samples[j]] = feat.id(); }
		}
	}

	return hash_table;
}

void ComputeFamilyCorrelation::initEdges(FeatureHierarchy* hierarchy){
	
	std::vector<Feature> features =  hierarchy->allFeatures();
	for(int i=0; i<(signed int)features.size(); i++)
	{
		mEdges[features[i].id()];
		mWeights[features[i].id()];

	}

}
int ComputeFamilyCorrelation::generateKdTreeAll(FeatureHierarchy*& rHierarchy, FeatureSegmentation*& rSegmentation, kdtree*& tree)
{

	tree = kd_create(3);
	int count = 0;

	std::vector<Feature> features = rHierarchy->allFeatures();
	for (int i = 0; i < (signed int)features.size(); i++)
	{
		Feature feat = features.at(i);

		TopologyFileFormat::Segment seg = rSegmentation->elementSegmentation(feat.id());
	
			Point3f point(0, 0, 0);
			if (seg.dim == 3) {
				for (uint64_t i = 0; i < seg.size; i++) {
					point = Point3f(seg.coordinates[3 * i], seg.coordinates[3 * i + 1], seg.coordinates[3 * i + 2]);
					std::pair< Point3f, LocalIndexType>p(point, feat.id());
					LocalIndexType id = feat.id();
					//	p.first.PrintFloat();
					std::pair<Point3f, LocalIndexType>* stored_data = (std::pair<Point3f, LocalIndexType>*)malloc(sizeof(std::pair<Point3f, LocalIndexType>));
					memcpy(stored_data, &p, sizeof(std::pair<Point3f, LocalIndexType>));
					int test = kd_insertf(tree, p.first.mData, stored_data);

					count++;
				}
			}
			else if (seg.dim == 2) {
				for (uint64_t i = 0; i < seg.size; i++) {
					point = Point3f(seg.coordinates[2 * i], seg.coordinates[2 * i + 1], 0);
					std::pair< Point3f, LocalIndexType>p(point, feat.id());
					LocalIndexType id = feat.id();
					//	p.first.PrintFloat();
					std::pair<Point3f, LocalIndexType>* stored_data = (std::pair<Point3f, LocalIndexType>*)malloc(sizeof(std::pair<Point3f, LocalIndexType>));
					memcpy(stored_data, &p, sizeof(std::pair<Point3f, LocalIndexType>));
					int test = kd_insertf(tree, p.first.mData, stored_data);

					count++;
					//  std::cout<<feature.box.p1<<" "<<feature.box.p2<<std::endl;

				}

			}
			
		
	}
	

	


	return 1;
}

void ComputeFamilyCorrelation::computeCorrelationsDistance() {

	std::vector<Feature> features1;
	std::vector<Feature> features2;
	if (parameter == NULL) {
		features1 = hierarchy1->allFeatures();
		features2 = hierarchy2->allFeatures();
	}
	else {
		FeatureHierarchy::ActiveIterator it;
		for (it = hierarchy1->beginActive(this->parameter); it != hierarchy1->endActive(); it++) {
			features1.push_back(**it);
		}
		for (it = hierarchy2->beginActive(this->parameter); it != hierarchy2->endActive(); it++) {
			features2.push_back(**it);
		}
		std::cout << "1 size is " << features1.size() << " 2 size is " << features2.size() << std::endl;
	}
	initEdges(hierarchy1);
	edgesOfNodes = std::vector<HashTable>(features1.size(), HashTable());
	// hash_table: 
	// Here, we create a hash table for timestep2
	//HashTable hash_table = generateHashTable(hierarchy2, segmentation2);

	// kd_tree: 
	// Here, we create a kdtree for timestep2
	kdtree* kd_tree;
	generateKdTreeAll(hierarchy2, segmentation2, kd_tree);
	

	// correlation_map: 
	// Here, we create a map for each element in the timestep1's hierarchy which stores the correlation details with the timestep2.
	// the key is the feature_id for timestep1, the value is the diatance of the timestep2_feature to the current timestep1_feature.  
	int k = 1;
	if (features1.size() > 0 && features2.size() > 0)
	{
		// get correlations
		std::vector<HashTable> correlation_map = ListMap(hierarchy1->allFeatures().size(), HashTable());
		for (int i = 0; i < (signed int)features1.size(); i++)
		{
			Feature feat1 = features1.at(i);
			TopologyFileFormat::Segment seg1 = segmentation1->elementSegmentation(feat1.id());

			for (LocalIndexType j = 0; j < seg1.size; j++)
			{
				FunctionType x = seg1.coordinates[seg1.dim * j + 0];
				FunctionType y = (seg1.dim > 1) ? seg1.coordinates[seg1.dim * j + 1] : 0;
				FunctionType z = (seg1.dim > 2) ? seg1.coordinates[seg1.dim * j + 2] : 0;
				double xyz[3];
				xyz[0] = x;
				xyz[1] = y;
				xyz[2] = z;

				if (radius <=0) {  //if radius not given, just get closest 

				//	std::pair<Point3f,LocalIndexType> return_data;

				//	double pt[3];
				//	kdres* res = kd_nearest(kd_tree, xyz);
				//	//	cout << "kd pt is " << pt[0]<<" "<< pt[1] << " " << pt[2] << endl;
				//	kd_res_item(res, pt);
				//	if (res->riter != nullptr) {
				//		return_data = *(std::pair<Point3f,LocalIndexType>*)kd_res_item_data(res); // data
				//		int i = 0;
				//	}
				//	kd_res_free(res);
				//	// get the nearest vertex (in the next timestep) for this vertex
				///*	
				//	kd_tree->annkSearch(point1, k, nnIdx, dists, 0);
				//	ANNpoint point2 = dataPts[nnIdx[0]];*/

				//	double dist = Point3f(x,y,z).dist(return_data.first);

				//	// find the feature (in the next timestep) of the nearest vertex
				//	LocalIndexType src_id = feat1.id();
				//	LocalIndexType des_id = return_data.second;

				//	// look to see if we have already overlapped with this element from grid 1          
				//	HashTable::iterator it = correlation_map.at(src_id).find(des_id);

				//	// if so then increment the count
				//	if (it != correlation_map.at(src_id).end())
				//	{
				//		if (it->second > dist) it->second = dist;
				//	}
				//	else // if not then insert it
				//	{
				//		correlation_map.at(src_id).insert(it, HashTable::value_type(des_id, dist));
				//	}
				}
				else {
					double pt[3];
					std::vector<std::pair<Point3f, LocalIndexType> > return_data;
					std::unordered_map<LocalIndexType, int> temp;
					kdres* res = kd_nearest_range(kd_tree, xyz, radius);
					//	cout << "kd pt is " << pt[0]<<" "<< pt[1] << " " << pt[2] << endl;
					kd_res_item(res, pt);
					if (res->riter != nullptr) {
						while (kd_res_end(res) == 0) {
							std::pair<Point3f, LocalIndexType> item = *(std::pair<Point3f, LocalIndexType>*)kd_res_item_data(res);
							return_data.push_back(item); // data
							kd_res_next(res);
							//i++;
						}
					}
					kd_res_free(res);
					// get the nearest vertex (in the next timestep) for this vertex
				/*
					kd_tree->annkSearch(point1, k, nnIdx, dists, 0);
					ANNpoint point2 = dataPts[nnIdx[0]];*/

					for (auto& pt : return_data) {
						std::unordered_map<LocalIndexType, int>::iterator it = temp.find(pt.second);
						if (it == temp.end()) {
							temp[pt.second] = 0;
						}
						temp[pt.second]++;
					}
					//std::cout << "res size is " << return_data.size() << "with this many indices found "<<temp.size()<<std::endl;
					for (auto& seg : return_data) {

						double dist =  double(Point3f(x, y, z).dist(seg.first));//we want 
						if (dist <= radius) {
							// find the feature (in the next timestep) of the nearest vertex
							LocalIndexType src_id = feat1.id();
							LocalIndexType des_id = seg.second;
							dist = mMaxDist - dist;
							// look to see if we have already overlapped with this element from grid 1          
							HashTable::iterator it = correlation_map.at(src_id).find(des_id);

							// if so then increment the count
							if (it != correlation_map.at(src_id).end())
							{
								if (it->second< dist) {
									it->second = dist;
									//std::cout << "at " << src_id << " to " << des_id << " updating dist to " << dist << std::endl;
								}
							}
							else // if not then insert it
							{
								//std::cout << "at " << src_id << " to " << des_id << " creating dist " << dist << std::endl;

								correlation_map.at(src_id).insert(it, HashTable::value_type(des_id, dist));

							}
						}
					}
					 
				}
			}
		}

		// combine correlations
		combineCorrelations(correlation_map);
	}
	kd_free(kd_tree);
	// re-assign edge weight
	/*for (auto& edge : mWeights){
		for (auto&w : mWeights.at(edge.first))
			mWeights.at(edge.first).at(w.first) = mMaxDist - mWeights.at(edge.first).at(w.first);
	}*/




}
void ComputeFamilyCorrelation::computeCorrelationsOverlap() {
	std::vector<Feature> features1;
		std::vector<Feature> features2;
	if (parameter == NULL) {
		features1= hierarchy1->allFeatures();
		features2= hierarchy2->allFeatures();
	}
	else {
		FeatureHierarchy::ActiveIterator it;
		for (it = hierarchy1->beginActive(this->parameter); it != hierarchy1->endActive(); it++) {
			features1.push_back(**it);
		}
		for (it = hierarchy2->beginActive(this->parameter); it != hierarchy2->endActive(); it++) {
			features2.push_back(**it);
		}
		std::cout << "1 size is " << features1.size() << " 2 size is " << features2.size() << std::endl;
	}

	initEdges(hierarchy1);
	std::cout << "Edges size is " << mEdges.size() << std::endl;
	edgesOfNodes = std::vector<HashTable>(features1.size(), HashTable());

	// hash_table: 
	// Here, we create a hash table for timestep2
	HashTable hash_table = generateHashTable(hierarchy2, segmentation2);
	std::cout << "Hash table created" << std::endl;

	// correlation_map: 
	// Here, we create a map for each element in the timestep1's hierarchy which stores the correlation details with the timestep2.
	// the key is the feature_id for timestep1, the value is the vertex_overlap of the timestep2_feature with the current timestep1_feature.  
	if (features1.size() > 0 && features2.size() > 0)
	{
		// get correlations
		std::vector<HashTable> correlation_map = ListMap(hierarchy1->allFeatures().size(), HashTable());
		for (int i = 0; i < (signed int)features1.size(); i++)
		{
			Feature feat1 = features1.at(i);
			TopologyFileFormat::Segment seg1 = segmentation1->elementSegmentation(feat1.id());

			for (LocalIndexType j = 0; j < seg1.size; j++)
			{
				HashTable::iterator it_hash_table = hash_table.find(seg1.samples[j]);
				if (it_hash_table != hash_table.end())
				{
					int src_id = feat1.id();
					int des_id = it_hash_table->second;

					// look to see if we have already overlapped with this element from grid 1          
					HashTable::iterator it = correlation_map.at(src_id).find(des_id);

					// if so then increment the count
					if (it != correlation_map.at(src_id).end())
					{
						(it->second)++;
					}
					else // if not then insert it
					{
						correlation_map.at(src_id).insert(it, HashTable::value_type(des_id, (double)1));
					}
				}
			}
		}
		std::cout << "Correlation map created" << std::endl;

		// combine correlations
		combineCorrelations(correlation_map);
		std::cout << "Correlations combined, " << mEdges.size() << " edges found" << std::endl;
		//constructEdgeMap();
	}

}
void ComputeFamilyCorrelation::computeCorrelationsGiven() {
	std::vector<Feature> features1;
	std::vector<Feature> features2;
	if (parameter == NULL) {
		features1 = hierarchy1->allFeatures();
		features2 = hierarchy2->allFeatures();
	}
	else {
		FeatureHierarchy::ActiveIterator it;
		for (it = hierarchy1->beginActive(this->parameter); it != hierarchy1->endActive(); it++) {
			features1.push_back(**it);
		}
		for (it = hierarchy2->beginActive(this->parameter); it != hierarchy2->endActive(); it++) {
			features2.push_back(**it);
		}
		std::cout << "1 size is " << features1.size() << " 2 size is " << features2.size() << std::endl;
	}

	initEdges(hierarchy1);
	std::cout << "Edges size is " << mEdges.size() << std::endl;
	edgesOfNodes = std::vector<HashTable>(features1.size(), HashTable());

	// hash_table: 
	// Here, we create a hash table for timestep2
	HashTable hash_table = generateHashTable(hierarchy2, segmentation2);
	std::cout << "Hash table created" << std::endl;

	// correlation_map: 
	// Here, we create a map for each element in the timestep1's hierarchy which stores the correlation details with the timestep2.
	// the key is the feature_id for timestep1, the value is the vertex_overlap of the timestep2_feature with the current timestep1_feature.  
	if (features1.size() > 0 && features2.size() > 0)
	{
		// get correlations
		std::vector<HashTable> correlation_map = ListMap(hierarchy1->allFeatures().size(), HashTable());
		for (int i = 0; i < (signed int)features1.size(); i++)
		{
			Feature feat1 = features1.at(i);
			TopologyFileFormat::Segment seg1 = segmentation1->elementSegmentation(feat1.id());
			// Otherwise, get the corresponding handle
			/*StatHandle handle = family.aggregate(statistic.first, statistic.second);

			Statistics::Factory factory;
			Statistics::Attribute* stat = factory.make_aggregator_array(handle.stat(), true);
			stat->resize(handle.elementCount());
			handle.readData(stat);*/



			/*for (LocalIndexType j = 0; j < seg1.size; j++)
			{
				HashTable::iterator it_hash_table = hash_table.find(seg1.samples[j]);
				if (it_hash_table != hash_table.end())
				{*/
					int src_id = feat1.id();
					for (auto& destft : assocation_map[feat1.id()]) {
						int des_id = destft;


						// look to see if we have already overlapped with this element from grid 1          
						HashTable::iterator it = correlation_map.at(src_id).find(des_id);

						// if so then increment the count
						if (it != correlation_map.at(src_id).end())
						{
							(it->second)++;
						}
						else // if not then insert it
						{
							double weight = association_weights[src_id][des_id];
							correlation_map.at(src_id).insert(it, HashTable::value_type(des_id, weight));
							insertEdge(src_id, des_id, weight, weight, false);
						}
					}
			/*	}
			}*/
		}
		std::cout << "Correlation map created" << std::endl;

		// combine correlations
		//combineCorrelations(correlation_map);
		std::cout << "Correlations combined, " << mEdges.size() << " edges found" << std::endl;
		//constructEdgeMap();
	}

}
void ComputeFamilyCorrelation::computeCorrelations(){ 
	switch (tracking_type) {
	case VERTEX_OVERLAP:
		mAccumType = SUM;
		computeCorrelationsOverlap();
		break;
	case DISTANCE:
		mAccumType = MAX;
		double low[3], high[3];
		segmentation1->boundingBox(low, high);
		mMaxDist = sqrt(pow((low[0] - high[0]), 2) + pow((low[1] - high[1]), 2) + pow((low[2] - high[2]), 2));
		mMaxDist = radius;
		std::cout << "MAX DISTANCE IS " << mMaxDist << std::endl;
		computeCorrelationsDistance();
		break;
	case GIVEN:
		mAccumType = SUM;

		computeCorrelationsGiven();
		break;
	default:
		std::cout << "type must be VERTEX_OVERLAP or DISTANCE";
		return;
	}
	
}

}//namespace TopologyFileFormat

