// vim: ts=2 sw=2 smarttab

#ifndef COHORT_VERIFIER_H
#define COHORT_VERIFIER_H

#include "visitor.h"
#include "block_reader.h"
#include "hash_file.h"


namespace cohort {

	// verifier uses class visitor to traverse the tree and
	// compare the hashes generated with those in the hash file
	class verifier : private visitor {
		private:
			block_reader &reader;
			hash_file &file;
			bool verbose;

		public:
			verifier(const hash_tree &tree, block_reader &reader,
					hash_file &file, bool verbose)
				: visitor(tree), reader(reader), file(file), verbose(verbose)
			{
			}

			// verify the checksums of all blocks in the range [dstart, dend-1],
			// along with all associated ancestors. maxblocks is required to
			// locate the root node
			bool verify(uint64_t dstart, uint64_t dend, uint64_t maxblocks);

		private:
			// overrides from visitor
			bool visit_node(const struct state &node, uint8_t depth);

			bool visit_leaf(uint64_t block, uint8_t position, const struct state &node);
	};

}

#endif // COHORT_VERIFIER_H
