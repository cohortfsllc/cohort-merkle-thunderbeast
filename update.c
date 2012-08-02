#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <openssl/sha.h>

#include "merkle.h"
#include "update.h"
#include "visitor.h"


/* update the hashes for dirty blocks in the given range,
 * along with all associated ancestors */
int merkle_update(struct merkle_context *context,
		uint64_t from_block, uint64_t to_block,
		uint64_t total_blocks)
{
	struct merkle_visitor visitor = {
		update_node,
		update_leaf,
		context
	};
	return merkle_visit(&visitor, context->k,
			from_block, to_block, total_blocks);
}

/* read a node and write its hash to the parent */
int update_node(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	struct merkle_context *context =
		(struct merkle_context*)user;
	uint64_t read_offset = context->hash_size * node->node * context->k;
	uint64_t write_offset = context->hash_size *
		(node->parent * context->k + node->position);
	unsigned char digest[SHA_DIGEST_LENGTH];
	SHA_CTX hash;
	int status;

	if (context->verbose)
		printf("%*snode %lu at %lu hash written to "
				"node %lu.%u at offset %lu\n",
				2*depth, "", node->node, read_offset,
				node->parent, node->position, write_offset);

	/* read the hashes from the child node */
	status = read_at(context->fd_out, read_offset,
			context->node_buffer, context->node_size);
	if (status)
		return status;

	SHA1_Init(&hash);
	SHA1_Update(&hash, context->node_buffer, context->node_size);
	SHA1_Final(digest, &hash);

	/* write the hash to the parent node */
	return write_at(context->fd_out, write_offset,
			digest, context->hash_size);
}

/* read a block and write its hash to the given leaf node */
int update_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user)
{
	struct merkle_context *context =
		(struct merkle_context*)user;
	uint64_t read_offset = block * context->block_size;
	uint64_t write_offset = context->hash_size *
		(node->node * context->k + position);
	unsigned char digest[SHA_DIGEST_LENGTH];
	SHA_CTX hash;
	int status;

	if (context->verbose)
		printf("block %lu hash written to node %lu.%u "
				"at offset %lu\n", block, node->node,
				position, write_offset);

	/* read the contents of the block */
	status = read_at(context->fd_in, read_offset,
			context->block_buffer, context->block_size);
	if (status)
		return status;

	SHA1_Init(&hash);
	SHA1_Update(&hash, context->block_buffer, context->block_size);
	SHA1_Final(digest, &hash);

	/* write the hash to the leaf node */
	return write_at(context->fd_out, write_offset,
			digest, context->hash_size);
}


/* common functions for file i/o */
int read_at(int fd, off_t offset, unsigned char *buffer, size_t length)
{
	ssize_t bytes;
	if (lseek(fd, offset, SEEK_SET) == -1) {
		fprintf(stderr, "lseek(%lu) failed with error %d\n",
				offset, errno);
		return errno;
	}
	bytes = read(fd, buffer, length);
	if (bytes == -1) {
		fprintf(stderr, "read() failed with error %d\n", errno);
		return errno;
	}
	/* zero-fill the remaining bytes */
	while (bytes < (ssize_t)length)
		buffer[bytes++] = 0;
	return 0;
}

int write_at(int fd, off_t offset, unsigned char *buffer, size_t length)
{
	ssize_t bytes;
	if (lseek(fd, offset, SEEK_SET) == -1) {
		fprintf(stderr, "lseek(%lu) failed with error %d\n",
				offset, errno);
		return errno;
	}
	while (length) {
		bytes = write(fd, buffer, length);
		if (bytes == -1) {
			fprintf(stderr, "write() failed with error %d\n",
					errno);
			return errno;
		}
		length -= bytes;
		buffer += bytes;
	}
	return 0;
}
