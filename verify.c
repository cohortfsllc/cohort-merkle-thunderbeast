#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>

#include "merkle.h"
#include "update.h"
#include "visitor.h"


static int verify_node(const struct merkle_state *node,
		uint8_t depth, void *user);
static int verify_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);


/* verify the checksums of all blocks in the given range,
 * along with all associated ancestors */
int merkle_verify(struct merkle_context *context,
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

/* read a node and compare its hash with the parent */
static int verify_node(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	struct merkle_context *context =
		(struct merkle_context*)user;
	uint64_t read_offset = context->hash_size * node->node * context->k;
	uint64_t write_offset = context->hash_size *
		(node->parent * context->k + node->position);
	unsigned char digest[SHA_DIGEST_LENGTH] = { 0 };
	SHA_CTX hash;
	int status;

	/* read the hashes from the child node */
	status = read_at(context->fd_out, read_offset,
			context->node_buffer, context->node_size);
	if (status)
		return status;

	/* check for zeroes outside of the valid range. the parent
	 * hash comparison won't catch this, because it generates
	 * the hash out of the file itself */
	uint64_t blocks = node->bend - node->bstart;
	uint8_t i = blocks / node->cleaves +
		(blocks % node->cleaves ? 1 : 0);
	for (; i < context->k; i++) {
		const unsigned char *buffer = context->node_buffer +
			i * context->hash_size;

		if (memcmp(digest, buffer, context->hash_size)) {
			fprintf(stderr, "%*snode %lu.%u at offset %lu expected "
					"zeroes\n", 2*depth, "", node->node, i,
					read_offset + i * context->hash_size);
			return -1;
		}

		if (context->verbose)
			printf("%*snode %lu.%u at %lu verified zeroes\n",
					2*depth, "", node->node, i,
					read_offset + i * context->hash_size);
	}

	/* compute the node hash */
	SHA1_Init(&hash);
	SHA1_Update(&hash, context->node_buffer, context->node_size);
	SHA1_Final(digest, &hash);

	/* read the expected node hash from its parent */
	status = read_at(context->fd_out, write_offset,
			context->node_buffer, context->hash_size);
	if (status)
		return status;

	/* compare the node hash with its expected parent hash */
	if (memcmp(digest, context->node_buffer, context->hash_size)) {
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
	struct merkle_context *context =
		(struct merkle_context*)user;
	uint64_t read_offset = block * context->block_size;
	uint64_t write_offset = context->hash_size *
		(node->node * context->k + position);
	unsigned char digest[SHA_DIGEST_LENGTH];
	SHA_CTX hash;
	int status;

	/* read the contents of the block */
	status = read_at(context->fd_in, read_offset,
			context->block_buffer, context->block_size);
	if (status)
		return status;

	/* compute the block hash */
	SHA1_Init(&hash);
	SHA1_Update(&hash, context->block_buffer, context->block_size);
	SHA1_Final(digest, &hash);

	/* read the expected block hash from the leaf node */
	status = read_at(context->fd_out, write_offset,
			context->node_buffer, context->hash_size);
	if (status)
		return status;

	/* compare the block hash with its expected leaf hash */
	if (memcmp(digest, context->node_buffer, context->hash_size)) {
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
