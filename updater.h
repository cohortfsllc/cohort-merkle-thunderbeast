// vim: ts=2 sw=2 smarttab

#ifndef COHORT_UPDATER_H
#define COHORT_UPDATER_H

#include <stack>

#include "hash_tree.h"
#include "block_reader.h"
#include "hash_file.h"


namespace cohort {

	// updater: perform a depth-first traversal of the tree, ignoring
	//  nodes that don't contain any dirty blocks.
	class updater {
		private:
			const hash_tree &tree;
			block_reader &reader;
			hash_file &file;

		public:
			updater(const hash_tree &tree, block_reader &reader, hash_file &file)
				: tree(tree), reader(reader), file(file)
			{
			}

		private:
			// state kept on a stack to simulate a recursive algorithm
			struct update
			{
				uint64_t node, parent; // index of the node and its parent
				uint64_t bstart, bend; // bounds of all blocks under this node
				uint64_t dstart, dend; // bounds of dirty blocks under this node
				uint64_t dirty; // bitmask of children traversed. XXX: only supports k <= 64
				uint8_t progress; // number of children processed
				uint8_t position; // child's position under parent node [0, k-1]
			};
			typedef std::stack<struct update> update_stack;

			bool hash_node(const struct update &node, uint64_t depth);

			bool hash_block(uint64_t block, uint8_t position, const struct update &node);

		public:
			// update the hash tree to reflect changes to the blocks
			// in range [dstart, dend].  maxblocks is also needed to
			// calculate the position of the tree's root node
			bool update(uint64_t dstart, uint64_t dend, uint64_t maxblocks);
	};

}

#endif // COHORT_UPDATER_H
