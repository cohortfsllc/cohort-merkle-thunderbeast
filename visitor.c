#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "visitor.h"
#include "tree.h"


#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static void child_bounds(const struct merkle_state *node,
		struct merkle_state *child)
{
	/* calculate which blocks are under this child */
	child->bstart = node->bstart + child->position * node->cleaves;
	child->bend = min(child->bstart + node->cleaves, node->bend);

	/* intersect with the parent's requested block range */
	child->dstart = max(child->bstart, node->dstart);
	child->dend = min(child->bend, node->dend);
}

/* visit all nodes associated with blocks in given range */
int merkle_visit(const struct merkle_visitor *visitor, uint8_t k,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks)
{
	uint64_t i, leaves = total_blocks / k +
		(total_blocks % k ? 1 : 0);
	struct merkle_state *stack, *node, *child;
	uint8_t depth, maxdepth = merkle_depth(k, leaves);
	int status;

	/* allocate the state stack, whose size is bounded by maxdepth */
	stack = (struct merkle_state*)calloc(maxdepth,
			sizeof(struct merkle_state));
	if (stack == NULL) {
		status = errno;
		fprintf(stderr, "merkle_visit() failed to allocate stack\n");
		goto out;
	}

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
	node->parent = -1ULL;
	node->bend = total_blocks;
	node->dstart = from_block;
	node->dend = to_block + 1;

	/* start traversal at the root node */
	depth = maxdepth;
	while (depth <= maxdepth) {
		node = &stack[depth-1];

		/* base case: visit each requested file block of the leaf node */
		if (depth == 1) {
			for (i = node->dstart; i < node->dend; i++) {
				status = visitor->visit_leaf(node, i,
						i - node->bstart, visitor->user);
				if (status)
					goto out_free;
			}

			/* traverse back up to parent node */
			depth++;
		} else {
			child = &stack[depth-2];
			child->parent = node->node;
			child->progress = 0;

			if (node->progress == k) {
				/* all children have been processed */
				for (child->position = 0; child->position < k; child->position++) {
					/* visit any requested children */
					child_bounds(node, child);
					if (child->dstart >= child->dend)
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
				child_bounds(node, child);
				if (child->dstart >= child->dend)
					continue;

				/* traverse down to child node */
				child->node = merkle_child(child->parent,
						child->position, node->cnodes, child->cnodes);
				depth--;
				break;
			}
		}
	}

	/* visit root node */
	status = visitor->visit_node(node, depth, visitor->user);
out_free:
	free(stack);
out:
	return status;
}
