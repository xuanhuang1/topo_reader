/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov 
* OCEC-08-107
* All rights reserved.  
*   
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*   
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*   
*  
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/


#ifndef ARCMETRICS_H
#define ARCMETRICS_H

#include <math.h>
#include <vector>

#include "MultiResGraph.h"
#include "Node.h"

//! Absolution difference in function value 
template <class NodeDataClass = DefaultNodeData>
class AbsolutePersistence : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  AbsolutePersistence(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}
  
  //! Copy constructor
  AbsolutePersistence(const AbsolutePersistence& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new AbsolutePersistence<NodeDataClass>(*this);}

  ~AbsolutePersistence() {}

  float operator()(Node* u, Node* v) const {
    return fabs(u->f()-v->f());
  }

  const char* name() const {return "Absolute Persistence";}

};

//! Difference in function value relative to the global range
template <class NodeDataClass = DefaultNodeData>
class RelativePersistence : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  RelativePersistence(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}
  
  //! Copy constructor
  RelativePersistence(const RelativePersistence& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new RelativePersistence<NodeDataClass>(*this);}

  ~RelativePersistence() {}

  float operator()(Node* u, Node* v) const {
    return fabs(u->f()-v->f()) / (this->mGraph->maxF() - this->mGraph->minF());
  }

  const char* name() const {return "Relative Persistence";}
};

//! Difference in function value relative to the global range
template <class NodeDataClass = DefaultNodeData>
class LogRelativePersistence : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  LogRelativePersistence(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}

  //! Copy constructor
  LogRelativePersistence(const LogRelativePersistence& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new LogRelativePersistence<NodeDataClass>(*this);}

  ~LogRelativePersistence() {}

  float operator()(Node* u, Node* v) const {
    fprintf(stderr,"%f %f    \n",fabs(transform(u->f()) - transform(v->f())),(transform(this->mGraph->maxF()) - transform(this->mGraph->minF())));
    return fabs(transform(u->f()) - transform(v->f())) / (transform(this->mGraph->maxF()) - transform(this->mGraph->minF()));
  }

  const char* name() const {return "Log-Relative Persistence";}
private:

  //! Function to transform the original function into log space
  float transform(FunctionType f) const {return log(1+this->mGraph->minF() + f);}
};


//! Absolute distance in function value of the saddle to the global maximum
template <class NodeDataClass = DefaultNodeData>
class HighestSaddleFirst : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  HighestSaddleFirst(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}
  
  //! Copy constructor
  HighestSaddleFirst(const HighestSaddleFirst<NodeDataClass>& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new HighestSaddleFirst<NodeDataClass>(*this);}

  ~HighestSaddleFirst() {}

  float operator()(Node* u, Node* v) const {
    if ((v->type() == LEAF) && (u->type() == LEAF))
      return this->mGraph->maxF() - std::min(u->f(),v->f());
    else if (v->type() == LEAF)
      return this->mGraph->maxF() - u->f();
    else if (u->type() == LEAF)
      return this->mGraph->maxF() - v->f();
    else
      return gMaxValue;
  }

  const char* name() const {return "High Threshold";}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.downSize() == 0) {
      life[0] = this->mGraph->minF();
    } 
    else {
      sterror(child.downSize() > 1,"For now this metric works only for merge trees.");
      life[0] = child.down()[0]->f();
    }

    life[1] = child.f();
    return 0;
  }

  /*! For the given metric compare two cancellations. The one with
   *  lesser "persistence" will be cancelled first. For the highest
   *  saddle first metric cancellations should break ties first by
   *  considering their saddle and only if the saddles are the same by
   *  looking at the extrema
   */
  virtual bool greater(const typename MultiResGraph<NodeDataClass>::Cancellation& c0,
                       const typename MultiResGraph<NodeDataClass>::Cancellation& c1) const {
    if (c0.p > c1.p) // If the persistences are unambiguous 
      return true;
    else if (c0.p == c1.p) {

      // If we have different saddles cancel the one with higher value
      // first
      if (*c0.a.v < *c1.a.v)
        return true;

      // Only if the saddles are the same look into the extrema
      if (c0.a.v == c1.a.v) {
      
        // If c0 is a max-saddle branch the lower maximum should be
        // cancelled (true == true), If c0 is a min-saddle branch the
        // higher minimum should be cancelled (false == false)
        if ((*c0.a.u > *c0.a.v) == (*c0.a.u > *c1.a.u)) 
          return true;
        else
          return false;
      }
      else
        return false;
    }    
    return false;
  }
};

