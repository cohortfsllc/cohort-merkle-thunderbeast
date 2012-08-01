#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

#include "merkle.h"


int usage(char *name)
{
	printf("Usage:\n"
			"%s <operation> [options] <input file> <output file>\n\n"
			"Operations:\n"
			"  write    Read blocks from the input file and write an updated\n"
			"           hash tree to the output file.\n\n"
			"  truncate Truncate the input file at the upper bound specified\n"
			"           with -r, and write an updated hash tree to the output file.\n\n"
			"  verify   Read blocks from the input file and compare the hashes\n"
			"           with those from the output file.\n\n"
			"Options:\n"
			"  -b #     Size of each file block in bytes. default: 512\n\n"
			"  -k #     Number of children for each hash tree node. Must be\n"
			"           a power of 2, between 2 and 128. default: 4\n\n"
			"  -r # #   Range of file blocks on which to operate, using\n"
			"           zero-based indices. default: all blocks\n\n"
			"  -v       Verbose output.\n", name);
	return 1;
}

/* command line options */
struct cmd_options {
	const char *operation;
	const char *source;
	const char *hash;
	uint8_t tree_width;
	uint32_t block_size;
	uint32_t range_from;
	uint32_t range_to;
	uint8_t verbose;
};


/* update the block range options once we know the total block count,
 * returning an error if our arguments are out of range. */
static int update_range(struct cmd_options *options, uint64_t blocks)
{
	if (options->range_to == 0xFFFFFFFF) {
		options->range_to = blocks - 1;
	} else if (options->range_to >= blocks) {
		fprintf(stderr, "Upper bound of block range '%u' "
				"larger than highest block in file '%lu'.\n",
				options->range_to, blocks - 1);
		return ERANGE;
	}
	return 0;
}


/* read from the input file and invoke merkle_update() to
 * write the merkle tree to the output file */
static int hash_write(struct cmd_options *options)
{
	struct merkle_context update;
	struct stat stat;
	uint64_t total_blocks;
	int status;

	update.verbose = options->verbose;
	update.k = options->tree_width;
	update.block_size = options->block_size;
	update.node_size = update.k * SHA_DIGEST_LENGTH;

	/* open input file for read */
	update.fd_in = open(options->source, O_RDONLY);
	if (update.fd_in == -1) {
		status = errno;
		fprintf(stderr, "Failed to open input file "
				"'%s' with error %d.\n",
				options->source, status);
		goto out;
	}

	/* open/create output file */
	update.fd_out = open(options->hash, O_RDWR | O_CREAT, 0600);
	if (update.fd_out == -1) {
		status = errno;
		fprintf(stderr, "Failed to open hash file "
				"'%s' with error %d.\n",
				options->hash, status);
		goto out_close_in;
	}

	/* allocate buffers needed for i/o */
	update.block_buffer = (unsigned char*)malloc(update.block_size);
	if (update.block_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate block buffer "
				"(%lu bytes) with error %d.\n",
				update.block_size, status);
		goto out_close_out;
	}
	update.node_buffer = (unsigned char*)malloc(update.node_size);
	if (update.node_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate node buffer "
				"(%lu bytes) with error %d.\n",
				update.node_size, status);
		goto out_free_block;
	}

	/* get the input file size */
	if (fstat(update.fd_in, &stat) == -1) {
		status = errno;
		fprintf(stderr, "Failed to get file size of "
				"input file '%s' with error %d.\n",
				options->source, status);
		goto out_free_node;
	}

	total_blocks = stat.st_size / update.block_size +
		(stat.st_size % update.block_size ? 1 : 0);

	status = update_range(options, total_blocks);
	if (status)
		goto out_free_node;

	/* start the update traversal */
	status = merkle_update(&update, options->range_from,
			options->range_to, total_blocks);
	if (status) {
		fprintf(stderr, "hash update failed with error %d.\n",
				status);
		goto out_free_node;
	}

	printf("hash update successful\n");

out_free_node:
	free(update.node_buffer);
out_free_block:
	free(update.block_buffer);
out_close_out:
	close(update.fd_out);
out_close_in:
	close(update.fd_in);
out:
	return status;
}

/* truncate the input file and invoke merkle_truncate() to
 * write an updated hash tree file */
