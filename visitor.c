#include <stdlib.h>
#include <errno.h>

#include "visitor.h"
#include "tree.h"


/* find the leaf node index that corresponds to the given block */
static uint64_t find_leaf(struct merkle_state *stack,
		uint8_t k, uint64_t block, uint64_t depth)
{
	struct merkle_state *node, *child;

	/* start traversal at the root node */
	for (;;) {
		node = &stack[depth-1];
		if (depth == 1) /* found the leaf node */
			return node->node;

		child = &stack[depth-2];
		child->parent = node->node;

		/* choose the child that contains this block */
		child->position = (block - node->bstart) / node->cleaves;
		child->bstart = node->bstart + child->position * node->cleaves;
		child->node = merkle_child(child->parent,
				child->position, node->cnodes, child->cnodes);
		depth--;
	}
}

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

/* returns nonzero when the node intersects the given block bounds */
static inline int node_in_bounds(const struct merkle_state *node,
		uint64_t from, uint64_t to)
{
	return max(node->bstart, from) < min(node->bend, to + 1);
}

/* visit all nodes associated with blocks in given range */
int merkle_visit(const struct merkle_visitor *visitor, uint8_t k,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks)
{
	uint64_t i, leaves;
	struct merkle_state *stack, *node, *child;
	uint8_t depth, maxdepth;
	int status;

	/* calculate the depth required to hold total_blocks */
	leaves = total_blocks / k + (total_blocks % k ? 1 : 0);
	maxdepth = merkle_depth(k, leaves);

	/* allocate the state stack, whose size is bounded by maxdepth */
	stack = (struct merkle_state*)malloc(maxdepth *
			sizeof(struct merkle_state));
	if (stack == NULL)
		return errno;

	/* precalculate cnodes and cleaves */
	stack[0].cnodes = 0;
	stack[0].cleaves = 1;
	for (i = 1; i < maxdepth; i++) {
		stack[i].cnodes = stack[i-1].cnodes * k + 1;
		stack[i].cleaves = stack[i-1].cleaves * k;
	}

	/* initialize the root node */
	node = &stack[maxdepth-1];
	node->node = stack[maxdepth-1].cnodes;
	node->bstart = 0;
	node->bend = total_blocks;
	node->position = 0;
	node->progress = 0;

	/* the root's parent is the last leaf node + 1 */
	node->parent = find_leaf(stack, k, total_blocks - 1, maxdepth) + 1;

	/* start traversal at the root node */
	depth = maxdepth;
	while (depth <= maxdepth) {
		node = &stack[depth-1];

		/* base case: visit each requested file block of the leaf node */
		if (depth == 1) {
			uint64_t end = min(node->bend, to_block + 1);
			for (i = max(node->bstart, from_block); i < end; i++) {
				status = visitor->visit_leaf(node, i,
						i - node->bstart, visitor->user);
				if (status)
					goto out_free;
			}

			/* traverse back up to parent node */
			depth++;
			continue;
		}

		child = &stack[depth-2];
		child->parent = node->node;
		child->progress = 0;

		if (node->progress == k) {
			/* all children have been traversed */
			for (child->position = 0; child->position < k; child->position++) {
				/* calculate which blocks are under this child */
				child->bstart = node->bstart + child->position * node->cleaves;
				child->bend = min(child->bstart + node->cleaves, node->bend);

				/* visit children in the requested range */
				if (!node_in_bounds(child, from_block, to_block))
					continue;

				child->node = merkle_child(child->parent,
						child->position, node->cnodes, child->cnodes);

				status = visitor->visit_node(child,
						depth-1, visitor->user);
				if (status)
					goto out_free;
			}

			/* traverse back up to parent node */
			depth++;
			continue;
		}

		/* consider each child node. remember our progress
		 * so we can traverse a single child, instead of having
		 * to push them all to the stack at once */
		while (node->progress < k)
		{
			child->position = node->progress++;

			/* calculate which blocks are under this child */
			child->bstart = node->bstart + child->position * node->cleaves;
			child->bend = min(child->bstart + node->cleaves, node->bend);

			if (!node_in_bounds(child, from_block, to_block))
				continue;

			/* traverse down to child node */
			child->node = merkle_child(child->parent,
					child->position, node->cnodes, child->cnodes);
			depth--;
			break;
		}
	}

	/* visit root node */
	status = visitor->visit_node(node, depth, visitor->user);
out_free:
	free(stack);
	return status;
}
