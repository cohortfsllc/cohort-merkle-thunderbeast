#ifndef COHORT_MERKLE_VISITOR_H
#define COHORT_MERKLE_VISITOR_H

#include <stdint.h>


/* node state passed to merkle_visitor callbacks */
struct merkle_state
{
	uint64_t node, parent; /* index of the node and its parent */
	uint64_t bstart, bend; /* bounds of all blocks under this node */
	uint64_t cnodes; /* number of nodes under each child */
	uint64_t cleaves; /* number of leaves under each child */
	uint8_t progress; /* number of children processed [0, k] */
	uint8_t position; /* child's position under parent node [0, k-1] */
};

/* visitor interface */
struct merkle_visitor
{
	/* callbacks functions for visitor implementation */
	int (*visit_leaf)(const struct merkle_state *node, uint64_t block,
			uint8_t position, void *user);
	int (*visit_node)(const struct merkle_state *node,
			uint8_t depth, void *user);
	int (*visit_root)(const struct merkle_state *node,
			uint8_t depth, void *user);

	void *user; /* user data passed to each callback */
};

/* perform a depth-first postorder traversal of the tree, ignoring
 * nodes that aren't in the requested block range. total_blocks is
 * required to locate the root node */
int merkle_visit(const struct merkle_visitor *visitor, uint8_t k,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks);

#endif /* COHORT_MERKLE_VISITOR_H */
