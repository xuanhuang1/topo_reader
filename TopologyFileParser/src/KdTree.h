#ifndef KDTREE
#define KDTREE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
namespace TopologyFileFormat {

	struct kdtree;
	struct kdres;


	/* create a kd-tree for "k"-dimensional data */
	struct kdtree* kd_create(int k);

	/* free the struct kdtree */
	void kd_free(struct kdtree* tree);

	/* remove all the elements from the tree */
	void kd_clear(struct kdtree* tree);

	/* if called with non-null 2nd argument, the function provided
	* will be called on data pointers (see kd_insert) when nodes
	* are to be removed from the tree.
	*/
	void kd_data_destructor(struct kdtree* tree, void (*destr)(void*));

	/* insert a node, specifying its position, and optional data */
	int kd_insert(struct kdtree* tree, const double* pos, void* data);
	int kd_insertf(struct kdtree* tree, const float* pos, void* data);
	int kd_insert3(struct kdtree* tree, double x, double y, double z, void* data);
	int kd_insert3f(struct kdtree* tree, float x, float y, float z, void* data);

	/* Find the nearest node from a given point.
	*
	* This function returns a pointer to a result set with at most one element.
	*/
	struct kdres* kd_nearest(struct kdtree* tree, const double* pos);
	struct kdres* kd_nearestf(struct kdtree* tree, const float* pos);
	struct kdres* kd_nearest3(struct kdtree* tree, double x, double y, double z);
	struct kdres* kd_nearest3f(struct kdtree* tree, float x, float y, float z);

	/* Find the N nearest nodes from a given point.
	*
	* This function returns a pointer to a result set, with at most N elements,
	* which can be manipulated with the kd_res_* functions.
	* The returned pointer can be null as an indication of an error. Otherwise
	* a valid result set is always returned which may contain 0 or more elements.
	* The result set must be deallocated with kd_res_free after use.
	*/
	struct kdres* kd_nearest_n(struct kdtree* tree, const double* pos, int num);
	struct kdres* kd_nearest_nf(struct kdtree* tree, const float* pos, int num);
	struct kdres* kd_nearest_n3(struct kdtree* tree, double x, double y, double z);
	struct kdres* kd_nearest_n3f(struct kdtree* tree, float x, float y, float z);

	/* Find any nearest nodes from a given point within a range.
	*
	* This function returns a pointer to a result set, which can be manipulated
	* by the kd_res_* functions.
	* The returned pointer can be null as an indication of an error. Otherwise
	* a valid result set is always returned which may contain 0 or more elements.
	* The result set must be deallocated with kd_res_free after use.
	*/
	struct kdres* kd_nearest_range(struct kdtree* tree, const double* pos, double range);
	struct kdres* kd_nearest_rangef(struct kdtree* tree, const float* pos, float range);
	struct kdres* kd_nearest_range3(struct kdtree* tree, double x, double y, double z, double range);
	struct kdres* kd_nearest_range3f(struct kdtree* tree, float x, float y, float z, float range);

	/* frees a result set returned by kd_nearest_range() */
	void kd_res_free(struct kdres* set);

	/* returns the size of the result set (in elements) */
	int kd_res_size(struct kdres* set);

	/* rewinds the result set iterator */
	void kd_res_rewind(struct kdres* set);

	/* returns non-zero if the set iterator reached the end after the last element */
	int kd_res_end(struct kdres* set);

	/* advances the result set iterator, returns non-zero on success, zero if
	* there are no more elements in the result set.
	*/
	int kd_res_next(struct kdres* set);

	/* returns the data pointer (can be null) of the current result set item
	* and optionally sets its position to the pointers(s) if not null.
	*/
	void* kd_res_item(struct kdres* set, double* pos);
	void* kd_res_itemf(struct kdres* set, float* pos);
	void* kd_res_item3(struct kdres* set, double* x, double* y, double* z);
	void* kd_res_item3f(struct kdres* set, float* x, float* y, float* z);

	/* equivalent to kd_res_item(set, 0) */
	void* kd_res_item_data(struct kdres* set);





	struct kdhyperrect {
		int dim;
		double* min, * max;              /* minimum/maximum coords */
	};

	struct kdnode {
		double* pos;
		int dir;
		void* data;

		struct kdnode* left, * right;	/* negative/positive side */
	};

	struct res_node {
		struct kdnode* item;
		double dist_sq;
		struct res_node* next;
	};

	struct kdtree {
		int dim;
		struct kdnode* root;
		struct kdhyperrect* rect;
		void (*destr)(void*);
	};

	struct kdres {
		struct kdtree* tree;
		struct res_node* rlist, * riter;
		int size;
	};

#define SQ(x) (x * x)


	static void clear_rec(struct kdnode* node, void (*destr)(void*));
	static int insert_rec(struct kdnode** node, const double* pos, void* data, int dir, int dim);
	static int rlist_insert(struct res_node* list, struct kdnode* item, double dist_sq);
	static void clear_results(struct kdres* set);

	static struct kdhyperrect* hyperrect_create(int dim, const double* min, const double* max);
	static void hyperrect_free(struct kdhyperrect* rect);
	static struct kdhyperrect* hyperrect_duplicate(const struct kdhyperrect* rect);
	static void hyperrect_extend(struct kdhyperrect* rect, const double* pos);
	static double hyperrect_dist_sq(struct kdhyperrect* rect, const double* pos);



#define alloc_resnode()		malloc(sizeof(struct res_node))
#define free_resnode(n)		free(n)




}


#endif
