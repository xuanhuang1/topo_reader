#ifndef MERGE_TREE
#define MERGE_TREE

#include "DistributedDefinitions.h"
#include "FlexArray/MappedElement.h"
#include "FlexArray/MappedArray.h"
#include "Token.h"

typedef uint32_t TreeNodeIndex;
const TreeNodeIndex TNULL = (TreeNodeIndex)-1;

typedef uint8_t MultiplicityType;

class TreeNode : public FlexArray::MappedElement<GlobalIndexType,TreeNodeIndex> {
public:

  friend class FlexArray::MappedArray<TreeNode,GlobalIndexType,TreeNodeIndex>;
  friend class FlexArray::BlockedArray<TreeNode>;
  friend class FlexArray::Array<TreeNode>;

  //! Default constructor
  TreeNode(LocalIndexType index, FunctionType f, MultiplicityType multiplicity);

  //! Copy constructor
  TreeNode(const TreeNode& node);

   //! Destructor
  ~TreeNode() {}

  //! Assignment operator
  TreeNode& operator=(const TreeNode& node);

  //! Convert the node into a vertex token
  operator const VertexToken() const;

  //! Return the function value
  FunctionType value() const {return mValue;}

  //! Return the current multiplicity
  MultiplicityType multiplicity() const {return mMultiplicity;}

  //! Reduce the multiplicity
  void reduceMultiplicity() {sterror(mMultiplicity<=0,"Negative multiplicity");mMultiplicity--;}

  //! A generic flag used for traversals
  bool flag() const {return mFlag;}

  void flag(bool f) {mFlag = f;}

public:
  //! These indices are public to allow easy access


  //! Index of one of the parents
	TreeNodeIndex up;

	//! Index of one of my siblings.
	/*! The index to one of my siblings. If this node is part of a tree
	 *  next == my_id and points to myself; otherwise next = TNULL,
	 *  indicating that this node is not part of a tree
	 */
	TreeNodeIndex next;

	//! index of my child
	TreeNodeIndex down;

private:

	//! Private constructor
	TreeNode() {}

	//! The function value
	FunctionType mValue;

	//! The multiplicity flag
	uint32_t mMultiplicity;
	
	//! A generic flag for traversals
	bool mFlag;


};

class MergeTree : public FlexArray::MappedArray<TreeNode,GlobalIndexType,TreeNodeIndex> {
public:

  //! Constructor
  MergeTree(GraphID id, uint8_t block_bits=FlexArray::BlockedArray<TreeNode>::sBlockBits) : 
      FlexArray::MappedArray<TreeNode,GlobalIndexType,TreeNodeIndex>(block_bits), mId(id) {}

  //! Destructor
  ~MergeTree() {}

  //! Return id
  GraphID id() const {return mId;}

  //! Add a node to the tree
  virtual void addNode(GlobalIndexType index, FunctionType value, MultiplicityType boundary_count);

  //! Add a node to the tree
  virtual void addNode(const VertexToken& token);

  //! Add an edge to the tree (note that the nodes are expected to be ordered
  virtual void addEdge(GlobalIndexType upper, GlobalIndexType lower);

  //! Bypass a vertex
  virtual void bypass(GlobalIndexType v) {}
protected:

  //! The global id of this tree
  const GraphID mId;
};







#endif
