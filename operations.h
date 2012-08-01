#ifndef COHORT_MERKLE_OPERATIONS_H
#define COHORT_MERKLE_OPERATIONS_H

#include <stddef.h>
#include <stdint.h>


/* context passed as user data to the merkle tree visitor */
struct merkle_op_context {
	/* buffer and size for reading blocks from the input file */
	unsigned char *block_buffer;
	size_t block_size;
	/* buffer and size for reading nodes from the output file */
	unsigned char *node_buffer;
	size_t node_size;
	int fd_in; /* input file */
	int fd_out; /* output file */
	uint8_t k;
	uint8_t verbose;
	uint8_t partial;
};

/* update the hashes for dirty blocks in the given range,
 * along with all associated ancestors.  total_blocks is
 * required to locate the root node */
int merkle_update(struct merkle_op_context *context,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks);

/* update the hash tree to reflect the given new last block,
 * truncating the hash file and regenerating the root checksum */
int merkle_truncate(struct merkle_op_context *context,
		uint64_t new_last_offset);

/* verify the checksums of all blocks in the given range,
 * along with all associated ancestors.  total_blocks is
 * required to locate the root node */
int merkle_verify(struct merkle_op_context *context,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks);


/* from visitor.h */
struct merkle_state;

/* visitor callbacks for update, used by truncate */
int merkle_update_node(const struct merkle_state *node,
		uint8_t depth, void *user);
int merkle_update_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);

#endif /* COHORT_MERKLE_OPERATIONS_H */
