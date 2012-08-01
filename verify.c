#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>

#include "operations.h"
#include "visitor.h"


static int verify_node(const struct merkle_state *node,
		uint8_t depth, void *user);
static int verify_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);


/* verify the checksums of all blocks in the given range,
 * along with all associated ancestors */
int merkle_verify(struct merkle_op_context *context,
		uint64_t from_block, uint64_t to_block, uint64_t maxblocks)
{
	struct merkle_visitor visitor = {
		verify_node,
		verify_leaf,
		context
	};
	return merkle_visit(&visitor, context->k,
			from_block, to_block, maxblocks);
}


static int read_at(int fd, off_t offset, unsigned char *buffer, size_t length)
{
	ssize_t bytes;

	if (lseek(fd, offset, SEEK_SET) == -1) {
		fprintf(stderr, "update: lseek() failed with error %d\n", errno);
		return errno;
	}
	bytes = read(fd, buffer, length);
	if (bytes == -1) {
		fprintf(stderr, "update: read() failed with error %d\n", errno);
		return errno;
	}
	/* zero-fill the remaining bytes */
	while (bytes < (ssize_t)length)
		buffer[bytes++] = 0;
	return 0;
}


/* read a node and compare its hash with the parent */
static int verify_node(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	struct merkle_op_context *context =
		(struct merkle_op_context*)user;
	uint64_t read_offset = SHA_DIGEST_LENGTH *
		(node->node * context->k + 1);
	uint64_t write_offset = node->parent == -1ULL ? 0 : SHA_DIGEST_LENGTH *
		(node->parent * context->k + node->position + 1);
	unsigned char digest[SHA_DIGEST_LENGTH] = { 0 };
	SHA_CTX hash;
	uint8_t i;
	int status;

	/* read the hashes from the child node */
	status = read_at(context->fd_out, read_offset,
			context->node_buffer, context->node_size);
	if (status)
		return status;

	if (node->parent != -1ULL) {
		/* check for zeroes outside of the valid range. the parent
		 * hash comparison won't catch this, because it generates
		 * the hash out of the file itself */
		i = (node->bend - node->bstart) / node->cleaves +
			((node->bend - node->bstart) % node->cleaves ? 1 : 0);
		for (; i < context->k; i++) {
			const unsigned char *buffer = context->node_buffer +
				i * SHA_DIGEST_LENGTH;

			if (memcmp(digest, buffer, SHA_DIGEST_LENGTH)) {
				fprintf(stderr, "%*snode %lu at offset %lu expected "
						"zeroes at node %lu.%u\n",
						2*depth, "", node->node, read_offset,
						node->parent, i);
				return -1;
			}

			if (context->verbose)
				printf("%*snode %lu at %lu verified zeroes "
						"at node %lu.%u\n", 2*depth, "",
						node->node, read_offset, node->parent, i);
		}
	}

	/* calculate the hash */
	SHA1_Init(&hash);
	SHA1_Update(&hash, context->node_buffer, context->node_size);
	SHA1_Final(digest, &hash);

	/* compare with the parent node */
	status = read_at(context->fd_out, write_offset,
			context->node_buffer, SHA_DIGEST_LENGTH);
	if (status)
		return status;

	if (memcmp(digest, context->node_buffer, SHA_DIGEST_LENGTH)) {
		fprintf(stderr, "%*snode %lu at %lu hash does not match "
				"node %lu.%u at offset %lu\n",
				2*depth, "", node->node, read_offset,
				node->parent, node->position, write_offset);
		return -1;
	}

	if (context->verbose)
		printf("%*snode %lu at %lu hash matches "
				"node %lu.%u at offset %lu\n",
				2*depth, "", node->node, read_offset,
				node->parent, node->position, write_offset);
	return 0;
}

/* read a block and compare its hash with the given node */
static int verify_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user)
{
	struct merkle_op_context *context =
		(struct merkle_op_context*)user;
	uint64_t read_offset = block * context->block_size;
	uint64_t write_offset = SHA_DIGEST_LENGTH *
		(node->node * context->k + position + 1);
	unsigned char digest[SHA_DIGEST_LENGTH];
	SHA_CTX hash;
	int status;

	/* read the contents of the block */
	status = read_at(context->fd_in, read_offset,
			context->block_buffer, context->block_size);
	if (status)
		return status;

	SHA1_Init(&hash);
	SHA1_Update(&hash, context->block_buffer, context->block_size);
	SHA1_Final(digest, &hash);

	/* compare with the parent node */
	status = read_at(context->fd_out, write_offset,
			context->node_buffer, SHA_DIGEST_LENGTH);
	if (status)
		return status;

	if (memcmp(digest, context->node_buffer, SHA_DIGEST_LENGTH)) {
		fprintf(stderr, "block %lu hash does not match "
				"node %lu.%u at offset %lu\n",
				block, node->node, position, write_offset);
		return -1;
	}

	if (context->verbose)
		printf("block %lu hash matches node %lu.%u at offset %lu\n",
				block, node->node, position, write_offset);
	return 0;
}
