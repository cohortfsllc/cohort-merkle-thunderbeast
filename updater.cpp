// vim: ts=2 sw=2 smarttab

#include "updater.h"
#include "hasher.h"

#include <iostream>


using namespace cohort;


// read a node and write its hash to the parent
bool updater::hash_node(const struct update &node, uint64_t depth)
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
bool updater::hash_block(uint64_t block, uint8_t position, const struct update &node)
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

// update the hash tree to reflect changes to the blocks
// in range [dstart, dend].  maxblocks is also needed to
// calculate the position of the tree's root node
bool updater::update(uint64_t dstart, uint64_t dend, uint64_t maxblocks)
{
	const uint64_t leaves = maxblocks / tree.k +
		(maxblocks % tree.k ? 1 : 0);
	uint64_t depth = tree.depth(leaves);

	// initialize the root node
	struct update root;
	root.node = tree.root(depth);
	root.parent = root.node + 1;
	root.bstart = 0;
	root.bend = leaves - 1;
	root.dstart = dstart;
	root.dend = dend + 1;
	root.dirty = 0;
	root.progress = 0;
	root.position = 0;

	// push it to the state stack
	update_stack stack;
	stack.push(root);

	// make sure the file can hold the entire hash tree
	file.resize(hasher::DIGEST_SIZE * (root.parent * tree.k + 1));

	while (!stack.empty())
	{
		struct update &node = stack.top();

		// base case: hash file blocks into their leaf node
		if (depth == 1)
		{
			if (node.bend - node.bstart != tree.k)
				std::cerr << std::string(2*depth, ' ')
					<< "error: leaf node " << node.node
					<< " with " << node.bend - node.bstart << " blocks!" << std::endl;

			for (; node.dstart < node.dend; node.dstart++)
			{
				if (!hash_block(node.dstart, node.dstart - node.bstart, node))
					return false;
			}

			depth++;
			stack.pop();
		}
		else
		{
			// distance between child nodes
			const uint64_t dnodes = tree.size(depth - 1);

			struct update child;
			child.parent = node.node;
			child.progress = 0;
			child.dirty = 0;

			if (node.progress == tree.k)
			{
				// all child nodes have been processed
				for (child.position = 0; child.position < tree.k; child.position++)
				{
					// update hash for any dirty children
					if (node.dirty & (1ULL << child.position))
					{
						child.node = child.parent - 1 - dnodes *
							(tree.k - child.position - 1);

						if (!hash_node(child, depth - 1))
							return false;
					}
				}

				// traverse back up to parent node
				depth++;
				stack.pop();
				continue;
			}

			// total number of blocks under each child
			const uint64_t dblocks = math::powi(tree.k, depth - 1);

			while (node.progress < tree.k) 
			{
				child.position = node.progress++;

				// calculate which blocks are under this node
				child.bstart = node.bstart + child.position * dblocks;
				child.bend = child.bstart + dblocks;

				// intersect with the parent's dirty block range
				child.dstart = std::max(child.bstart, node.dstart);
				child.dend = std::min(child.bend, node.dend);

				if (child.dstart < child.dend)
				{
					// traverse down to child node
					child.node = child.parent - 1 - dnodes *
						(tree.k - child.position - 1);
					depth--;
					stack.push(child);
					node.dirty |= (1ULL << child.position);
					break;
				}
			}
		}
	}

	// write final hash of root node
	return hash_node(root, depth);
}
