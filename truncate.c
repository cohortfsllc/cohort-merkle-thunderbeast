#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <openssl/sha.h>

#include "merkle.h"
#include "update.h"
#include "visitor.h"


static int truncate_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);
static int truncate_node(const struct merkle_state *node,
		uint8_t depth, void *user);
static int truncate_root(const struct merkle_state *node,
		uint8_t depth, void *user);


/* update the hash tree to reflect the given new last block,
 * truncating the hash file and regenerating the root checksum. */
int merkle_truncate(struct merkle_context *context,
		uint64_t new_last_block)
{
	struct merkle_visitor visitor = {
		truncate_leaf,
		truncate_node,
		truncate_root,
		context
	};
	/* traverse only the nodes with 'new_last_block' in their range */
	return merkle_visit(&visitor, context->k,
			new_last_block, new_last_block, new_last_block + 1);
}

/* rehash the root node and truncate the hash file */
static int truncate_root(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	const struct merkle_context *context =
		(const struct merkle_context*)user;
	uint64_t truncate_offset = context->hash_size *
		(node->parent * context->k + 1);
	int status;

	/* generate the root checksum */
	status = update_node(node, depth, user);
	if (status)
		return status;

	/* truncate the hash file directly after the root checksum */
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

/* write zeroes to the node hashes from the given position */
static int zero_hashes(uint64_t node, uint8_t position,
		uint8_t depth, const struct merkle_context *context)
{
	unsigned char digest[SHA_DIGEST_LENGTH] = { 0 };
	uint64_t write_offset;
	int status;

	while (position < context->k) {
		write_offset = context->hash_size *
			(node * context->k + position);

		if (context->verbose)
			printf("%*swrote zeroes to node %lu.%u at offset %lu\n",
					2*depth, "", node, position, write_offset);

		status = write_at(context->fd_out, write_offset,
				digest, context->hash_size);
		if (status)
			return status;

		position++;
	}
	return 0;
}

/* rehash the child node and zero any parent hashes after */
static int truncate_node(const struct merkle_state *node,
		uint8_t depth, void *user)
{
	const struct merkle_context *context =
		(const struct merkle_context*)user;
	int status;

	/* rehash this node into its parent */
	status = update_node(node, depth, user);
	if (status)
		return status;

	/* zero parent hashes for any nodes after this one */
	return zero_hashes(node->parent, node->position + 1, depth, context);
}

/* rehash the new last block and zero any node hashes after */
static int truncate_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user)
{
	const struct merkle_context *context =
		(const struct merkle_context*)user;
	int status;

	/* if truncation was not on a block boundary, rehash this block */
	if (context->partial) {
		status = update_leaf(node, block, position, user);
		if (status)
			return status;
	}

	/* zero node hashes for any blocks after this one */
	return zero_hashes(node->node, position + 1, 0, context);
}
