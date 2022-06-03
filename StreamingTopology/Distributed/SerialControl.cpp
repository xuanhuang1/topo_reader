/*
 * SerialControl.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "SerialControl.h"
#include "MergeTree.h"
#include <iostream>

SerialControl::SerialControl(bool invert) : mInvert(invert),
mLocalAlgorithm(NULL), mGatherAlgorithm(NULL), mScatterAlgorithm(NULL),
mGatherFlow(NULL), mScatterFlow(NULL)
{
}

SerialControl::~SerialControl()
{
  if (mLocalAlgorithm != NULL)
    delete mLocalAlgorithm;

  if (mGatherAlgorithm != NULL)
    delete mGatherAlgorithm;

  if (mScatterAlgorithm != NULL)
    delete mScatterAlgorithm;

}


void SerialControl::localAlgorithm(LocalAlgorithmType local)
{
  mLocalAlgorithm = LocalComputeAlgorithm::make(local,mInvert);
}

void SerialControl::gatherAlgorithm(GatherAlgorithmType gather)
{
  mGatherAlgorithm = GatherAlgorithm::make(gather,mInvert);
}

void SerialControl::scatterAlgorithm(ScatterAlgorithmType scatter)
{
  mScatterAlgorithm = ScatterAlgorithm::make(scatter,mInvert);
}


int SerialControl::initialize(const std::vector<Patch*>& patches, uint8_t block_bits)
{
  uint32_t i,k;
  std::vector<uint32_t> sinks;

  mPatches = patches;

  // A serial control has all patches locally
  std::cout << "num patches " << patches.size() << std::endl;
  for (i=0;i<patches.size();i++) {
    // and we create a segmented tree for each of them
    std::cout << "making tree " << i << std::endl;
    mTrees.push_back(new SegmentedMergeTree(i, block_bits));
  }

  std::cout << "mTrees.size " << mTrees.size() << std::endl;
  i = 0;
  while (i < mTrees.size()) {
    std::cout << "trees.size = " << mTrees.size() << std::endl;
    std::cout << "i = " << i << std::endl;
    std::cout << "mTrees[i]->id = " << mTrees[i]->id() << std::endl;

    // Which trees do we need downstream
    sinks = mGatherFlow->sinks(mTrees[i]->id());

    for (k=0;k<sinks.size();k++) {

      // If these have not been create
      if (sinks[k] >= mTrees.size())
        mTrees.push_back(new MergeTree(mTrees.size(), block_bits)); // do so
    }
    i++;
  }

  // Now create all the input/output streams.
  mInputStreams.reserve(mTrees.size());
  mOutputStreams.reserve(mTrees.size());
  for (i=0;i<mTrees.size();i++) {
    mInputStreams[i] = new SerialInputStream(mTrees[i]->id(),&mCommunicator);
    mOutputStreams[i] = new SerialOutputStream(std::vector<GraphID>(1,mTrees[i]->id()),&mCommunicator);
  }
}

int SerialControl::compute()
{
  uint32_t i,k;
  std::vector<uint32_t> sinks;
  std::vector<uint32_t> sources;
  TopoOutputStream* upstream;

  // Phase 1: For all patches compute the local tree
  for (i=0;i<mPatches.size();i++) {

    // Which tree do we need downstream
    sinks = mGatherFlow->sinks(mTrees[i]->id());

    mLocalAlgorithm->apply(mPatches[i]->field(0),
                           *static_cast<SegmentedMergeTree*>(mTrees[i]),
                           mOutputStreams[sinks[0]]);
  }

  // For the rest of the trees call gather
  for (;i<mTrees.size();i++) {

    // Which tree do we need downstream
    sinks = mGatherFlow->sinks(mTrees[i]->id());

    // Which trees do we need upstream
    sources = mScatterFlow->sinks(mTrees[i]->id());

    // Create an output stream for the upstream scatter
    upstream = new SerialOutputStream(sources,&mCommunicator);

    // Apply the gather algorithm
    mGatherAlgorithm->apply(*mTrees[i],mInputStreams[i],mOutputStreams[sinks[0]],upstream,mGatherFlow);

    // Remove the output stream
    delete upstream;
  }

  // Finally, in reverse order call all scatter
  for (i=mTrees.size()-1;i!=0;i++) {

    // Which trees do we need downstream
    sources = mScatterFlow->sinks(mTrees[i]->id());

    // Create an output stream for the upstream scatter
    upstream = new SerialOutputStream(sources,&mCommunicator);

    // Apply the scatter algorithm
    mScatterAlgorithm->apply(*mTrees[i],mInputStreams[i],upstream);

    // Remove the output stream
    delete upstream;
  }
}

int SerialControl::save()
{
}
