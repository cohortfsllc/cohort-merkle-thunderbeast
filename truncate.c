#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <openssl/sha.h>

#include "merkle.h"
#include "update.h"
#include "visitor.h"


static int truncate_node(const struct merkle_state *node,
		uint8_t depth, void *user);
static int truncate_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);


/* update the hash tree to reflect the given new last block,
 * truncating the hash file and regenerating the root checksum. */
int merkle_truncate(struct merkle_context *context,
		uint64_t new_last_block)
{
	struct merkle_visitor visitor = {
		truncate_node,
		truncate_leaf,
		context
	};
	/* traverse only the nodes with 'new_last_block' in their range */
	return merkle_visit(&visitor, context->k,
			new_last_block, new_last_block, new_last_block + 1);
}

/* rehash the child node, and zero any parent hashes after */
static int truncate_node(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	struct merkle_context *context =
		(struct merkle_context*)user;
	unsigned char digest[SHA_DIGEST_LENGTH] = { 0 };
	uint64_t write_offset;
	uint8_t i;
	int status;

	/* rehash this node into its parent */
	status = update_node(node, depth, user);
	if (status)
		return status;

	/* the root node's parent only has one hash */
	if (node->parent == -1ULL)
		return 0;

	/* zero parent hashes for any nodes after this one */
	for (i = node->position + 1; i < context->k; i++) {
		write_offset = SHA_DIGEST_LENGTH *
			(node->parent * context->k + i + 1);
		if (context->verbose)
			printf("%*snode %lu wrote zeroes to "
					"node %lu.%u at offset %lu\n",
					2*depth, "", node->node,
					node->parent, i, write_offset);

		status = write_at(context->fd_out, write_offset,
			       	digest, SHA_DIGEST_LENGTH);
		if (status)
			return status;
	}

	return 0;
}

/* rehash the new last block, and truncate the output file */
static int truncate_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user)
{
	struct merkle_context *context =
		(struct merkle_context*)user;
	uint64_t truncate_offset = SHA_DIGEST_LENGTH *
		(node->node * context->k + position + 2);
	int status;

	/* if truncation was not on a block boundary, update this block */
	if (context->partial) {
		status = update_leaf(node, block, position, user);
		if (status)
			return status;
	}

	/* truncate the hash file immediately after this leaf node */
	if (ftruncate(context->fd_out, truncate_offset) == -1) {
		status = errno;
		fprintf(stderr, "truncate: ftruncate(%lu) failed with %d\n",
				truncate_offset, status);
		return status;
	}
	
	if (context->verbose)
		printf("truncated hash file at %lu\n", truncate_offset);
	return 0;
}
