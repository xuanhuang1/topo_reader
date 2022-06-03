/*
	PURPOSE: add mean position for each feature
*/


#include <cstdio>
#include <cstdlib>

#include "AggregatorFactory.h"
#include "Attribute.h"
#include "ClanHandle.h"
#include "FeatureHierarchy.h"
#include "FeatureSegmentation.h"
#include "StatHandle.h"
#include "TalassConfig.h"


static const char *stat_names[] = {
	"mean x position",
	"mean y position",
	"mean z position"
};


using namespace TopologyFileFormat;
using namespace Statistics;


int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: ./program <family> <segmentation>\n");
		exit(EXIT_FAILURE);
	}

	ClanHandle clan, sclan;

	// open .family file
	clan.attach(argv[1]);
	FamilyHandle family_handle = clan.family(0);
	FeatureHierarchy hierarchy;
	hierarchy.initialize(family_handle.simplification(0));

	// open .seg file
	sclan.attach(argv[2]);
	SegmentationHandle seg_handle = sclan.family(0).segmentation();
	FeatureSegmentation seg;
	seg.initialize(seg_handle);

	Data<float> stats[3];
	for (int i = 0; i != 3; ++i)
		stats[i].resize(hierarchy.featureCount());

	LocalIndexType feature_count = 0;
	std::vector<Feature>::const_iterator it;

	for (it = hierarchy.allFeatures().begin(); it != hierarchy.allFeatures().end(); ++it) {
		std::vector<Segment> segments;
		// TODO: aggregate instead using all constituent vertices
		seg.segmentation(&(*it), segments);


		float mean_position[3] = {0.0f, 0.0f, 0.0f};

		// mean position of all vertices
		// TODO how to get coordinates without going through segments?
		std::vector<Segment>::const_iterator seg_it;
		LocalIndexType vertex_count = 0;
		for (seg_it = segments.begin(); seg_it != segments.end(); ++seg_it) {
			// now go through all vertices in segment
			for (LocalIndexType i = 0; i != seg_it->size;++i) {
				for (int j = 0; j != 3; ++j)
					mean_position[j] += seg_it->coordinates[i*seg_it->dim + j];

				++vertex_count;
			}	
		}

		// each feature has at least one vertex (no division by zero)
		for (int i = 0; i != 3; ++i)
			stats[i][feature_count] = mean_position[i]/vertex_count;

		++feature_count;
	}


	// append stats to .family file
	for (int i = 0; i != 3; ++i) {
		StatHandle stat_handle;
		stat_handle.aggregated(true);

		// TODO: what is "sum"?
		stat_handle.stat("sum");

		stat_handle.species(stat_names[i]);
		stat_handle.encoding(false);
		stat_handle.setData(&stats[i]);

		// TODO we can't use family_handle (why??, does it have some weird dependency
		// pointer to clan?)
		clan.family(0).append(stat_handle);
	}
}
