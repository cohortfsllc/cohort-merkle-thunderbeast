#ifndef COHORT_MERKLE_H
#define COHORT_MERKLE_H

#include <stddef.h>
#include <stdint.h>


/* context passed as argument to merkle tree operations */
struct merkle_context {
	/* buffer and size for reading blocks from the input file */
	unsigned char *block_buffer;
	size_t block_size;
	/* buffer and size for reading nodes from the output file */
	unsigned char *node_buffer;
	size_t node_size; /* node size = k * hash size */
	int fd_in; /* input file */
	int fd_out; /* output file */
	uint8_t k; /* number of children per hash tree node */
	uint8_t verbose; /* verbose output */
	uint8_t partial; /* truncate that was not on a block boundary */
};

/* update the hashes for dirty blocks in the given range,
 * along with all associated ancestors. total_blocks is
 * required to locate the root checksum.
 * context.fd_out must be opened for write access */
int merkle_update(struct merkle_context *context,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks);

/* update the hash tree to reflect the given new last block,
 * truncating the hash file and regenerating the root checksum.
 * context.fd_in and fd_out must be opened for write access */
int merkle_truncate(struct merkle_context *context,
		uint64_t new_last_block);

/* verify the checksums of all blocks in the given range,
 * along with all associated ancestors. total_blocks is
 * required to locate the root checksum */
int merkle_verify(struct merkle_context *context,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks);

#endif /* COHORT_MERKLE_H */
