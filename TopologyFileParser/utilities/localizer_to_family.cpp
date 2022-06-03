#include "TalassConfig.h"
#include "ClanHandle.h"
#include "SegmentationHandle.h"
#include "SimplificationHandle.h"
#include "FeatureSegmentation.h"
#include "FeatureHierarchy.h"
#include "FeatureData.h"
#include "Segment.h"
#include "FeatureElement.h"
#include <vector>
#include <string>
#include <sstream>
//#include "GridParser.h"
//#include "Attribute.h"

using namespace std;
using namespace TopologyFileFormat;
//typedef GenericData<FunctionType> ParseType;


// Example file to demonstrate how to output a one-parameter
// hierarchy as .family and .seg files
int main(int argc, const char* argv[])
{

    if (argc != 2) std::cout << "enter csv with frame#,x,y... x dim y dim" << std::endl;

    std::string fn = argv[1];

    std::ifstream file(fn);

    std::stringstream buffer;
    if (file)
    {

        buffer << file.rdbuf();

        file.close();

        // operations on the buffer...
    }
    std::string tmp;
    vector<string> words;
    char delim = ','; // Ddefine the delimiter to split by

    while (std::getline(buffer, tmp, delim)) {
        // Provide proper checks here for tmp like if empty
        // Also strip down symbols like !, ., ?, etc.
        // Finally push it.
        //std::cout << tmp << std::endl;
        words.push_back(std::string(tmp));
    }
    std::unordered_map<int, std::vector<std::pair<double, double> > > frames;

    for (int i = 0; i < words.size() - 3; i += 3) {
        //std::cout <<i<<" "<<(words[i]) << std::endl;
        //std::cout << stoi(words[i]) << std::endl;
        frames[stoi(words[i])].push_back(std::make_pair<double, double>(stod(words[i + 1]), stod(words[i + 2])));
    }

    //return 0;

    // Data in the topology file format is ultimately stored in types
    // derived from FileData. Most often these end up being std::vectors

    // Lets define a hierarchy of 7 nodes. Note that we tell the constructor that all
    // our features will have a single representative/parent which is what we normally
    // encounter (everybody has a single parent but maybe multiple children)
    int time = 0;

    for (auto& step : frames) {

        std::string filename = std::string("TestOutput_") + std::string(std::to_string(step.first)) + std::string(".family");


        vector<FeatureElement> features(step.second.size() * 2, FeatureElement(SINGLE_REPRESENTATIVE));


        // Lets assume we have a binary clustering hierarchy where all 4 leafs
        // exist at 0 and the threshold is some sort of distance criterion

        //                    threshold
        // 0     1   2   3       0
        //  \   /     \ /        1
        //   \ /       5         2
        //    4       /          3
        //     \     /           4
        //      \   /            5
        //        6              6

        // Now we set the intervals at which these features are alive
        // and all the other information
        for (int i = 0; i < step.second.size() * 2; i += 2) {
            features[i].lifeTime(0, .5);
            features[i].direction(true);
            features[i + 1].lifeTime(.5, 1);
            features[i + 1].direction(true);
            features[i].addLink(i + 1);


        }
        int feature_count = step.second.size() * 2;
        //// The node/feature "0" is alive if the threshold is [0,3)
        //features[0].lifeTime(0, 3); // We set this lifetime

        //// See Feature.h:Line 89 : These features are "born" at their low value and
        //// "die" at the high value which is considered an ascending hierarchy
        //features[0].direction(true);


        //// Now we do the same for all other nodes
        //features[1].lifeTime(0, 3);
        //features[1].direction(true);

        //features[2].lifeTime(0, 2);
        //features[2].direction(true);

        //features[3].lifeTime(0, 2);
        //features[3].direction(true);

        //features[4].lifeTime(3, 6);
        //features[4].direction(true);

        //features[5].lifeTime(2, 6);
        //features[5].direction(true);

        //features[6].lifeTime(6, 7); // Note we could hvae picked any value here larger than 6
        //features[6].direction(true);


        //// Now we link these up. Note that we only need one direction
        //features[0].addLink(4);
        //features[1].addLink(4);

        //features[2].addLink(5);
        //features[3].addLink(5);

        //features[4].addLink(6);
        //features[5].addLink(6);



        // Now we save this to a file

        // The name of the family file
        ClanHandle clan(filename);

        // The name of the data set (all time steps must have
        // the same data set name)
        clan.dataset("Rebika-SPT");

        // Create the family which is the internal name for the collection
        // of all the information related to a time step
        FamilyHandle family;

        // The index of this time step
        family.timeIndex(step.first);

        // The actual floating point real time. Often this is identical
        // to the index but it does not have to be. Also this allows to
        // accept time series with unevenly distributed time steps
        family.time(step.first);

        // The name of the threshold parameter we are using
        family.variableName("ParameterName");

        // The range of the parameter
        family.range(0, 1);

        // Now we create the actual hierarchy in form of a simplification
        // sequence. As before we first create the handle
        SimplificationHandle simplification;

        // Set some more meta data
        simplification.metric("Threshold");
        simplification.fileType(SINGLE_REPRESENTATIVE);
        simplification.setRange(0, 1);
        simplification.encoding(true); // For testing purposes we write this in ascii

        // Now we assign the data to this handle. To comply with the API we need to
        // have a type derived from FileData
        Data<FeatureElement> data(&features); // This does not copy the data but is simply a pointer

        simplification.setData(&data); // Again this only passes pointers


        // Now we attach the hierarchy to the family
        family.add(simplification);

        // and the family to the clan
        clan.add(family);

        // And finally we write the clan to file
        clan.write();


        // Now let's figure out how to add some geometric points to act as elements of each cluster

        //std::cout << "Num features is " << step.second.size() << std::endl;
        // First we create a 10x10 grid of points (standard row major layout)
        vector<FunctionType> coords(step.second.size() * 2 * 5);

        for (int j = 0; j < step.second.size(); j++) {
            //for (int i = 0; i < 10; i+=2) {
            coords[2 * 5 * j] = step.second[j].first;     // x-coord
            coords[2 * 5 * j + 1] = step.second[j].second; // y-coord

            coords[2 * 5 * j + 2] = step.second[j].first - 1; // x-coord
            coords[2 * 5 * j + 3] = step.second[j].second - 1; // y-coord

            coords[2 * 5 * j + 4] = step.second[j].first + 1; // x-coord
            coords[2 * 5 * j + 5] = step.second[j].second + 1; // y-coord

            coords[2 * 5 * j + 6] = step.second[j].first + 1; //x-coord
            coords[2 * 5 * j + 7] = step.second[j].second - 1; // y-coord

            coords[2 * 5 * j + 8] = step.second[j].first - 1; // x-coord
            coords[2 * 5 * j + 9] = step.second[j].second + 1; // y-coord
        //}
        }

        // And we add those to a geometry handle
        GeometryHandle geometry;
        geometry.dimension(2); // We could have done this in 2 dimensions
        geometry.encoding(true); // Make things ascii for testing

        Data<FunctionType> points(&coords);
        geometry.setData(&points);


        // Now we create the segmentation. One way of doing this is to store a set of sets
        // style representation. Here "segment" 0 will correspond to all point indices part
        // of feature/node 0 etc.
        //std::cout << "size is " << step.second.size() * 2 << std::endl;
        vector<vector<GlobalIndexType> > segments(step.second.size() * 2);


        for (int i = 0; i < step.second.size() * 2; i += 2) {
            segments[i].resize(5);
            segments[i][0] = (i / 2) * 10;
            segments[i][1] = (i / 2) * 10 + 1 * 2;
            segments[i][2] = (i / 2) * 10 + 2 * 2;
            segments[i][3] = (i / 2) * 10 + 3 * 2;
            segments[i][4] = (i / 2) * 10 + 4 * 2;

        }
        //// In this case let's assume only the leafs contain data (like in a clustering hierarchy)
        //segments[0].resize(25);
        //segments[1].resize(25);
        //segments[2].resize(25);
        //segments[3].resize(25);

        //// And for convenience let's just do things by stripes
        //for (int i = 0; i < 25; i++) {
        //    segments[0][i] = i;
        //    segments[1][i] = 25 + i;
        //    segments[2][i] = 50 + i;
        //    segments[3][i] = 75 + i;
        //}


        // And just like usual we now add this data to a segmentation handle
        SegmentationHandle segmentation;
        segmentation.encoding(true); // Make things ascii for testing
        segmentation.domainType(POINT_SET);

        char domain_description[100];
        sprintf(domain_description, "2 %f %f %f %f ", 0.0, 128.0, 0.0, 128.0);
        segmentation.domainDescription(domain_description);


        segmentation.setSegmentation(&segments);

        //Now we add the geometry to the segmentation
        segmentation.add(geometry);
        //sterror(values.size() != attribute_index.size(), "Number of statistics does not match the number of attribute indices.");
        //std::vector<Attribute* > values;
        //std::vector<Attribute*>::iterator aIt;
        //// Make sure there is enough space for each value array
        //for (aIt = values.begin(); aIt != values.end(); aIt++)
        //    (*aIt)->resize(feature_count);
        //GlobalIndexType gRawDimensions[3];
        //gRawDimensions[0] = 128;
        //gRawDimensions[1] = 128;
        //gRawDimensions[2] = 1;
        //std::vector<FILE*> gAttributeFiles;
        //FILE* raw_file = fopen("step_0.raw", "r");
        //gAttributeFiles.push_back(raw_file);
        //std::vector<GlobalIndexType> persistent_attributes;
        //GridParser<ParseType>*parser = new GridParser<ParseType>(gAttributeFiles, gRawDimensions[0], gRawDimensions[1], gRawDimensions[2],
        //    2, persistent_attributes, false, NULL);

        //const std::vector<Parser<GenericData<FunctionType> >::CacheArray*>& attributes=parser->attributes();
        // Now go through the complete segmentation and aggregate all necessary values

        //std::string raw_file = "step_0.raw";
        //std::ifstream myData(raw_file, std::ios::binary);
        //float value;
        //int i = 0;
        //char buf[sizeof(float)];
        //int j = 0;
        //float raw_data[128][128]; //TODO: not hardcode dimensions

        //while (myData.read(buf, sizeof(buf)))
        //{
        //    memcpy(&value, buf, sizeof(value));
        //    //std::cout << value << " ";
        //    raw_data[i][j] = value;
        //    j++;
        //    if (j % 128 == 0) {
        //        j = 0;
        //        i++;
        //    }


        //}
        //std::vector<Attribute* > values;
        //std::vector<Attribute*>::iterator aIt;
        //for (int k = 0; k < segments.size(); k++) {
        //    for (int j = 0; j < segments.size(); j++) {

        //        // Only if this vertex is assigned to a feature do we need to consider it


        //            /*sterror(segments[k] >= feature_count, "Number of features was supposed to be %llu but we found a segmentation index %llu. Did you use --raw-segmentation ?.",
        //                (uint64_t)(feature_count), (uint64_t)segments[k]);*/

        //        for (i = 0; i < values.size(); i++) {
        //            if (values[i]->numVariables() == 1) {
        //                (*values[i])[segments[k]].addVertex(raw_data[k][j], k * j + k);
        //            }
        //            else if (values[i]->numVariables() == 1) {
        //                (*values[i])[segments[k]].addVertex(0, k);
        //            }
        //            else {
        //                (*values[i])[segments[k]].addVertex(attributes[attribute_index[i][0]]->at(k),
        //                    attributes[attribute_index[i][1]]->at(k), k);
        //            }
        //        }


        //    }
            filename = std::string("TestOutput_") + std::string(std::to_string(step.first)) + std::string(".seg");

            // Just for demonstration purposes we re-open the earlier file
            ClanHandle clanseg(filename);
            clanseg.dataset("Rebika-SPT");

            // Create the family which is the internal name for the collection
            // of all the information related to a time step
            FamilyHandle familyseg;

            // The index of this time step
            familyseg.timeIndex(step.first);

            // The actual floating point real time. Often this is identical
            // to the index but it does not have to be. Also this allows to
            // accept time series with unevenly distributed time steps
            familyseg.time(step.first);

            // The name of the threshold parameter we are using
            familyseg.variableName("ParameterName");

            // The range of the parameter
            familyseg.range(0, 1);
            clanseg.add(familyseg);
            // We happen to know that this is the first family in the file and we "append" our information
            // Note that this call will automatically write this information to file and update the meta data
            clanseg.family(0).add(segmentation);
            clanseg.write();
            time++;
            // We are done ... have a look at the bottom of the file to better understand what is happening
        }




        return 1;
    }
