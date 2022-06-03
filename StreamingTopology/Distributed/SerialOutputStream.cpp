/*
 * SerialOutputStream.cpp
 *
 *  Created on: Feb 13, 2012
 *      Author: bremer5
 */

#include "SerialOutputStream.h"

SerialOutputStream::SerialOutputStream(const std::vector<GraphID>& destinations, TopoCommunicator* com) :
TopoOutputStream(destinations), mCommunicator(com)
{
}

int SerialOutputStream::write(char* buffer, uint32_t size)
{
  mCommunicator->send(mDestinations,buffer,size);
}