//! Relative distance in function value of the saddle to the global maximum 
template <class NodeDataClass = DefaultNodeData>
class HighestSaddleFirstRelative : public HighestSaddleFirst<NodeDataClass>
{
public:
  //! Default constructor
  HighestSaddleFirstRelative(const TopoGraphInterface* graph) : HighestSaddleFirst<NodeDataClass>(graph) {}
  
  //! Copy constructor
  HighestSaddleFirstRelative(const HighestSaddleFirstRelative<NodeDataClass>& metric) :
    HighestSaddleFirst<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new HighestSaddleFirstRelative<NodeDataClass>(*this);}

  ~HighestSaddleFirstRelative() {}

  float operator()(Node* u, Node* v) const {
    float f =  HighestSaddleFirst<NodeDataClass>::operator()(u,v);
    
    if (f < gMaxValue)
      return f / (this->mGraph->maxF() - this->mGraph->minF());
    else
      return gMaxValue;
  }

  const char* name() const {return "High Relative Threshold";}

  //uint8_t lifeTime(const Node& child, FunctionType life[2]) const {
  //  return ArcMetric<NodeDataClass>::lifeTime(child,life);
  //}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.downSize() == 0) {
      life[1] = 1;
    } 
    else {
      sterror(child.downSize() > 1,"For now this metric works only for merge trees.");
      life[1] = (this->mGraph->maxF() - child.down()[0]->f()) / (this->mGraph->maxF() - this->mGraph->minF());
    }

    life[0] = (this->mGraph->maxF() - child.f()) / (this->mGraph->maxF() - this->mGraph->minF());
    return 0;
  }

};

//! Absolute distance in function value of the saddle to the global minimum
template <class NodeDataClass = DefaultNodeData>
class LowestSaddleFirst : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  LowestSaddleFirst(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}
  
  //! Copy constructor
  LowestSaddleFirst(const LowestSaddleFirst& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new LowestSaddleFirst<NodeDataClass>(*this);}

  ~LowestSaddleFirst() {}

  float operator()(Node* u, Node* v) const {

    if (u->type() == LEAF)
      return v->f() - this->mGraph->minF();
    else if (v->type() == LEAF)
      return u->f() - this->mGraph->minF();
    else
      return gMaxValue;
  }

  const char* name() const {return "Low Threshold";}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {
    life[0] = child.f();

    if (child.upSize() == 0)
      life[1] = this->mGraph->maxF();
    else
      life[1] = child.up()[0]->f();
    return 1;
  }

  /*! For the given metric compare two cancellations. The one with
   *  lesser "persistence" will be cancelled first. For the lowest
   *  saddle first metric cancellations should break ties first by
   *  considering their saddle and only if the saddles are the same by
   *  looking at the extrema
   */
  virtual bool greater(const typename MultiResGraph<NodeDataClass>::Cancellation& c0,
                       const typename MultiResGraph<NodeDataClass>::Cancellation& c1) const {
    if (c0.p > c1.p) // If the persistences are unambiguous 
      return true;
    else if (c0.p == c1.p) {

      // If we have different saddles cancel the one with lower value
      // first
      if (*c0.a.v > *c1.a.v)
        return true;

      // Only if the saddles are the same look into the extrema
      if (c0.a.v == c1.a.v) {
      
        // If c0 is a max-saddle branch the lower maximum should be
        // cancelled (true == true), If c0 is a min-saddle branch the
        // higher minimum should be cancelled (false == false)
        if ((*c0.a.u > *c0.a.v) == (*c0.a.u > *c1.a.u)) 
          return true;
        else
          return false;
      }
      else
        return false;
    }    
    return false;
  }
};

//! Relative distance in function value of the saddle to the global minimum
template <class NodeDataClass = DefaultNodeData>
class LowestSaddleFirstRelative : public LowestSaddleFirst<NodeDataClass>
{
public:
  //! Default constructor
  LowestSaddleFirstRelative(const TopoGraphInterface* graph) : LowestSaddleFirst<NodeDataClass>(graph) {}
  
  //! Copy constructor
  LowestSaddleFirstRelative(const LowestSaddleFirstRelative<NodeDataClass>& metric) :
    LowestSaddleFirst<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new LowestSaddleFirstRelative<NodeDataClass>(*this);}

  ~LowestSaddleFirstRelative() {}
  
  float operator()(Node* u, Node* v) const {
    float f =  LowestSaddleFirst<NodeDataClass>::operator()(u,v);
    
    if (f < gMaxValue)
      return f / (this->mGraph->maxF() - this->mGraph->minF());
    else
      return gMaxValue;
  }

  const char* name() const {return "Low Relative Threshold";}

  uint8_t lifeTime(const Node& child, FunctionType life[2]) const {
    return ArcMetric<NodeDataClass>::lifeTime(child,life);
  }

};



