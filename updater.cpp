// vim: ts=2 sw=2 smarttab

#include "updater.h"
#include "hasher.h"

#include <iostream>


using namespace cohort;


bool updater::update(uint64_t dstart, uint64_t dend, uint64_t maxblocks)
{
	uint64_t leaves = maxblocks / tree.k +
		(maxblocks % tree.k ? 1 : 0);
	uint64_t depth = tree.depth(leaves);
	uint64_t highest = tree.root(depth) + 1;
	uint64_t filesize = hasher::DIGEST_SIZE * (highest * tree.k + 1);

	// make sure we can fit the entire tree in the file
	if (!file.resize(filesize))
		return false;

	return visitor::visit(dstart, dend, depth);
}

// read a node and write its hash to the parent
bool updater::visit_node(const struct state &node, uint64_t depth)
{
	uint64_t read_offset = hasher::DIGEST_SIZE * node.node * tree.k;
	uint64_t write_offset = hasher::DIGEST_SIZE *
		(node.parent * tree.k + node.position);

	std::cout << std::string(2*depth, ' ')
		<< "node " << node.node << " at " << read_offset
		<< " hash written to node " << node.parent << "." << (uint32_t)node.position
		<< " at offset " << write_offset << std::endl;

	// read the hashes from the child node
	const unsigned char *inbuf = file.read(read_offset);
	if (inbuf == NULL)
		return false;

	hasher hash;
	hash.process(inbuf, hasher::DIGEST_SIZE * tree.k);

	// write the hash to the parent node
	unsigned char outbuf[hasher::DIGEST_SIZE];
	hash.finish(outbuf);
	return file.write(outbuf, write_offset);
}

// read a block and write its hash to the given node
bool updater::visit_leaf(uint64_t block, uint8_t position,
		const struct state &node)
{
	uint64_t write_offset = hasher::DIGEST_SIZE *
		(node.node * tree.k + position);

	std::cout << "block " << block
		<< " hash written to node " << node.node << "." << (uint32_t)position
		<< " at offset " << write_offset << std::endl;

	hasher hash;
	hash.process(reader.read(block), reader.blocksize());

	// write the hash to the parent node
	unsigned char outbuf[hasher::DIGEST_SIZE];
	hash.finish(outbuf);
	return file.write(outbuf, write_offset);
}
