// vim: ts=2 sw=2 smarttab

#ifndef COHORT_TRUNCATOR_H
#define COHORT_TRUNCATOR_H

#include "visitor.h"
#include "block_reader.h"
#include "hash_file.h"


namespace cohort {

	// truncator uses class visitor to traverse the tree,
	// clear the hash values for any removed nodes, and
	// recalculate the hashes for affected intermediate nodes
	class truncator : private visitor {
		private:
			block_reader &reader;
			hash_file &file;
			bool verbose;

		public:
			truncator(const hash_tree &tree, block_reader &reader,
					hash_file &file, bool verbose)
				: visitor(tree), reader(reader), file(file), verbose(verbose)
			{
			}

			// update the hash tree to reflect the given new last block,
			// truncating the hash file and regenerating the root checksum
			bool truncate(uint64_t new_last_block);

		private:
			// overrides from visitor
			bool visit_node(const struct state &node, uint8_t depth);

			bool visit_leaf(uint64_t block, uint8_t position, const struct state &node);
	};

}

#endif // COHORT_TRUNCATOR_H
