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
#include "GridParser.h"
#include "Attribute.h"
#include "AggregatorFactory.h"
#include<cmath>
#include <FamilyHandle.cpp>
#include "ComputeTracking.h"
#include "EdgeHandle.h"
#include "HandleCollection.h"


using namespace std;
using namespace TopologyFileFormat;
typedef GenericData<FunctionType> ParseType;

class GeneratedFeature {
public:
    GeneratedFeature(std::vector<std::string> row) {
        frame = stoi(row[3]);
        if (row[9] != "NaN") {
            merge_track = stoi(row[9]);
            ignore = true;
        }
        else {
            merge_track = -1;
            ignore = false;
        }
        if (row[8] != "NaN")
            split_track = stoi(row[8]);
        else
            split_track = -1;
        x = stod(row[1]);
        y = stod(row[2]);
        //if (row[10] != "0")
        //    associations.push_back();
        info = row;
        identifier = identify(row);
        track_id = stoi(row[0]);
    }
    GeneratedFeature(GeneratedFeature& cpy) {
        this->identifier = cpy.identifier;
        this->associations = cpy.associations;
        this->back_associations = cpy.back_associations;
        this->id = cpy.id;
        this->frame = cpy.frame;
        this->merge_track = cpy.merge_track;
        this->split_track = cpy.merge_track;
        this->track_id = cpy.track_id;
        this->info = cpy.info;
        this->association_weights = cpy.association_weights;
        this->ignore = cpy.ignore;
        this->x = cpy.x;
        this->y = cpy.y;
        this->adjx = cpy.adjx;
        this->adjy = cpy.adjy;
        this->adj = cpy.adj;



    }
    std::string identify(std::vector<std::string> row) {
        return row[0]+'|'+row[3] + '|' + row[1] + '|' + row[2];
        //return row[3] + '|' + row[1] + '|' + row[2];

    }

    std::string identifier; //frame +'|' +x+'|' +y
    std::vector<GeneratedFeature*> associations;
    std::vector<GeneratedFeature*> back_associations;

    int frame;
    int id;
    int merge_track;
    int split_track;
    int track_id;
    std::vector<std::string> info;
    std::vector<int> association_weights;
    bool ignore;
    int x;
    int y;
    double adjx;
    double adjy;
    bool adj;

};

