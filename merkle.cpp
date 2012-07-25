// vim: ts=2 sw=2 smarttab

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstring>

#include "updater.h"
#include "verifier.h"
#include "block_reader.h"
#include "hasher.h"


namespace {

	int usage(char *name)
	{
		std::cout << "Usage:\n"
			<< name << " write [options] <input file> <output file>\n"
			<< name << " verify [options] <input file> <hash file>\n\n"
			"Options:\n"
			" -b #    Size of each file block in bytes. default: 512\n\n"
			" -k #    Number of children for each hash tree node. Must be\n"
			"         a power of 2, between 2 and 128. default: 4\n\n"
			" -r # #  Range of file blocks on which to operate, using\n"
			"         zero-based indices. default: all blocks\n\n"
			" -v      Verbose output.\n" << std::endl;
		return 1;
	}

	// command line options
	class cmd_options {
		public:
			const char *operation;
			const char *source;
			const char *hash;
			uint8_t tree_width;
			uint32_t block_size;
			uint32_t range_from;
			uint32_t range_to;
			bool verbose;

			cmd_options()
				: operation(NULL),
					source(NULL),
					hash(NULL),
					tree_width(4),
					block_size(512),
					range_from(0),
					range_to(0xFFFFFFFF),
					verbose(false)
			{
			}

			bool parse(int argc, char *argv[]);
	};

	// helper class to clean up a file descriptor
	class auto_fd {
		private:
			const int fd;
		public:
			// must construct with a fd
			explicit auto_fd(int fd) : fd(fd) {}

			// close on destruction
			~auto_fd() { if (fd != -1) ::close(fd); }

			// cast directly to int
			operator int() const { return fd; }

		private: // disable default construction and copy/assignment
			auto_fd();
			auto_fd(const auto_fd&);
			const auto_fd& operator=(const auto_fd&);
	};


	int hash_write(const cmd_options &options)
	{
		// open input file for r (fail if not exist)
		auto_fd fdin(::open(options.source, O_RDONLY));
		if (fdin == -1)
		{
			std::cerr << "Failed to open input file '" << options.source
				<< "' with error " << errno << std::endl;
			return 2;
		}

		// open output file for exclusive rw (fail if locked)
		auto_fd fdout(::open(options.hash, O_RDWR | O_CREAT, 0600));
		if (fdout == -1)
		{
			std::cerr << "Failed to open output file '" << options.hash
				<< "' with error " << errno << std::endl;
			return 3;
		}

		// stat the input file for file size
		struct stat stat;
		if (::fstat(fdin, &stat) == -1)
		{
			std::cerr << "Failed to get file size of input file '"
				<< options.source << "' with error " << errno << std::endl;
			return 4;
		}

		cohort::block_reader blocks(fdin,
				stat.st_size, options.block_size);
		cohort::hash_file file(fdout,
				cohort::hasher::DIGEST_SIZE, options.tree_width);
		cohort::hash_tree tree(options.tree_width);
		cohort::updater updater(tree, blocks, file, options.verbose);

		if (!updater.update(0, blocks.count()-1, blocks.count()))
		{
			std::cerr << "hash update failed" << std::endl;
			return 5;
		}

		std::cout << "hash update successful" << std::endl;
		return 0;
	}

	int hash_verify(const cmd_options &options)
	{
		// open input file for r (fail if not exist)
		auto_fd fdin(::open(options.source, O_RDONLY));
		if (fdin == -1)
		{
			std::cerr << "Failed to open input file '" << options.source
				<< "' with error " << errno << std::endl;
			return 2;
		}

		// open output file for r (fail if not exist)
		auto_fd fdout(::open(options.hash, O_RDONLY));
		if (fdout == -1)
		{
			std::cerr << "Failed to open output file '" << options.hash
				<< "' with error " << errno << std::endl;
			return 3;
		}

		// stat the input file for file size
		struct stat stat;
		if (::fstat(fdin, &stat) == -1)
		{
			std::cerr << "Failed to get file size of input file '"
				<< options.source << "' with error " << errno << std::endl;
			return 4;
		}

		cohort::block_reader blocks(fdin,
				stat.st_size, options.block_size);
		cohort::hash_file file(fdout,
				cohort::hasher::DIGEST_SIZE, options.tree_width);
		cohort::hash_tree tree(options.tree_width);
		cohort::verifier verifier(tree, blocks, file, options.verbose);

		if (!verifier.verify(0, blocks.count()-1, blocks.count()))
		{
			std::cerr << "hash verification failed" << std::endl;
			return 5;
		}

		std::cout << "hash verification successful" << std::endl;
		return 0;
	}


	// parse command line options
	bool cmd_options::parse(int argc, char *argv[])
	{
		argc--;
		argv++;
		operation = argv[0];
		if (strcmp(operation, "write") &&
				strcmp(operation, "verify"))
		{
			std::cerr << "Unknown operation '"
				<< operation << "'.\n" << std::endl;
			return false;
		}

		argc--;
		argv++;
		while (argc && argv[0][0] == '-')
		{
			if (strcmp(argv[0], "-b") == 0)
			{
				if (argc < 2)
				{
					std::cerr << "Option -b missing argument.\n" << std::endl;
					return false;
				}
				block_size = atoi(argv[1]);
				if (block_size == 0)
				{
					std::cerr << "Invalid block size '"
						<< argv[1] << "'.\n" << std::endl;
					return false;
				}
				argc -= 2;
				argv += 2;
			}
			else if (strcmp(argv[0], "-k") == 0)
			{
				if (argc < 2)
				{
					std::cerr << "Option -k missing argument.\n" << std::endl;
					return false;
				}
				tree_width = atoi(argv[1]);
				if (tree_width != 2 && tree_width != 4 &&
						tree_width != 8 && tree_width != 16 &&
						tree_width != 32 && tree_width != 64 &&
						tree_width != 128)
				{
					std::cerr << "Invalid value for k='" << argv[1]
						<< "': not a power of 2 between 2 and 128.\n" << std::endl;
					return false;
				}
				argc -= 2;
				argv += 2;
			}
			else if (strcmp(argv[0], "-r") == 0)
			{
				if (argc < 3)
				{
					std::cerr << "Option -r missing arguments.\n" << std::endl;
					return false;
				}
				range_from = atoi(argv[1]);
				range_to = atoi(argv[2]);
				if (range_from >= range_to)
				{
					std::cerr << "Invalid block range: " << argv[1]
						<< " -> " << argv[2] << ".\n" << std::endl;
					return false;
				}
				argc -= 3;
				argv += 3;
			}
			else if (strcmp(argv[0], "-v") == 0)
			{
				verbose = true;
				argc--;
				argv++;
			}
			else
			{
				std::cerr << "Unrecognized option " << argv[0] << "." << std::endl;
				argc--;
				argv++;
			}
		}

		if (argc < 1)
		{
			std::cerr << "Missing argument for input file.\n" << std::endl;
			return false;
		}
		source = argv[0];

		if (argc < 2)
		{
			std::cerr << "Missing argument for hash file.\n" << std::endl;
			return false;
		}
		hash = argv[1];
		return true;
	}
}

int main(int argc, char *argv[])
{
	cmd_options options;
	if (options.parse(argc, argv))
	{
		if (strcmp(options.operation, "write") == 0)
			return hash_write(options);

		if (strcmp(options.operation, "verify") == 0)
			return hash_verify(options);
	}
	return usage(argv[0]);
}
