// vim: ts=2 sw=2 smarttab

#include <vector>

#include "visitor.h"


using namespace cohort;


// visit all nodes associated with blocks in range [dstart, dend]
bool visitor::visit(uint64_t dstart, uint64_t dend, uint64_t maxdepth)
{
	// allocate the state stack, whose size is bounded by maxdepth
	std::vector<struct state> stack;
	stack.resize(maxdepth);

	// initialize the root node
	struct state &root = stack[maxdepth-1];
	root.node = tree.root(maxdepth);
	root.parent = root.node + 1;
	root.bstart = 0;
	root.bend = tree.leaves(maxdepth) - 1;
	root.dstart = dstart;
	root.dend = dend + 1;
	root.dirty = 0;
	root.progress = 0;
	root.position = 0;

	// precalculate.cnodes and.cleaves for each level,
	// instead of redoing them at each node traversed
	stack[0].cnodes = 0;
	stack[0].cleaves = 1;
	for (int i = 1; i < maxdepth; i++)
	{
		stack[i].cnodes = stack[i-1].cnodes * tree.k + 1;
		stack[i].cleaves = stack[i-1].cleaves * tree.k;
	}

	// start traversal at the root node
	int depth = maxdepth;
	while (depth <= maxdepth)
	{
		struct state &node = stack[depth-1];

		// base case: hash each file block for its leaf node
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
					// update hash for any dirty children
					if (node.dirty & (1ULL << child.position))
					{
						child.node = child.parent - 1 - node.cnodes *
							(tree.k - child.position - 1);

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
				child.bend = child.bstart + node.cleaves;

				// intersect with the parent's dirty block range
				child.dstart = std::max(child.bstart, node.dstart);
				child.dend = std::min(child.bend, node.dend);

				if (child.dstart < child.dend)
				{
					// traverse down to child node
					child.node = child.parent - 1 - node.cnodes *
						(tree.k - child.position - 1);
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
