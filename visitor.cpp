// vim: ts=2 sw=2 smarttab

#include <vector>

#include "visitor.h"


using namespace cohort;


// visit all nodes associated with blocks in range [dstart, dend]
bool visitor::visit(uint64_t dstart, uint64_t dend, uint64_t maxblocks)
{
	uint64_t leaves = maxblocks / tree.k +
		(maxblocks % tree.k ? 1 : 0);
	uint8_t maxdepth = tree.depth(leaves);

	// allocate the state stack, whose size is bounded by maxdepth
	std::vector<struct state> stack;
	try {
		stack.resize(maxdepth);
	} catch (const std::bad_alloc&) {
		return false;
	}

	// initialize the root node
	struct state &root = stack[maxdepth-1];
	root.node = tree.root(maxdepth);
	root.parent = -1ULL;
	root.bstart = 0;
	root.bend = maxblocks;
	root.dstart = dstart;
	root.dend = dend + 1;
	root.dirty = 0;
	root.progress = 0;
	root.position = 0;

	// precalculate cnodes and cleaves for each level
	stack[0].cnodes = 0;
	stack[0].cleaves = 1;
	for (uint8_t i = 1; i < maxdepth; i++)
	{
		stack[i].cnodes = stack[i-1].cnodes * tree.k + 1;
		stack[i].cleaves = stack[i-1].cleaves * tree.k;
	}

	// start traversal at the root node
	uint8_t depth = maxdepth;
	while (depth <= maxdepth)
	{
		struct state &node = stack[depth-1];

		// base case: visit each file block of the leaf node
		if (depth == 1)
		{
			for (; node.dstart < node.dend; node.dstart++)
			{
				if (!visit_leaf(node.dstart, node.dstart - node.bstart, node))
					return false;
			}

			// traverse back up to parent node
			depth++;
		}
		else
		{
			struct state &child = stack[depth-2];
			child.parent = node.node;
			child.progress = 0;
			child.dirty = 0;

			if (node.progress == tree.k)
			{
				// all child nodes have been processed
				for (child.position = 0; child.position < tree.k; child.position++)
				{
					// calculate hash for any dirty children
					if (node.dirty & (1ULL << child.position))
					{
						child.node = tree.child_index(child.parent,
								child.position, node.cnodes, child.cnodes);

						child.bstart = node.bstart + child.position * node.cleaves;
						child.bend = std::min(child.bstart + node.cleaves, node.bend);

						if (!visit_node(child, depth - 1))
							return false;
					}
				}

				// traverse back up to parent node
				depth++;
				continue;
			}

			// consider each child node. remember our progress
			// so we can traverse a single child, instead of having
			// to push them all to the stack at once
			while (node.progress < tree.k) 
			{
				child.position = node.progress++;

				// calculate which blocks are under this node
				child.bstart = node.bstart + child.position * node.cleaves;
				child.bend = std::min(child.bstart + node.cleaves, node.bend);

				// intersect with the parent's dirty block range
				child.dstart = std::max(child.bstart, node.dstart);
				child.dend = std::min(child.bend, node.dend);

				if (child.dstart < child.dend)
				{
					// traverse down to child node
					child.node = tree.child_index(child.parent,
							child.position, node.cnodes, child.cnodes);
					depth--;
					node.dirty |= (1ULL << child.position);
					break;
				}
			}
		}
	}

	// final hash of root node
	return visit_node(root, depth);
}
