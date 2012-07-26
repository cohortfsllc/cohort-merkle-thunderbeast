// vim: ts=2 sw=2 smarttab

#ifndef COHORT_TRUNCATOR_H
#define COHORT_TRUNCATOR_H

#include "visitor.h"
#include "block_reader.h"
#include "hash_file.h"


namespace cohort {

	class truncator : private visitor {
		private:
			block_reader &reader;
			hash_file &file;
			bool verbose;
			bool partial; // true if file was not truncated on block boundary

		public:
			truncator(const hash_tree &tree, block_reader &reader,
					hash_file &file, bool verbose)
				: visitor(tree), reader(reader), file(file), verbose(verbose)
			{
			}

			bool truncate(uint64_t new_last_block, bool partial);

		private:
			// overrides from visitor
			bool visit_node(const struct state &node, uint8_t depth);

			bool visit_leaf(uint64_t block, uint8_t position, const struct state &node);
	};

}

#endif // COHORT_TRUNCATOR_H
