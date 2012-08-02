#ifndef COHORT_MERKLE_UPDATE_H
#define COHORT_MERKLE_UPDATE_H

#include <stdint.h>


/* from visitor.h */
struct merkle_state;

/* visitor callbacks for update, for internal use by truncate */
int update_leaf(const struct merkle_state *node, uint64_t block,
		uint8_t position, void *user);
int update_node(const struct merkle_state *node,
		uint8_t depth, void *user);

/* common functions for file i/o */
int read_at(int fd, off_t offset, unsigned char *buffer, size_t length);
int write_at(int fd, off_t offset, unsigned char *buffer, size_t length);

#endif /* COHORT_MERKLE_UPDATE_H */
