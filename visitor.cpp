// vim: ts=2 sw=2 smarttab

#include <iostream>

#include "visitor.h"


using namespace cohort;


// visit all nodes associated with blocks in range [dstart, dend]
bool visitor::visit(uint64_t dstart, uint64_t dend, uint64_t depth)
{
	// initialize the root node
	struct state root;
	root.node = tree.root(depth);
	root.parent = root.node + 1;
	root.bstart = 0;
	root.bend = tree.leaves(depth) - 1;
	root.dstart = dstart;
	root.dend = dend + 1;
	root.dirty = 0;
	root.progress = 0;
	root.position = 0;

	// push it to the state stack
	state_stack stack;
	stack.push(root);

	while (!stack.empty())
	{
		struct state &node = stack.top();

		// base case: hash file blocks into their leaf node
		if (depth == 1)
		{
			if (node.bend - node.bstart != tree.k)
				std::cerr << std::string(2*depth, ' ')
					<< "error: leaf node " << node.node
					<< " with " << node.bend - node.bstart << " blocks!" << std::endl;

			for (; node.dstart < node.dend; node.dstart++)
			{
				if (!visit_leaf(node.dstart, node.dstart - node.bstart, node))
					return false;
			}

			depth++;
			stack.pop();
		}
		else
		{
			// distance between child nodes
			const uint64_t dnodes = tree.size(depth - 1);

			struct state child;
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
						child.node = child.parent - 1 - dnodes *
							(tree.k - child.position - 1);

						if (!visit_node(child, depth - 1))
							return false;
					}
				}

				// traverse back up to parent node
				depth++;
				stack.pop();
				continue;
			}

			// total number of blocks under each child
			const uint64_t dblocks = math::powi(tree.k, depth - 1);

			while (node.progress < tree.k) 
			{
				child.position = node.progress++;

				// calculate which blocks are under this node
				child.bstart = node.bstart + child.position * dblocks;
				child.bend = child.bstart + dblocks;

				// intersect with the parent's dirty block range
				child.dstart = std::max(child.bstart, node.dstart);
				child.dend = std::min(child.bend, node.dend);

				if (child.dstart < child.dend)
				{
					// traverse down to child node
					child.node = child.parent - 1 - dnodes *
						(tree.k - child.position - 1);
					depth--;
					stack.push(child);
					node.dirty |= (1ULL << child.position);
					break;
				}
			}
		}
	}

	// write final hash of root node
	return visit_node(root, depth);
}
