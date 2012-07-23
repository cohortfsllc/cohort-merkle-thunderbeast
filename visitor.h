// vim: ts=2 sw=2 smarttab

#ifndef COHORT_VISITOR_H
#define COHORT_VISITOR_H

#include <stack>

#include "hash_tree.h"


namespace cohort {

	// visitor: perform a depth-first traversal of the tree, ignoring
	//  nodes that don't contain any dirty blocks.
	class visitor {
		protected:
			const hash_tree &tree;

		public:
			visitor(const hash_tree &tree)
				: tree(tree)
			{
			}

			// visit all nodes associated with blocks in range [dstart, dend]
			bool visit(uint64_t dstart, uint64_t dend, uint64_t depth);

		protected:
			// state kept on a stack to simulate a recursive algorithm
			struct state
			{
				uint64_t node, parent; // index of the node and its parent
				uint64_t bstart, bend; // bounds of all blocks under this node
				uint64_t dstart, dend; // bounds of dirty blocks under this node
				uint64_t dirty; // bitmask of children traversed. XXX: only supports k <= 64
				uint8_t progress; // number of children processed
				uint8_t position; // child's position under parent node [0, k-1]
			};
			typedef std::stack<struct state> state_stack;

			// virtual functions
			virtual bool visit_node(const struct state &node, uint64_t depth)
			{
			}
			virtual bool visit_leaf(uint64_t block, uint8_t position,
				 const struct state &node)
			{
			}
	};

}

#endif // COHORT_VISITOR_H