static int hash_truncate(struct cmd_options *options)
{
	struct merkle_context truncate;
	struct stat stat;
	uint64_t total_blocks, new_size;
	int status;

	truncate.verbose = options->verbose;
	truncate.k = options->tree_width;
	truncate.block_size = options->block_size;
	truncate.node_size = truncate.k * SHA_DIGEST_LENGTH;
	truncate.partial = 0;

	/* open input file for read/write */
	truncate.fd_in = open(options->source, O_RDWR);
	if (truncate.fd_in == -1) {
		status = errno;
		fprintf(stderr, "Failed to open input file "
				"'%s' with error %d.\n",
				options->source, status);
		goto out;
	}

	/* open output file for read/write */
	truncate.fd_out = open(options->hash, O_RDWR);
	if (truncate.fd_out == -1) {
		status = errno;
		fprintf(stderr, "Failed to open hash file "
				"'%s' with error %d.\n",
				options->hash, status);
		goto out_close_in;
	}

	/* allocate buffers needed for i/o */
	truncate.block_buffer = (unsigned char*)malloc(truncate.block_size);
	if (truncate.block_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate block buffer "
				"(%lu bytes) with error %d.\n",
				truncate.block_size, status);
		goto out_close_out;
	}
	truncate.node_buffer = (unsigned char*)malloc(truncate.node_size);
	if (truncate.node_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate node buffer "
				"(%lu bytes) with error %d.\n",
				truncate.node_size, status);
		goto out_free_block;
	}

	/* get the input file size */
	if (fstat(truncate.fd_in, &stat) == -1) {
		status = errno;
		fprintf(stderr, "Failed to get file size of "
				"input file '%s' with error %d.\n",
				options->source, status);
		goto out_free_node;
	}

	total_blocks = stat.st_size / truncate.block_size +
		(stat.st_size % truncate.block_size ? 1 : 0);

	if (options->range_to >= total_blocks - 1) {
		status = ERANGE;
		fprintf(stderr, "The truncate operation requires a new last "
				"block smaller than %lu, specified in the second "
				"argument of the -r option.\n", total_blocks - 1);
		goto out_free_node;
	}

	/* truncate the input file after the given block */
	new_size = (options->range_to + 1) * options->block_size;

	if (ftruncate(truncate.fd_in, new_size) == -1) {
		status = errno;
		fprintf(stderr, "Failed to truncate input file "
				"'%s' with error %d.\n",
				options->source, status);
		goto out_free_node;
	}

	/* start the truncate traversal */
	status = merkle_truncate(&truncate, options->range_to);
	if (status) {
		fprintf(stderr, "hash truncate failed with error %d.\n",
				status);
		goto out_free_node;
	}

	printf("hash truncate successful\n");

out_free_node:
	free(truncate.node_buffer);
out_free_block:
	free(truncate.block_buffer);
out_close_out:
	close(truncate.fd_out);
out_close_in:
	close(truncate.fd_in);
out:
	return status;
}

/* read from the input file and invoke merkle_verify() to
 * compare the generated hashes with the hash tree file */
static int hash_verify(struct cmd_options *options)
{
	struct merkle_context verify;
	struct stat stat;
	uint64_t total_blocks;
	int status;

	verify.verbose = options->verbose;
	verify.k = options->tree_width;
	verify.block_size = options->block_size;
	verify.node_size = verify.k * SHA_DIGEST_LENGTH;

	/* open input file for read */
	verify.fd_in = open(options->source, O_RDONLY);
	if (verify.fd_in == -1) {
		status = errno;
		fprintf(stderr, "Failed to open input file "
				"'%s' with error %d.\n",
				options->source, status);
		goto out;
	}

	/* open output file for read */
	verify.fd_out = open(options->hash, O_RDONLY);
	if (verify.fd_out == -1) {
		status = errno;
		fprintf(stderr, "Failed to open hash file "
				"'%s' with error %d.\n",
				options->hash, status);
		goto out_close_in;
	}

	/* allocate buffers needed for i/o */
	verify.block_buffer = (unsigned char*)malloc(verify.block_size);
	if (verify.block_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate block buffer "
				"(%lu bytes) with error %d.\n",
				verify.block_size, status);
		goto out_close_out;
	}
	verify.node_buffer = (unsigned char*)malloc(verify.node_size);
	if (verify.node_buffer == NULL) {
		status = errno;
		fprintf(stderr, "Failed to allocate node buffer "
				"(%lu bytes) with error %d.\n",
				verify.node_size, status);
		goto out_free_block;
	}

	/* get the input file size */
	if (fstat(verify.fd_in, &stat) == -1) {
		status = errno;
		fprintf(stderr, "Failed to get file size of "
				"input file '%s' with error %d.\n",
				options->source, status);
		goto out_free_node;
	}

	total_blocks = stat.st_size / verify.block_size +
		(stat.st_size % verify.block_size ? 1 : 0);

	status = update_range(options, total_blocks);
	if (status)
		goto out_free_node;

	/* start the verification traversal */
	status = merkle_verify(&verify, options->range_from,
			options->range_to, total_blocks);
	if (status) {
		fprintf(stderr, "hash verification failed with error %d.\n",
				status);
		goto out_free_node;
	}

	printf("hash verification successful\n");

