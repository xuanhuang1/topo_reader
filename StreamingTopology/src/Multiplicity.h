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

#ifndef MULTIPLICITY_H_
#define MULTIPLICITY_H_


//! This class encapsulates he multiplicity information for shared vertices
/*! The multiplicity information of a vertex contains two independent pieces:
 *  The expected global multiplicity (how many copies are out there); and The
 *  number of copies that are represented by this instance.
 */
class Multiplicity
{
public:

  //! The maximal multiplicity we can represent
  static const uint8_t sMaxMultiplicity = 15;

  //! Default constructor
  Multiplicity(uint8_t expected=1, uint8_t current=1) : mInfo((expected << 4) + current) {}

  //! Destructor
  ~Multiplicity() {}

  //! Add another multiplicity piece
  Multiplicity& operator+=(const Multiplicity& mult) {
    sterror(expected() != mult.expected(),"Expected multiplicites do not match inconsistent information.");
    sterror(current() + mult.current() > sMaxMultiplicity,"Joined multiplicity too high. Cannot represent information.");

    mInfo += mult.current();
    return *this;
  }

  //! Return the number of expected copies
  uint8_t expected() const {return mInfo >> 4;}

  //! Return the number of copies seen so far
  uint8_t current() const {return mInfo & 15;}

  //! Return whether we have seen all copies
  bool complete() const {return (expected() == current());}

  //! Function to set the flags directly. The called is responsible for the correctess
  void set(uint8_t info) {mInfo = info;}

private:

  //! The flags storing the two counters
  uint8_t mInfo;

};

#endif /* MULTIPLICITY_H_ */