//! Persistence relative to the local extremum
template <class NodeDataClass = DefaultNodeData>
class MaximaRelevance : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  MaximaRelevance(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}

  //! Copy constructor
  MaximaRelevance(const MaximaRelevance& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new MaximaRelevance(*this);}

  ~MaximaRelevance() {}

  float operator()(Node* u, Node* v) const {

    if (u->type() == LEAF) {
      FunctionType local_extremum;

      // Find the highest value in this subtree
      //local_extremum = findSubtreeMax(v);
      local_extremum = v->representative()->f();

      if (v->downSize() == 0)
        return 1;
      else
        return (local_extremum - v->f()) / (local_extremum - this->mGraph->minF());
    }
    else if (v->type() == LEAF) {
      FunctionType local_extremum;

      // Find the highest value in this subtree
      //local_extremum = findSubtreeMax(u);
      local_extremum = u->representative()->f();

      if (u->downSize() == 0)
        return 1;
      else
        return (local_extremum - u->f()) / (local_extremum - this->mGraph->minF());
    }
    else
      return gMaxValue;
  }

  const char* name() const {return "Maxima Relevance";}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.downSize() == 0) {
      life[0] = 1;
      life[1] = 1;
    }
    else {

      sterror(child.downSize() > 1,"For now this metric works only for merge trees.");

      if (child.type() == LEAF)
        life[0] = 0;
      else
        life[0] = child.persistence();
      life[1] = child.down()[0]->persistence();
    }

    sterror(life[0] > life[1],"Found inverted interval in arc metric.");

    return 1;
  }



private:

  FunctionType findSubtreeMax(const Node* v) const {

    FunctionType high = -gMaxValue;

    std::vector<Node* >::const_iterator it;
    std::stack<const Node*> front;
    const Node* top;

    front.push(v);

    while (!front.empty()) {
      top = front.top();
      front.pop();

      if (top->up().empty())
        high = std::max(high,top->f());
      else {
        for (it=top->up().begin();it!=top->up().end();it++)
          front.push(*it);
      }
    }

    return high;
  }

};

//! Persistence relative to the local extremum
template <class NodeDataClass = DefaultNodeData>
class MinimaRelevance : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  MinimaRelevance(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}

  //! Copy constructor
  MinimaRelevance(const MinimaRelevance& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new MinimaRelevance(*this);}

  ~MinimaRelevance() {}

  float operator()(Node* u, Node* v) const {

    Node* top = u;
    Node* bottom = v;
    FunctionType local_extremum;

    if (*top < *bottom) {
      bottom = u;
      top = v;
    }

    sterror(bottom->morseType() != MINIMUM,"The lower node in a minimum hierarchy should always be a minimum.");



    //local_extremum = findSubtreeMin(top);
    local_extremum = top->representative()->f();

    /*
    if (local_extremum != top->representative()->f()) {
      fprintf(stderr,"Found inconsistent representatives %f %f\n",local_extremum,top->representative()->f());
     }
    else {
      fprintf(stderr,"A OK %f %f\n",local_extremum,top->representative()->f());
    }
     */
    return (top->f() - local_extremum) / (this->mGraph->maxF() - local_extremum);


    /*
    if (u->type() == LEAF) {
      FunctionType local_extremum;

      // Find the highest value in this subtree
      local_extremum = findSubtreeMin(v);

      if (v->upSize() == 0)
        return 1;
      else
        return (v->f() - local_extremum) / (this->mGraph->maxF() - local_extremum);
    }
    else if (v->type() == LEAF) {
      FunctionType local_extremum;

      // Find the highest value in this subtree
      local_extremum = findSubtreeMin(u);

      if (u->upSize() == 0)
        return 1;
      else
        return (u->f() - local_extremum) / (this->mGraph->maxF() - local_extremum);
    }
    else {
      return gMaxValue;
    }
    */
  }

  const char* name() const {return "Minima Relevance";}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.upSize() == 0) {
      // If this is a root that was part of canceled root branch
      if (child.persistence() < 10)
        life[0] = child.persistence();
      else // If not it is an isolated node that has MAX_PERSISTENCE which we set to 0,1
        life[0] = 0;
      life[1] = 1;
    }
    else {

      sterror(child.upSize() > 1,"For now this metric works only for split trees.");

      if (child.type() == LEAF)
        life[0] = 0;
      else
        life[0] = child.persistence();
      life[1] = child.up()[0]->persistence();
    }

    sterror(life[0] > life[1],"Found inverted interval in arc metric  %f > %f.",life[0],life[1]);

    return 1;
  }