out_free_node:
	free(verify.node_buffer);
out_free_block:
	free(verify.block_buffer);
out_close_out:
	close(verify.fd_out);
out_close_in:
	close(verify.fd_in);
out:
	return status;
}


/* parse command line options */
int parse(struct cmd_options *options, int argc, char *argv[])
{
	argc--;
	argv++;
	if (argc < 1) {
		fprintf(stderr, "Missing argument for operation.\n");
		return -1;
	}
	options->operation = argv[0];
	if (strcmp(options->operation, "write") &&
			strcmp(options->operation, "truncate") &&
			strcmp(options->operation, "verify")) {
		fprintf(stderr, "Unknown operation '%s'.\n", options->operation);
		return -1;
	}

	argc--;
	argv++;
	while (argc && argv[0][0] == '-') {
		if (strcmp(argv[0], "-b") == 0) {
			if (argc < 2) {
				fprintf(stderr, "Option -b missing argument.\n");
				return -1;
			}
			options->block_size = atoi(argv[1]);
			if (options->block_size == 0) {
				fprintf(stderr, "Invalid block size '%s'.\n", argv[1]);
				return -1;
			}
			argc -= 2;
			argv += 2;
		} else if (strcmp(argv[0], "-k") == 0) {
			if (argc < 2) {
				fprintf(stderr, "Option -k missing argument.\n");
				return -1;
			}
			options->tree_width = atoi(argv[1]);
			if (options->tree_width != 2 && options->tree_width != 4 &&
					options->tree_width != 8 && options->tree_width != 16 &&
					options->tree_width != 32 && options->tree_width != 64 &&
					options->tree_width != 128) {
				fprintf(stderr, "Invalid value for k='%s': not a "
						"power of 2 between 2 and 128.\n", argv[1]);
				return -1;
			}
			argc -= 2;
			argv += 2;
		} else if (strcmp(argv[0], "-r") == 0) {
			if (argc < 3) {
				fprintf(stderr, "Option -r missing arguments.\n");
				return -1;
			}
			options->range_from = atoi(argv[1]);
			options->range_to = atoi(argv[2]);
			if (options->range_from >= options->range_to) {
				fprintf(stderr, "Invalid block range: %s to %s.\n",
						argv[1], argv[2]);
				return -1;
			}
			argc -= 3;
			argv += 3;
		} else if (strcmp(argv[0], "-v") == 0) {
			options->verbose = 1;
			argc--;
			argv++;
		} else {
			fprintf(stderr, "Unrecognized option %s.\n", argv[0]);
			argc--;
			argv++;
		}
	}

	if (argc < 1) {
		fprintf(stderr, "Missing argument for input file.\n");
		return -1;
	}
	options->source = argv[0];

	if (argc < 2) {
		fprintf(stderr, "Missing argument for hash file.\n");
		return -1;
	}
	options->hash = argv[1];
	return 0;
}

int main(int argc, char *argv[])
{
	struct cmd_options options = {
		NULL,
		NULL,
		NULL,
		4,
		4096,
		0,
		0xFFFFFFFF,
		0
	};
	if (parse(&options, argc, argv) == 0) {
		if (strcmp(options.operation, "write") == 0)
			return hash_write(&options);

		if (strcmp(options.operation, "truncate") == 0)
			return hash_truncate(&options);

		if (strcmp(options.operation, "verify") == 0)
			return hash_verify(&options);
	}
	return usage(argv[0]);
}
