// vim: ts=2 sw=2 smarttab

#include "verifier.h"
#include "hasher.h"

#include <iostream>


using namespace cohort;


bool verifier::verify(uint64_t dstart, uint64_t dend, uint64_t maxblocks)
{
	uint64_t leaves = maxblocks / tree.k +
		(maxblocks % tree.k ? 1 : 0);
	uint64_t depth = tree.depth(leaves);

	return visitor::visit(dstart, dend, depth);
}

// read a node and compare its hash with the parent
bool verifier::visit_node(const struct state &node, uint64_t depth)
{
	uint64_t read_offset = hasher::DIGEST_SIZE * node.node * tree.k;
	uint64_t write_offset = hasher::DIGEST_SIZE *
		(node.parent * tree.k + node.position);

	// read the hashes from the child node
	const unsigned char *inbuf = file.read(read_offset,
			hasher::DIGEST_SIZE * tree.k);
	if (inbuf == NULL)
		return false;

	hasher hash;
	hash.process(inbuf, hasher::DIGEST_SIZE * tree.k);

	// calculate the hash
	unsigned char outbuf[hasher::DIGEST_SIZE];
	hash.finish(outbuf);

	// compare with the parent node
	const unsigned char *expected = file.read(write_offset,
			hasher::DIGEST_SIZE);
	if (expected == NULL)
		return false;

	if (!std::equal(outbuf, outbuf + hasher::DIGEST_SIZE, expected))
	{
		std::cerr << "node " << node.node << " at " << read_offset
			<< " hash does not match node " << node.parent << "." << (uint32_t)node.position
			<< " at offset " << write_offset << std::endl;
		return false;
	}

	std::cout << std::string(2*depth, ' ')
		<< "node " << node.node << " at " << read_offset
		<< " hash matches node " << node.parent << "." << (uint32_t)node.position
		<< " at offset " << write_offset << std::endl;
	return true;
}

// read a block and compare its hash with the given node
bool verifier::visit_leaf(uint64_t block, uint8_t position,
		const struct state &node)
{
	uint64_t write_offset = hasher::DIGEST_SIZE *
		(node.node * tree.k + position);

	// read the contents of the block
	const unsigned char* inbuf = reader.read(block);
	if (inbuf == NULL)
		return false;

	hasher hash;
	hash.process(inbuf, reader.blocksize());

	// calculate the hash
	unsigned char outbuf[hasher::DIGEST_SIZE];
	hash.finish(outbuf);

	// compare with the parent node
	const unsigned char *expected = file.read(write_offset,
			hasher::DIGEST_SIZE);
	if (expected == NULL)
		return false;

	if (!std::equal(outbuf, outbuf + hasher::DIGEST_SIZE, expected))
	{
		std::cerr << "block " << block << " hash does not match node "
			<< node.node << "." << (uint32_t)position
			<< " at offset " << write_offset << std::endl;
		return false;
	}

	std::cout << "block " << block << " hash matches node "
		<< node.node << "." << (uint32_t)position
		<< " at offset " << write_offset << std::endl;
	return true;
}
