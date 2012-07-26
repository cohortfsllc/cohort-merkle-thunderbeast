// vim: ts=2 sw=2 smarttab

#include "truncator.h"
#include "hasher.h"

#include <iostream>


using namespace cohort;


bool truncator::truncate(uint64_t new_last_block, bool partial)
{
	uint64_t leaves = new_last_block / tree.k +
		(new_last_block % tree.k ? 1 : 0);
	uint8_t depth = tree.depth(leaves);

	this->partial = partial;

	// traverse only the nodes with 'new_last_block' in their range
	return visitor::visit(new_last_block, new_last_block, depth);
}

// rehash the child node, and zero any parent hashes after
bool truncator::visit_node(const struct state &node, uint8_t depth)
{
	uint64_t read_offset = hasher::DIGEST_SIZE * (node.node * tree.k + 1);
	uint64_t write_offset = node.parent == -1ULL ? 0 :
		hasher::DIGEST_SIZE * (node.parent * tree.k + node.position + 1);

	if (verbose)
		std::cout << std::string(2*depth, ' ')
			<< "node " << node.node << " at " << read_offset
			<< " hash written to node " << node.parent << "." << (uint32_t)node.position
			<< " at offset " << write_offset << std::endl;

	// read the hashes from the child node
	const unsigned char *inbuf = file.read(read_offset,
			hasher::DIGEST_SIZE * tree.k);
	if (inbuf == NULL)
		return false;

	hasher hash;
	hash.process(inbuf, hasher::DIGEST_SIZE * tree.k);

	// write the hash to the parent node
	unsigned char outbuf[hasher::DIGEST_SIZE];
	hash.finish(outbuf);
	if (!file.write(outbuf, write_offset))
		return false;

	// the root node's parent only has one hash
	if (node.parent == -1ULL)
		return true;

	// zero parent hashes for any nodes after this one
	std::fill(outbuf, outbuf + hasher::DIGEST_SIZE, 0);

	for (uint8_t i = node.position + 1; i < tree.k; i++)
	{
		if (verbose)
			std::cout << std::string(2*depth, ' ') << "node " << node.node
				<< " wrote zeroes to node " << node.parent << "." << (uint32_t)i
				<< " at offset " << write_offset << std::endl;

		if (!file.write(outbuf, hasher::DIGEST_SIZE *
					(node.parent * tree.k + i + 1)))
			return false;
	}
	return true;
}

// rehash the new last block, and zero any parent hashes after
bool truncator::visit_leaf(uint64_t block, uint8_t position,
		const struct state &node)
{
	unsigned char outbuf[hasher::DIGEST_SIZE];
	uint64_t write_offset = hasher::DIGEST_SIZE *
		(node.node * tree.k + position + 1);

	// if truncation was not on a block boundary, this needs rehashing
	if (partial)
	{
		if (verbose)
			std::cout << "block " << block
				<< " hash written to node " << node.node << "." << (uint32_t)position
				<< " at offset " << write_offset << std::endl;

		// read the contents of the block
		const unsigned char* inbuf = reader.read(block);
		if (inbuf == NULL)
			return false;

		hasher hash;
		hash.process(inbuf, reader.blocksize());

		// write the hash to the parent node
		hash.finish(outbuf);
		if (!file.write(outbuf, write_offset))
			return false;
	}

	// truncate the output file directly after this block's hash
	if (!file.resize(write_offset + hasher::DIGEST_SIZE))
		return false;

	if (verbose)
		std::cout << "truncated hash tree file to "
			<< write_offset + hasher::DIGEST_SIZE << " bytes." << std::endl;

	// zero parent hashes for any blocks after this one
	std::fill(outbuf, outbuf + hasher::DIGEST_SIZE, 0);

	for (uint8_t i = position + 1; i < tree.k; i++)
	{
		if (verbose)
			std::cout << "block " << block
				<< " wrote zeroes to node " << node.node << "." << (uint32_t)i
				<< " at offset " << write_offset << std::endl;

		if (!file.write(outbuf, hasher::DIGEST_SIZE *
					(node.node * tree.k + i + 1)))
			return false;
	}

	return true;
}
