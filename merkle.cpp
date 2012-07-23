// vim: ts=2 sw=2 smarttab

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstring>

#include "updater.h"
#include "verifier.h"
#include "block_reader.h"
#include "hasher.h"


static int usage(char *name)
{
	std::cout << "Usage:\n"
		<< name << " write <input file> <output file> [blocksize] [tree width]\n"
		<< name << " verify <input file> <hash file>" << std::endl;
	return 1;
}

static int hash_write(char *input, char *output,
		int blocksize, int tree_width)
{
	// open input file for r (fail if not exist)
	int fdin = open(input, O_RDONLY);
	if (fdin == -1)
	{
		std::cerr << "Failed to open input file '" << input
			<< "' with error " << errno << std::endl;
		return 2;
	}

	// open output file for exclusive rw (fail if locked)
	int fdout = open(output, O_RDWR | O_CREAT, 0600);
	if (fdout == -1)
	{
		std::cerr << "Failed to open output file '" << output
			<< "' with error " << errno << std::endl;
		close(fdin);
		return 3;
	}

	// stat the input file for file size
	struct stat stat;
	if (fstat(fdin, &stat) == -1)
	{
		std::cerr << "Failed to get file size of input file '"
			<< input << "' with error " << errno << std::endl;
		close(fdin);
		close(fdout);
		return 4;
	}

	cohort::block_reader blocks(fdin, stat.st_size, blocksize);
	cohort::hash_file file(fdout, cohort::hasher::DIGEST_SIZE, tree_width);
	cohort::hash_tree tree(tree_width);
	cohort::updater updater(tree, blocks, file);

	bool result = updater.update(0, blocks.count()-1, blocks.count());

	close(fdout);
	close(fdin);

	if (!result)
	{
		std::cerr << "hash update failed" << std::endl;
		return 5;
	}

	std::cout << "hash update successful" << std::endl;
	return 0;
}

static int hash_verify(char *input, char *output,
		int blocksize, int tree_width)
{
	// open input file for r (fail if not exist)
	int fdin = open(input, O_RDONLY);
	if (fdin == -1)
	{
		std::cerr << "Failed to open input file '" << input
			<< "' with error " << errno << std::endl;
		return 2;
	}

	// open output file for r (fail if not exist)
	int fdout = open(output, O_RDONLY);
	if (fdout == -1)
	{
		std::cerr << "Failed to open output file '" << output
			<< "' with error " << errno << std::endl;
		close(fdin);
		return 3;
	}

	// stat the input file for file size
	struct stat stat;
	if (fstat(fdin, &stat) == -1)
	{
		std::cerr << "Failed to get file size of input file '"
			<< input << "' with error " << errno << std::endl;
		close(fdin);
		close(fdout);
		return 4;
	}

	cohort::block_reader blocks(fdin, stat.st_size, blocksize);
	cohort::hash_file file(fdout, cohort::hasher::DIGEST_SIZE, tree_width);
	cohort::hash_tree tree(tree_width);
	cohort::verifier verifier(tree, blocks, file);

	bool result = verifier.verify(0, blocks.count()-1, blocks.count());

	close(fdout);
	close(fdin);

	if (!result)
	{
		std::cerr << "hash verification failed" << std::endl;
		return 5;
	}

	std::cout << "hash verification successful" << std::endl;
	return 0;
}

int main(int argc, char *argv[])
{
	int blocksize = 512;
	int tree_width = 4;

	if (argc < 4)
		return usage(argv[0]);

	if (argc > 4)
	{
		int tmp = atoi(argv[4]);
		if (tmp > 0)
			blocksize = tmp;
	}
	if (argc > 5)
	{
		int tmp = atoi(argv[5]);
		if (tmp > 0 && tmp < 256)
			tree_width = tmp;
	}

	if (strcmp(argv[1], "write") == 0)
	{
		return hash_write(argv[2], argv[3], blocksize, tree_width);
	}

	if (strcmp(argv[1], "verify") == 0)
	{
		return hash_verify(argv[2], argv[3], blocksize, tree_width);
	}

	return usage(argv[0]);
}