private:

  FunctionType findSubtreeMin(const Node* v) const {

    FunctionType low = gMaxValue;

    std::vector<Node* >::const_iterator it;
    std::stack<const Node*> front;
    const Node* top;

    front.push(v);

    while (!front.empty()) {
      top = front.top();
      front.pop();

      if (top->down().empty())
        low = std::min(low,top->f());
      else {
        for (it=top->down().begin();it!=top->down().end();it++)
          front.push(*it);
      }
    }

    return low;
  }

};


//! Persistence relative to the local extremum
template <class NodeDataClass = DefaultNodeData>
class LocalThreshold : public ArcMetric<NodeDataClass>
{
public:
  //! Default constructor
  LocalThreshold(const TopoGraphInterface* graph) : ArcMetric<NodeDataClass>(graph) {}
  
  //! Copy constructor
  LocalThreshold(const LocalThreshold& metric) : ArcMetric<NodeDataClass>(metric) {}

  //! Create a new identical copy
  ArcMetric<NodeDataClass>* clone() const {return new LocalThreshold(*this);}

  ~LocalThreshold() {}

  float operator()(Node* u, Node* v) const {

    if (u->type() == LEAF) {
      FunctionType local_extremum;

      if (u->morseType() == MAXIMUM) {
        // Find the highest value in this subtree
        //local_extremum = findSubtreeMax(v);
        local_extremum = v->representative()->f();
        
        if (v->downSize() == 0)
          return 1;
        else
          return (local_extremum - v->f()) / (local_extremum - this->mGraph->minF());
      }
      else {
        // Find the lowest value in this subtree
        //local_extremum = findSubtreeMin(v);
        local_extremum = v->representative()->f();

        if (v->upSize() == 0)
          return 1;
        else
          return (v->f() - local_extremum) / (this->mGraph->maxF() - local_extremum);
      }
    }
    else if (v->type() == LEAF) {
      FunctionType local_extremum;

      if (v->morseType() == MAXIMUM) {
        // Find the highest value in this subtree
        //local_extremum = findSubtreeMax(u);
        local_extremum = u->representative()->f();
        
        if (u->downSize() == 0)
          return 1;
        else
          return (local_extremum - u->f()) / (local_extremum - this->mGraph->minF());
      }
      else {
        // Find the lowest value in this subtree
        //local_extremum = findSubtreeMin(u);
        local_extremum = u->representative()->f();

        if (u->upSize() == 0)
          return 1;
        else
          return (u->f() - local_extremum) / (this->mGraph->maxF() - local_extremum);
      }
    }
    else
      return gMaxValue;
  }

  const char* name() const {return "Local Threshold";}

  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.downSize() == 0) {
      life[0] = 1;
      life[1] = 1;
    }
    else {

      sterror(child.downSize() > 1,"For now this metric works only for merge trees.");

      if (child.type() == LEAF)
        life[0] = 0;
      else
        life[0] = child.persistence();
      life[1] = child.down()[0]->persistence();
    }

    sterror(life[0] > life[1],"Found inverted interval in arc metric.");

    return 1;
  }

  /*!
  //! Determine the life time of a node
  virtual uint8_t lifeTime(const Node& child, FunctionType life[2]) const {

    if (child.downSize() == 0) {
      life[0] = 0;
      life[1] = 0;
    }
    else {

      sterror(child.downSize() > 1,"For now this metric works only for merge trees.");

      if (child.type() == LEAF)
        life[1] = 1;
      else
        life[1] = 1 - child.persistence();

      life[0] = 1 - child.down()[0]->persistence();
    }

    sterror(life[0] > life[1],"Found inverted interval in arc metric.");

    return 0;
  }
*/

private:
  
  FunctionType findSubtreeMax(const Node* v) const {

    FunctionType high = -gMaxValue;

    std::vector<Node* >::const_iterator it;
    std::stack<const Node*> front;
    const Node* top;

    front.push(v);

    while (!front.empty()) {
      top = front.top();
      front.pop();

      if (top->up().empty())
        high = std::max(high,top->f());
      else {
        for (it=top->up().begin();it!=top->up().end();it++)
          front.push(*it);
      }
    }

    return high;
  }
    
  FunctionType findSubtreeMin(const Node* v) const {
    
    FunctionType low = gMaxValue;

     std::vector<Node* >::const_iterator it;
     std::stack<const Node*> front;
     const Node* top;

     front.push(v);

     while (!front.empty()) {
       top = front.top();
       front.pop();

       if (top->down().empty())
         low = std::min(low,top->f());
       else {
         for (it=top->down().begin();it!=top->down().end();it++)
           front.push(*it);
       }
     }

     return low;
  }
    
};



#endif
