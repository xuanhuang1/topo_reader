#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <vector>


#include "xmlParser.h"

using namespace std;

const char* sFileVersion = "0.0.1"; // The current version of the file format
const char* sTopName = "FeatureFamily"; // The string identifying the top node
const char* sFamilyName = "Family"; // The string identifying the simplification sequence
const char* sAttributeName = "Attribute"; // The string identifying the attribute node
const char* sSimpName = "Simplification"; // The string identifying the attribute node

int main(int argc, const char** argv)
{

  FILE *file;
  file = fopen("testTree.family", "w");
  // Create the top-level node with the version information
  XMLNode xml = XMLNode::createXMLTopNode(sTopName);
  xml.addAttribute("version", sFileVersion);
  XMLNode family = xml.addChild(sFamilyName);
  XMLNode simp = family.addChild(sSimpName);  


  family.addAttribute("encoding", "ascii");
  family.addAttribute("globalsize", "4");
  family.addAttribute("localsize", "4");
  family.addAttribute("precision","float");
  char token[256];
  sprintf(token,"%f %f",16.0, 3.0);
  family.addAttribute("range",token);

  sprintf(token,"%ld",ftell(file));
  family.addAttribute("addr",token);
  
  family.addAttribute("familysize","14");

  
  // write ids
  for(uint32_t i=0; i < 14; i++) {
    fprintf(file,"%llu\n",(uint64_t)i);
  }
  
  // Add the name of the hierarchy used to create this simplification sequence
  simp.addAttribute("name","Highest Threshold");
  
  // For the moment nodes can only store a single parent pointer thus the number
  // of dependence is always 1
  simp.addAttribute("numdependents", "1");
  
  // Add the address of the starting bytes of this simplification block
  sprintf(token,"%ld",ftell(file));
  simp.addAttribute("addr", token);



  // write simpSeq
  fprintf(file,"%e %e %u %llu\n", 7.0, 9.0, 1, (uint64_t)1);
  fprintf(file,"%e %e %u %llu\n", 6.0, 7.0, 1, (uint64_t)2);
  fprintf(file,"%e %e %u %llu\n", 4.0, 6.0, 1, (uint64_t)3);
  fprintf(file,"%e %e %u %llu\n", 3.0, 4.0, 1, (uint64_t)4);
  fprintf(file,"%e %e %u %llu\n", -10e35, 3.0, 1, (uint64_t)-1);
  fprintf(file,"%e %e %u %llu\n", 6.0, 8.0, 1, (uint64_t)2);
  fprintf(file,"%e %e %u %llu\n", 7.0, 5.0, 1, (uint64_t)3);
  fprintf(file,"%e %e %u %llu\n", 8.0, 10.0, 1, (uint64_t)5);
  fprintf(file,"%e %e %u %llu\n", 10.0, 11.0, 1, (uint64_t)7);
  fprintf(file,"%e %e %u %llu\n", 11.0, 13.0, 1, (uint64_t)8);
  fprintf(file,"%e %e %u %llu\n", 11.0, 14.0, 1, (uint64_t)8);
  fprintf(file,"%e %e %u %llu\n", 12.0, 16.0, 1, (uint64_t)13);
  fprintf(file,"%e %e %u %llu\n", 12.0, 15.0, 1, (uint64_t)13);
  fprintf(file,"%e %e %u %llu\n", 10.0, 12.0, 1, (uint64_t)7);

 
  // Create a new xml node for this attribute
  XMLNode attribute = family.addChild(sAttributeName);
  attribute.addAttribute("name", "vertexCount");
  attribute.addAttribute("attribute","none");
  attribute.addAttribute("encoding","ascii");
  attribute.addAttribute("mapping","direct");
  sprintf(token,"%ld",ftell(file));
  attribute.addAttribute("addr", token);
    

  fprintf(file,"7\n");
  fprintf(file,"2\n");
  fprintf(file,"4\n");
  fprintf(file,"5\n");
  fprintf(file,"6\n");
  fprintf(file,"8\n");
  fprintf(file,"9\n");
  fprintf(file,"3\n");
  fprintf(file,"11\n");
  fprintf(file,"10\n");
  fprintf(file,"15\n");
  fprintf(file,"14\n");
  fprintf(file,"13\n");
  fprintf(file,"12\n");

  // Create a new xml node for this attribute
  attribute = family.addChild(sAttributeName);
  attribute.addAttribute("name", "min");
  attribute.addAttribute("attribute","x");
  attribute.addAttribute("encoding","ascii");
  attribute.addAttribute("mapping","direct");
  sprintf(token,"%ld",ftell(file));
  attribute.addAttribute("addr", token);
    

  fprintf(file,"0.0\n");
  fprintf(file,"1.0\n");
  fprintf(file,"2.0\n");
  fprintf(file,"3.0\n");
  fprintf(file,"4.0\n");
  fprintf(file,"5.0\n");
  fprintf(file,"6.0\n");
  fprintf(file,"7.0\n");
  fprintf(file,"8.0\n");
  fprintf(file,"9.0\n");
  fprintf(file,"10.0\n");
  fprintf(file,"11.0\n");
  fprintf(file,"12.0\n");
  fprintf(file,"13.0\n");


  long start = ftell(file);
  XMLSTR header = xml.createXMLString();
  fprintf(file,header);
  fwrite(&start,sizeof(long),1,file);

  std::cout << "done! goodbye." << std::endl;
  return 1;
}
