#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <cstdio>
#include <fstream>

#include "TalassConfig.h"
#include "SimplificationHandle.h"

namespace TopologyFileFormat 
{

class Feature;

//! ASCII input
std::istream& operator>>(std::istream& input, Feature& f);

//! ASCII output
std::ostream& operator<<(std::ostream& output, const Feature& f);


class Feature
  {
  public:

    //! Friend declaration to allow the hierarchy to change the ids
    friend class FeatureHierarchy;

    //! Typedef for the iterator through the constituents / representative lists
    typedef std::vector<Feature*>::const_iterator iterator;

    //! Bitmask used for storing the alive flag 
    static const uint8_t sAliveMask = 1;

    //! Bitmask used for storing the parity flag
    static const uint8_t sParityMask = 2;

    //! Bitmask used for storing the direction flag
    static const uint8_t sDirectionMask = 4;

    //! Bitmask used for stroing the visited flag
    static const uint8_t sVisitedMask = 8;  

    //! Default constructor
    Feature();

    //! Copy constructor
    Feature(const Feature& f) { *this = f; }

    //! Destructor
    virtual ~Feature() {}

    //! Assignment operator
    Feature& operator=(const Feature& f);

    /*******************************************************************************************
     ****************************  Manipulating various flags    *******************************
     ******************************************************************************************/

    //! Determine whether the feature is alive for the given parameter
    bool living(FunctionType p) const;

    //! Determine whether the feature is merged for the given parameter
    bool merged(FunctionType p) const;

    //! Return whether the feature is marked alive
    bool alive() const {return getBitFlag(sAliveMask);}

    //! Set the alive flag
    void alive(bool a) { setBitFlag(sAliveMask,a); }

    //! Return whether the feature is counted an odd or even number of times
    bool parity() const { return getBitFlag(sParityMask); }
  
    //! Set whether the feature is counted an odd or even number of times
    void parity(bool p) { setBitFlag(sParityMask,p); }

    //! Flip the parity flag
    void flipParity() { flipBitFlag(sParityMask); }

    //! Return whether the feature has been visited
    bool visited() const { return getBitFlag(sVisitedMask); }

    //! Set the visited flag
    void visited(bool v) { setBitFlag(sVisitedMask,v); }

    //! Get the direction flag.
    /*! The direction marks on which side of its life span a feature will get born
     *  vs. merge. If the flag is true then we have a ascending hierarchy in
     *  which a feature is born at mLifeTime[0] and merges are mLifeTime[1]. In
     *  this case the life span is open on the top,
     *  i.e. [mLifeTime[0],mLifeTime[1]). If the flag is false the feature will be
     *  born at the high point of the interval and merge at the low point.
     */
    bool direction() const {return getBitFlag(sDirectionMask);}

    //! Set the direction flag
    void direction(bool d) {setBitFlag(sDirectionMask,d);}

    /*******************************************************************************************
     ****************************  Access local data  ******************************************
     ******************************************************************************************/
  
    //! Return the life span
    const FunctionType* lifeTime() const {return mLifeTime;}

    //! Set the life span
    void lifeTime(FunctionType birth, FunctionType death)  {mLifeTime[0] = birth;mLifeTime[1]=death;}

    //! Return my agent
    const Feature* agent() const {return mAgent;}

    //! Set the agent
    void agent(const Feature* a) {mAgent = a;}

    //! Return my id
    LocalIndexType id() const {return mId;}

    //! Set my id
    void id(LocalIndexType id) {mId = id;}

    /*******************************************************************************************
     **************************** Functionts to travers the hierarchy **************************
     ******************************************************************************************/

    //! Return the vector of constituents
    const std::vector<Feature*>& constituents() const {return mCon;}

    //! Return a pointer to the i'th constituent
    Feature* con(uint32_t i);

    //! Return a const pointer to the i'th constituent
    const Feature* con(uint32_t i) const;

    //! Return the number of constituents
    uint32_t conSize() const {return mCon.size();}

    //! Add a constituent
    void addCon(Feature* f);

    //! Return the vector of representatives
    const std::vector<Feature*>& representatives() const {return mRep;}

    //! Return a pointer to the i'th representative
    Feature* rep(uint32_t i);

    //! Return a const pointer to the i'th representative
    const Feature* rep(uint32_t i) const;

    //! Return the number of representatives
    uint32_t repSize() const {return mRep.size();}

    //! Add a representative
    void addRep(Feature* f);

    /*******************************************************************************************
     ****************************  File Interface  *********************************************
     ******************************************************************************************/
  
    //! Return the size of a feature in bytes
    uint32_t size() const {return sizeof(Feature);}

    //! Set the first feature pointer (must be set before File I/O)
    //static void firstFeature(Feature* first) { sFirstFeature = first; }

    //! Set the feature file type
    static void fileType(FeatureFileType type) { sFileType = type; }

    //! Write the binary version of the element to the file
    void writeBinary(std::ofstream& output, const Feature* first) const;

    //! Read from a binary file
    void readBinary(std::ifstream& input, Feature* first);

    //! Write the ascii version of the feature to the file
    virtual void writeASCII(std::ofstream& output, const Feature* first) const;

    //! Read from a binary file
    void readASCII(std::ifstream& input, Feature* first);

  private:

    //! Pointer to the first element in an array of features used for I/O
    //static Feature* sFirstFeature;

    //! Type of feature file-type used for I/O
    static FeatureFileType sFileType;

    //! Collection of constituent pointers
    std::vector<Feature*> mCon;

    //! Collection of representative pointers
    std::vector<Feature*> mRep;

    //! Pointer to my agent
    const Feature* mAgent;

    //! The parameter in which thius node is alive
    FunctionType mLifeTime[2];

    //! My own id
    LocalIndexType mId;

    //! A collection of flags
    uint8_t mFlags;

    //! Setting a flag using an actual bit mask
    void setBitFlag(uint8_t mask, bool f) {if (f) mFlags |= mask; else mFlags &= ~mask;}
  
    //! Getting a flag using an actual bit mask
    bool getBitFlag(uint8_t mask) const {return mFlags & mask;}

    //! Flipping a flag using an actual bit mask
    void flipBitFlag(uint8_t mask) {if (getBitFlag(mask)) setBitFlag(mask,false); else setBitFlag(mask,true);}

  };

  class LifeTimeCmp {
  public:
    LifeTimeCmp(uint8_t l) : mLimit(l) {}

    bool operator()(const Feature* f0, const Feature* f1) const {
      return f0->lifeTime()[mLimit] < f1->lifeTime()[mLimit];
    }

    const uint8_t mLimit;
  };

}

#endif