// Example file to demonstrate how to output a one-parameter
// hierarchy as .family and .seg files
int main(int argc, const char* argv[])
{

    if (argc != 2) std::cout << "enter csv with 'frame#,x,y,int...' x dim y dim outpath" << std::endl;
    
    std::string fn = argv[1];

    float x_dim = stof(argv[2]);
    float y_dim = stof(argv[3]);

    std::string outpath = argv[4];

    //std::ifstream file(fn);

    //std::stringstream buffer;
    //if (file)
    //{

    //    buffer << file.rdbuf();

    //    file.close();

    //    // operations on the buffer...
    //}
    //std::string tmp;
    //vector<string> words;
    //char delim = ','; // Ddefine the delimiter to split by

    //while (std::getline(buffer, tmp, delim)) {
    //    // Provide proper checks here for tmp like if empty
    //    // Also strip down symbols like !, ., ?, etc.
    //    // Finally push it.
    //    //std::cout << tmp << std::endl;
    //    words.push_back(std::string(tmp));
    //}
    //std::ifstream file(fn);

    //std::stringstream buffer;
    //if (file)
    //{

    //    buffer << file.rdbuf();

    //    file.close();

    //    // operations on the buffer...
    //}
    //std::string tmp;
    //vector<string> words;
    //char delim = ','; // Ddefine the delimiter to split by

    //while (std::getline(buffer, tmp, delim)) {
    //    // Provide proper checks here for tmp like if empty
    //    // Also strip down symbols like !, ., ?, etc.
    //    // Finally push it.
    //    //std::cout << tmp << std::endl;
    //    words.push_back(std::string(tmp));
    //}



    std::map<int, std::vector<std::pair<float, float>>> frames;
    std::unordered_map<int, std::vector<float>> intensities;
    std::unordered_map<int, std::vector<float>> trackids;
    std::map<int, std::map<LocalIndexType, std::set<LocalIndexType >>> mTimeToEdges;
    std::map<int, std::vector< GeneratedFeature* > >gen_features; //frame to gen features
    std::map<int, std::vector<GeneratedFeature*>> tracks; //trackid to gen features
    std::set<std::string> added_features;

    // File pointer 
    fstream fin;

    // Open an existing file 
    fin.open(fn, std::ios::in);


    // Read the Data from the file 
    // as String Vector 
    std::vector<std::string> row;
    std::string line, word, temp;
    std::vector<std::string> prev_track;
    while (fin >> temp) {

        row.clear();

        // read an entire row and 
        // store it in a string variable 'line' 
        //std::getline(fin, line);

        // used for breaking words 
        stringstream s(temp);

        // read every column data of a row and 
        // store it in a string variable, 'word' 
        while (std::getline(s, word, ',')) {

            // add all the column data 
            // of a row to a vector 
            row.push_back(word);
        }
        if (row[3] == "3165")
            std::cout << "asdf" << std::endl;
        //for (int i = 0; i < row.size(); i ++) {
            //std::cout <<i<<" "<<(words[i]) << std::endl;
            //std::cout << stoi(words[i]) << std::endl;
            if (row[1] == "NaN") {
                std::vector < std::string> to_use = prev_track;
                to_use[3] = row[3];
                to_use[1] = "0.0";
                to_use[2] = "0.0";

                GeneratedFeature* new_feat= new GeneratedFeature(to_use);
                if (added_features.find(new_feat->identifier) == added_features.end()) {
                    added_features.insert(new_feat->identifier);
                    frames[stoi(to_use[3]) - 1].push_back(std::make_pair<float, float>(stof(to_use[1]), stof(to_use[2])));
                    intensities[stoi(to_use[3]) - 1].push_back(stof(to_use[4]));
                    trackids[stoi(to_use[3]) - 1].push_back(stof(to_use[0]));
                    new_feat->id = frames[stoi(to_use[3]) - 1].size() - 1;
                    gen_features[stoi(to_use[3]) - 1].push_back(new_feat);
                }

                tracks[stoi(row[0])].push_back(new_feat);
            }
            else {
                GeneratedFeature* new_feat = new GeneratedFeature(row);
                if (added_features.find(new_feat->identifier) == added_features.end()) {
                    added_features.insert(new_feat->identifier);
                    frames[stoi(row[3]) - 1].push_back(std::make_pair<float, float>(stof(row[1]), stof(row[2])));
                    intensities[stoi(row[3]) - 1].push_back(stof(row[4]));
                    trackids[stoi(row[3]) - 1].push_back(stof(row[0]));
                    new_feat->id = frames[stoi(row[3]) - 1].size() - 1;
                    gen_features[stoi(row[3]) - 1].push_back(new_feat);
                }

                tracks[stoi(row[0])].push_back(new_feat); //hhhmmmmmmmmmm
            }
            if (row[1] != "NaN") {
                prev_track = row;
            }


    }

    for (auto& track : tracks) {
        //std::cout << "for track " << track.first << std::endl;
        for (int i = 0; i < track.second.size() - 1; i++) {
            //std::cout << "    for frame " << track.second[i]->frame << std::endl;
            //if (track.first == 13855) {
            //    if (track.second[i]->frame == 3780) {
            //        //std::cout << 'a' << std::endl;
            //    }
            //}

            if (track.second[i + 1]->merge_track == -1) {
                track.second[i]->associations.push_back(track.second[i + 1]);
                
                //track.second[i + 1]->back_associations.push_back(track.second[i]);
                //int mf = track.second[track.second.size() - 1]->frame;
                //int sf = tracks[track.second[track.second.size() - 1]->merge_track][0]->frame;
                track.second[i]->association_weights.push_back(track.second.size()-i);//weight is length of track so far
            }
            else {
                int mf = track.second[i]->frame; //frame at which feature merges
                int sf = tracks[track.second[i+1]->merge_track][0]->frame;//start of track that feature is merging into
                //std::cout << "mf is " << mf << " sf is " << sf << " and index is " << mf - sf << std::endl;
                //std::cout << " track id " << track.first << " merge track is " << track.second[i + 1]->merge_track << " mf-sf+1 is " << mf - sf + 1 << std::endl;

                track.second[i]->associations.push_back(tracks[track.second[i+1]->merge_track][mf - sf+1]);
                //tracks[track.second[i + 1]->merge_track][mf - sf + 1]->back_associations.push_back(track.second[i]);
                //track.second[track.second.size() - 1]->association_weights.push_back(tracks[track.second[track.second.size() - 1]->merge_track].size()-(mf-sf));
                track.second[i]->association_weights.push_back((tracks[track.second[i + 1]->merge_track].size()));
                //std::cout << "       split frame is " << mf << " start of track " << sf << " mf-sf " << mf - sf << " frame association being added to " << tracks[track.second[i+1]->merge_track][mf - sf+1]->frame << std::endl;
                
            }

        }
        //merge, happens at end of track
        if (track.second[track.second.size() - 1]->merge_track != -1) { //when we encounter a merge, we want to add an association to merge_tag at frame number
           
        }
        //split
        if (track.second[0]->split_track != -1) { //we want to add an association to feature in track specified by 
            int mf = track.second[0]->frame; //frame at which split happened 58
            int sf = tracks[track.second[0]->split_track][0]->frame; //begginning of frame
            //std::cout << " track id "<<track.first<< " split track is " << track.second[0]->split_track << " mf-sf-1 is " <<mf - sf - 1 << std::endl;
            tracks[track.second[0]->split_track][mf-sf-1]->associations.push_back(track.second[0]);//go to merge track, append this feat to associations
            //track.second[0]->back_associations.push_back(tracks[track.second[0]->split_track][mf - sf - 1]);
            tracks[track.second[0]->split_track][mf - sf-1]->association_weights.push_back(tracks[track.second[0]->split_track].size());//go to merge track, append this feat to associations
            //std::cout << "       split frame is " << mf << " start of track " << sf << " mf-sf " << mf - sf <<" frame association being added to "<< tracks[track.second[0]->split_track][mf - sf-1]->frame<<std::endl;
        }
    }


    std::map<int, std::map<int, std::unordered_map<int, double>>> association_weight_map;
    std::map<int, std::map<int, std::vector<int>>> assocation_map;

    


    for (auto& frame : gen_features) {
                //std::cout << "for timestep "<<frame.first << std::endl;
        for (auto& ft : frame.second) {
            //std::cout << "for feature " << ft->id << std::endl;
            if (ft->ignore)
                continue;
            int i = 0;
            for (auto& assoc : ft->associations) {
                //std::cout << "adding association to " << assoc->id << " ";
                assocation_map[frame.first][ft->id].push_back(assoc->id);
                if (assoc->track_id == ft->track_id) {
                    association_weight_map[frame.first][ft->id][assoc->id] = 1;
                }
                else {
                    association_weight_map[frame.first][ft->id][assoc->id] = .1;
                }
                i++;
            }
            if (ft->associations.size() > 1) {
                /*int max = 0;
                int max_id = -1;
                for (auto& assoc : ft->associations) {
                    int tlength = tracks[assoc->track_id].size();
                    if (tlength > max) {
                        max = tlength;
                        max_id = assoc->id;
                    }
                }*/
                //for (auto& assoc : ft->associations) {
                //    //association_weights[frame.first][ft->id][assoc->id] = tracks[assoc->track_id].size();
                //}
            }
            //std::cout<<std::endl;
        }
    }

    //return 0;

    // Data in the topology file format is ultimately stored in types
    // derived from FileData. Most often these end up being std::vectors

    // Lets define a hierarchy of 7 nodes. Note that we tell the constructor that all
    // our features will have a single representative/parent which is what we normally
    // encounter (everybody has a single parent but maybe multiple children)
    int time = 0;

    for (auto& step : frames) {

        std::string filename = outpath+ std::string("/TestOutput_") + std::string(std::to_string(step.first)) + std::string(".family");

        int feature_count = step.second.size();
        int seg_count = step.second.size();

        vector<FeatureElement> features(step.second.size(), FeatureElement(SINGLE_REPRESENTATIVE));


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
        for (int I = 0; I < step.second.size(); I++) {
            features[I].lifeTime(0, 1);
            features[I].direction(true);
          /*  features[I + seg_count].lifeTime(1,1);
            features[I + seg_count].direction(true);
            features[I].addLink(I + seg_count);*/
            //std::cout << I + seg_count << std::endl;

        }
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

        // and the family to the clan

        // And finally we write the clan to file


        // Now let's figure out how to add some geometric points to act as elements of each cluster

        //std::cout << "Num features is " << step.second.size() << std::endl;
        // First we create a 10x10 grid of points (standard row major layout)
        vector<FunctionType> coords(step.second.size() * 2);

        for (int j = 0; j < step.second.size(); j++) {
            //for (int i = 0; i < 10; i+=2) {
            coords[2  * j] = step.second[j].first;     // x-coord
            coords[2 * j + 1] = step.second[j].second; // y-coord

            //coords[2 * 5 * j + 2] = step.second[j].first - 1; // x-coord
            //coords[2 * 5 * j + 3] = step.second[j].second - 1; // y-coord

            //coords[2 * 5 * j + 4] = step.second[j].first + 1; // x-coord
            //coords[2 * 5 * j + 5] = step.second[j].second + 1; // y-coord

            //coords[2 * 5 * j + 6] = step.second[j].first + 1; //x-coord
            //coords[2 * 5 * j + 7] = step.second[j].second - 1; // y-coord

            //coords[2 * 5 * j + 8] = step.second[j].first - 1; // x-coord
            //coords[2 * 5 * j + 9] = step.second[j].second + 1; // y-coord
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
        vector<vector<GlobalIndexType> > segments(step.second.size());


        for (int i = 0; i < step.second.size() ; i++) {
            segments[i].resize(1);
            segments[i][0] = (i) * 2;
            /*segments[i][1] = (i) * 10 + 1 * 2;
            segments[i][2] = (i) * 10 + 2 * 2;
            segments[i][3] = (i) * 10 + 3 * 2;
            segments[i][4] = (i) * 10 + 4 * 2;*/

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
        sprintf(domain_description, "2 %f %f %f %f ", 0.0, x_dim, 0.0, y_dim);
        segmentation.domainDescription(domain_description);


        segmentation.setSegmentation(&segments);

        //Now we add the geometry to the segmentation
        segmentation.add(geometry);


        Statistics::Factory gAggFactory;
        std::vector<std::string> agg_name{ "mean","min","max","vertexCount","sum" };
        std::vector<Statistics::Attribute* > values;
        values.resize(agg_name.size());
        for (int i = 0; i < agg_name.size(); i++) {
            values[i] = gAggFactory.make_aggregator(agg_name[i]);
        }
       std::vector<Statistics::Attribute*>::iterator aIt;
       for (aIt = values.begin(); aIt != values.end(); aIt++)
           (*aIt)->resize(feature_count);


        for (int k = 0; k < feature_count; k++) {
            for (int j = 0; j < segments[k].size(); j++) {

                // Only if this vertex is assigned to a feature do we need to consider it


                    /*sterror(segments[k] >= feature_count, "Number of features was supposed to be %llu but we found a segmentation index %llu. Did you use --raw-segmentation ?.",
                        (uint64_t)(feature_count), (uint64_t)segments[k]);*/

                for (int i = 0; i < values.size(); i++) {
                    if (values[i]->numVariables() == 1) {
                            //(*values[i])[k].addVertex(intensities[step.first][k], k); //uncomment for normal
                            (*values[i])[k].addVertex(trackids[step.first][k], k); //uncomment for xiyu format

                    }
                   /* else if (values[i]->numVariables() == 1) {
                        (*values[i])[segments[k][j]].addVertex(0, k * j + j);
                    }*/
                    /* else {
                         (*values[i])[segments[k]].addVertex(attributes[attribute_index[i][0]]->at(k),
                             attributes[attribute_index[i][1]]->at(k), k);
                     }*/
                }
            }
            
        }

        
        TopologyFileFormat::StatHandle handle;

        handle.encoding(true);
        handle.aggregated(true);
        /*if (aggregate)
            handle.aggregated(true);*/
            clan.add(family);
            clan.family(0).add(simplification);

        //family.add(simplification);

        for (uint8_t i = 0; i < values.size(); i++) {

            handle.setData(values[i]);
            handle.stat(std::string(values[i]->typeName()));

            std::string species("");
            /*for (uint8_t k = 0; k < values[i]->numVariables(); k++)
                species += values[i]->variableName(k);
            handle.species(species);*/

            clan.family(0).add(handle);
            
        }


        //// found
        //FamilyHandle family = collection.findFamilyHandle(fKey);

        //// If we could not find this handle return NULL
        //if (!family.valid())
        //    return Visus::SharedPtr<TopologyFileFormat::FeatureHierarchy>();


        //// If the metric is not found
        //if (!family.providesSimplification(metric))
        //    return Visus::SharedPtr<TopologyFileFormat::FeatureHierarchy>();

        //// Otherwise, get the corresponding handle
        //SimplificationHandle handle = family.simplification(metric);

        // Instantiate the corresponding hierarchy
      

        clan.write();

        filename = outpath+std::string("/TestOutput_") + std::string(std::to_string(step.first)) + std::string(".seg");

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

        //TopologyFileFormat::FeatureHierarchy* hierarchy = new TopologyFileFormat::FeatureHierarchy();
        //hierarchy->initialize(simplification);
        //FeatureHierarchy::LivingIterator it;
        //hierarchy->parameter(.1);
        //TopologyFileFormat::FeatureSegmentation* fsegmentation = new TopologyFileFormat::FeatureSegmentation();
        //fsegmentation->initialize(clanseg.family(0).segmentation());
        //for (it = hierarchy->beginLiving(0); it != hierarchy->endLiving(); it++) {
        //    std::cout << "id is " << it->id() << std::endl;
        //    TopologyFileFormat::Segment seg;
        //    seg = fsegmentation->elementSegmentation(it->id());
        //    std::cout << "seg size is " << seg.size << std::endl;
        //    for (uint64_t i = 0; i < seg.size; i++) {
        //        //point.pos = Visus::Point3d(seg.coordinates[2 * i], seg.coordinates[2 * i + 1], 0);
        //        std::cout << "seg coords " << seg.coordinates[2 * i] << " " << seg.coordinates[2 * i + 1] << std::endl;
        //    }
        //}


        time++;
        for (int i = 0; i < values.size(); i++) {
            delete values[i];
        }
        // We are done ... have a look at the bottom of the file to better understand what is happening
    }


    for (auto&frame :frames) {

        std::vector<std::string> filepaths;
        filepaths.push_back( outpath + std::string("/TestOutput_") + std::string(std::to_string(frame.first)) + std::string(".family"));
        filepaths.push_back(outpath + std::string("/TestOutput_") + std::string(std::to_string(frame.first +1)) + std::string(".family"));
        filepaths.push_back(outpath + std::string("/TestOutput_") + std::string(std::to_string(frame.first)) + std::string(".seg"));
        filepaths.push_back(outpath + std::string("/TestOutput_") + std::string(std::to_string(frame.first +1)) + std::string(".seg"));

        //! The meta data store
        TopologyFileFormat::HandleCollection collection;

        // The set of all clan keys we have found
        std::vector<TopologyFileFormat::ClanKey> clans;

        //! The set of available time indices for each clan
        std::map<TopologyFileFormat::ClanKey, std::set<int> > timesteps;
        collection.initialize(filepaths);

        collection.getClanKeys(clans);

        std::vector<TopologyFileFormat::FamilyKey> keys;
        std::vector<TopologyFileFormat::FamilyKey>::iterator kIt;
        std::vector<TopologyFileFormat::ClanKey>::iterator it;

        std::vector<TopologyFileFormat::AssociationKey> aKeys;
        std::string filename;



        for (it = clans.begin(); it != clans.end(); it++) {

            collection.getFamilyKeys(*it, keys);
            collection.getAssociationKeys(*it, aKeys);
            //break;

            // AssociationHandle association =

            for (kIt = keys.begin(); kIt != keys.end() - 1; kIt++) {
                //t.insert(kIt->timeIndex());
                //std::cout<<kIt->timeIndex()<<std::endl;

                FamilyHandle family1 = collection.findFamilyHandle(*kIt);

                //kIt++; //get next family
                FamilyHandle family2 = collection.findFamilyHandle(*(kIt + 1));

                //note for monday: get the heigherarchy/segmentation and then pass that into the compute correlation. then attatch edge handle to asscociation handle and update clan+
                for (int i = 0; i < family1.numSimplifications(); i++) {

                    filename = collection.getFileFromFamily(*kIt);

                    //std::cout<<"dataset is "<<dataset<<std::endl;
                    filename = filename.substr(0, filename.find(".fam")) + std::string(".family");

                    // std::string filename= family1.mFileName;
                    //std::cout << "attatching clan to " << filename << std::endl;
                    ClanHandle clan;
                    clan.attach(filename);

                    //clan.dataset(dataset);
                    //clan.write();


                    SimplificationHandle handle1 = family1.simplification(i);
                    //std::cout<<"num simps is "<<family1.numSimplifications()<<std::endl;
                    Data<FeatureElement> data;
                    handle1.readData(data);
                    // Instantiate the corresponding hierarchy
                    TopologyFileFormat::FeatureHierarchy* hierarchy1 = new TopologyFileFormat::FeatureHierarchy();
                    hierarchy1->initialize(handle1);

                    SegmentationHandle seg1 = family1.segmentation();
                    TopologyFileFormat::FeatureSegmentation* segmentation1 = new TopologyFileFormat::FeatureSegmentation();
                    segmentation1->initialize(seg1);

                    for (int j = 0; j < family2.numSimplifications(); j++) {
                        SimplificationHandle handle2 = family2.simplification(j);

                        // Instantiate the corresponding hierarchy
                        TopologyFileFormat::FeatureHierarchy* hierarchy2 = new TopologyFileFormat::FeatureHierarchy();
                        hierarchy2->initialize(handle2);

                        SegmentationHandle seg2 = family2.segmentation();
                        TopologyFileFormat::FeatureSegmentation* segmentation2 = new TopologyFileFormat::FeatureSegmentation();
                        segmentation2->initialize(seg2);

                        //now construct association handle

                        AssociationHandle association;

                        SimplificationKey sk1(handle1, *kIt);
                        SimplificationKey sk2(handle2, *(kIt + 1));

                        association.source(sk1);
                        association.destination(sk2);


                        AssociationKey a_key(sk1, sk2);
                        //std::cout << "for association key " << a_key.source().familyKey().timeIndex() << " " << a_key.destination().familyKey().timeIndex() << "there are this many associations: " << clan.numAssociations() << std::endl;
                        if (clan.numAssociations() == 0) {


                            ComputeFamilyCorrelation* correlation = new ComputeFamilyCorrelation(hierarchy1, hierarchy2, segmentation1, segmentation2, VERTEX_OVERLAP);
                            correlation->computeCorrelations();
                            //edge handle
                            EdgeHandle edge;
                            edge.setData(correlation);
                            edge.encoding(true);
                            edge.setTrackingType(VERTEX_OVERLAP);

                            ComputeFamilyCorrelation* correlationdist = new ComputeFamilyCorrelation(hierarchy1, hierarchy2, segmentation1, segmentation2, DISTANCE, 3);
                            correlationdist->computeCorrelations();
                            //edge handle
                            EdgeHandle edgedist;
                            edgedist.setData(correlationdist);
                            edgedist.encoding(true);
                            edgedist.setTrackingType(DISTANCE);

                            ComputeFamilyCorrelation* correlationgiven = new ComputeFamilyCorrelation(hierarchy1, hierarchy2, segmentation1, segmentation2, GIVEN);
                            correlationgiven->assocation_map = assocation_map[frame.first];
                            correlationgiven->association_weights = association_weight_map[frame.first];

                            correlationgiven->computeCorrelations();
                            //edge handle
                            EdgeHandle edgegiven;
                            edgegiven.setData(correlationgiven);
                            edgegiven.encoding(true);
                            edgegiven.setTrackingType(GIVEN);


                            //std::cout << "writing to file" << std::endl;
                            association.add(edgegiven);
                            association.add(edge);
                            association.add(edgedist);

                            //clan.append(association);

                            std::ofstream ofs;
                            ofs.open(filename, std::ofstream::out | std::ofstream::trunc);
                            ofs.close();
                            ClanHandle clanw(filename);
                            clanw.dataset("Rebika-SPT");

                            // Create the family which is the internal name for the collection
                            // of all the information related to a time step
                            FamilyHandle family;

                            // The index of this time step
                            family.timeIndex(family1.timeIndex());

                            // The actual floating point real time. Often this is identical
                            // to the index but it does not have to be. Also this allows to
                            // accept time series with unevenly distributed time steps
                            family.time(family1.time());

                            // The name of the threshold parameter we are using
                            family.variableName("ParameterName");

                            // The range of the parameter
                            family.range(0, 1);
                            SimplificationHandle simplification;
                            simplification.setData(&data);
                            simplification.metric("Threshold");
                            simplification.fileType(SINGLE_REPRESENTATIVE);
                            simplification.setRange(0, 1);
                            simplification.encoding(true); // For testing purposes we write this in ascii
                            clanw.add(family);
                            clanw.family(0).add(simplification);
                            clanw.add(association);
                            clanw.write();

                            //ClanHandle tclan;
                            //tclan.attach(filename);


                        }
                    }

                }
                //////////////////////uncomment for double checking written data//////////////////////////////
                        //ClanHandle handle;
                        //handle.attach(filename);
                        //std::cout << " there are this many associations for this file " << handle.numAssociations() << std::endl;

                        //if (handle.numAssociations() > 0) {
                        //	AssociationHandle a = handle.association(0);
                        //	std::cout << "for association key " << a.source().familyKey().timeIndex() << " " << a.destination().familyKey().timeIndex() << " and filename is " << a.getFileName() << std::endl;
                        //	std::cout << "loaded association handle" << std::endl;
                        //	EdgeHandle e;
                        //	e = a.edges();
                            ///*TopologyFileFormat::FamilyCorrelation* corr = new TopologyFileFormat::FamilyCorrelation();
                            //corr->initialize(e);*/
                        //	std::cout << "loaded edge handle with feat count " << e.featureCount() << " and filename " << e.getFileName() << std::endl;
                        //	std::vector<LocalIndexType> offsets;
                        //	e.readOffsets(offsets);
                        //	std::cout << "offset size is " << offsets.size() << " "<< offsets[offsets.size()-1]<<std::endl;
                        //	std::vector<LocalIndexType> neighbors;
                        //	e.readNeighbors(neighbors);
                        //	std::cout << "neighbor size is " << neighbors.size() << std::endl;
                            //std::vector<double> weights;

                            //e.readWeights(weights);
                            //std::cout << "weights size is " << weights.size() << std::endl;
                            //std::cout << "//////////////////////////////" << std::endl;
                        //}
            }
        }
    }

    std::cout << "FINISHED WITH NO ERRORS" << std::endl;



    return 1;
}
