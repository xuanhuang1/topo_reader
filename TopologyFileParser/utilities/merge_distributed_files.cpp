/*
 * merge_distributed_files.cpp
 *
 *  Created on: Apr 13, 2013
 *      Author: bremer5
 */

#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <errno.h>

#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SimplificationHandle.h"
#include "FeatureElement.h"
#include "FileData.h"

using namespace TopologyFileFormat;

/*!This executable will merge a number of distributed files into a
 * single FeatureHierarchy and FeatureSegementation. It will also
 * perform some basic consistency checks. The incoming format is
 * assumed to be as follows. A collection of binary files with each
 * File storing:
 *
 * <LocalIndexType>                     // Count of vertex-id seg-id pairs
 * <GlobalIndexType> <GlobalIndexType>  // vertex-id - seg-id
 * :
 * :
 * <GlobalIndexType> <GlobalIndexType>   // node-id - node-id
 *
 * Where the node-id pairs represent arcs of the tree.
 *
 */

int main(int argc, const char *argv[])
{
  if (argc < 4) {
    fprintf(stderr,"Usage: %s <input0> ... <input1> <output-name> <domain-type>", argv[0]);
    exit(0);
  }

  // The map of global node/seg ids to local indices in the array
  std::map<GlobalIndexType,LocalIndexType> index_map;
  std::map<GlobalIndexType,LocalIndexType>::const_iterator mIt,mIt2;
  std::vector<std::vector<GlobalIndexType> > segmentation;
  Data<FeatureElement> simplification;
  FeatureElement feature(SINGLE_REPRESENTATIVE);
  int error_count = 0;

  std::ifstream input;
  LocalIndexType count,file_length;
  std::vector<GlobalIndexType> pairs;
  for (uint32_t i=1;i<argc-2;i++) {
    input.open(argv[i],std::ios::in | std::ios::binary);
    if (input.fail()) {
      fprintf(stderr,"Could not open file \"%s\" . Got errno %d = \"%s\".\n",argv[i],errno,strerror(errno));
      exit(0);
    }
    // Get the length of the file
    input.seekg (0, input.end);
    file_length = input.tellg();
    input.seekg (0, input.beg);


    fprintf(stderr,"\n\n<<<<<<<<<<<<<<<< Tree %s\n",argv[i]);
    input.read((char*)&count,sizeof(LocalIndexType));

    pairs.resize(count);
    input.read((char*)&(pairs[0]),sizeof(GlobalIndexType)*count);

    // For all point - seg_index pairs
    for (LocalIndexType k=0;k<count;k+=2) {
      mIt = index_map.find(pairs[k+1]);

      // If we have not seen this segment before
      if (mIt == index_map.end()) {
        // We make a new segment
        index_map[pairs[k+1]] = segmentation.size();
        segmentation.push_back(std::vector<GlobalIndexType>());
        mIt = index_map.find(pairs[k+1]);
      }

      // We add the vertex to the segment
      segmentation[mIt->second].push_back(pairs[k]);
    }

    pairs.resize((file_length - sizeof(LocalIndexType)) / sizeof(GlobalIndexType) - count);
    input.read((char*)&(pairs[0]),file_length - sizeof(GlobalIndexType)*count - sizeof(LocalIndexType));
    input.close();

    feature.direction(true);
    simplification.resize(segmentation.size(),feature);

    // For all arcs of the tree
    for (LocalIndexType k=0;k<pairs.size();k+=2) {

      // First we must make sure that both nodes have segments
      mIt = index_map.find(pairs[k]);
      if (mIt == index_map.end()) {
        // We make a new segment
        index_map[pairs[k]] = segmentation.size();
        segmentation.push_back(std::vector<GlobalIndexType>());
        simplification.push_back(feature);
        mIt = index_map.find(pairs[k]);
      }

      mIt2 = index_map.find(pairs[k+1]);
      if (mIt2 == index_map.end()) {
        // We make a new segment
        index_map[pairs[k+1]] = segmentation.size();
        segmentation.push_back(std::vector<GlobalIndexType>());
        simplification.push_back(feature);
        mIt2 = index_map.find(pairs[k+1]);
      }

      fprintf(stderr,"%d,%d -> %d,%d\n",pairs[k],mIt->second,pairs[k+1],mIt2->second);

      // Now add mIt2 as representative to mIt and do a sanity check
      if (!simplification[mIt->second].links().empty()) {
        // If we have seen this feature before its representative should be the same
        //assert(simplification[mIt->second].link(0) == mIt2->second);
        if (simplification[mIt->second].link(0) != mIt2->second)
          fprintf(stderr,"ERROR second child found  (%dth error)\n",error_count++);
      }
      else
        simplification[mIt->second].addLink(mIt2->second);

      // Now set the lifetime
      simplification[mIt->second].lifeTime(0,1);
    } // End for all arcs
  }// End for all files


  // Now we need to write the two files
  char output_name[200];
  sprintf(output_name,"%s.seg",argv[argc-2]);
  ClanHandle clan(output_name);
  FamilyHandle family;
  SegmentationHandle seg;

  seg.setSegmentation(&segmentation);
  seg.encoding(false);
  seg.domainDescription(std::string(argv[argc-1]));

  family.add(seg);
  clan.add(family);
  clan.write();

  sprintf(output_name,"%s.family",argv[argc-2]);
  clan = ClanHandle(output_name);
  family = FamilyHandle();
  SimplificationHandle simp;

  simp.metric("Absolute Threshold");
  simp.fileType(SINGLE_REPRESENTATIVE);
  simp.setRange(0,1);
  simp.setData(&simplification);

  family.add(simp);
  clan.add(family);
  clan.write();


  return 1;
}



