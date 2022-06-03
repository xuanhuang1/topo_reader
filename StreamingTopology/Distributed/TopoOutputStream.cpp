/*
 * TopoOutputStream.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#include "TopoOutputStream.h"

//! The default message size
const uint32_t TopoOutputStream::sDefaultMessageSize = 1024;

TopoOutputStream::TopoOutputStream(const std::vector<GraphID>& destinations, uint32_t min_message)
: mDestinations(destinations), mBuffer(2*min_message), mMinimalMessageSize(min_message)
{
}


