// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASH_FILE_H
#define COHORT_HASH_FILE_H

#include <vector>
#include <iostream>


namespace cohort {

	// stub class for reading/writing to the hash file
	class hash_file {
		private:
			const int fd;
			const uint64_t digest_size;
			std::vector<unsigned char> buffer;

		public:
			hash_file(int fd, uint64_t digest_size, uint8_t digests_per_node)
				: fd(fd), digest_size(digest_size), buffer(digest_size * digests_per_node)
			{
			}

			bool resize(size_t length)
			{
				if (::ftruncate64(fd, length) == -1)
				{
					std::cerr << "hash_file: ftruncate() failed with error " << errno << std::endl;
					return false;
				}
				return true;
			}

			// read an entire node into a private buffer and return a pointer
			const unsigned char* read(size_t offset, size_t length)
			{
				if (length > buffer.size())
				{
					std::cerr << "hash_file: requested length " << length
						<< " larger than buffer size " << buffer.size() << std::endl;
					return false;
				}
				if (::lseek64(fd, offset, SEEK_SET) == -1)
				{
					std::cerr << "hash_file: lseek() failed with error " << errno << std::endl;
					return false;
				}
				ssize_t bytes = ::read(fd, buffer.data(), buffer.size());
				if (bytes == -1)
				{
					std::cerr << "hash_file: read failed with error " << errno << std::endl;
					return NULL;
				}
				if (bytes < (ssize_t)length)
				{
					// zero-fill the rest of the buffer
					std::fill(buffer.begin() + bytes, buffer.end(), 0);
				}
				return buffer.data();
			}

			// write a single hash
			bool write(const unsigned char *data, size_t offset)
			{
				if (lseek64(fd, offset, SEEK_SET) == -1)
				{
					std::cerr << "hash_file: lseek() failed with error " << errno << std::endl;
					return false;
				}
				ssize_t bytes = ::write(fd, data, digest_size);
				if (bytes == -1)
				{
					std::cerr << "hash_file: write failed with error " << errno << std::endl;
					return false;
				}
				else if (bytes < (ssize_t)digest_size)
				{
					std::cerr << "hash_file: wrote less than requested" << std::endl;
					return false;
				}
				return true;
			}
	};

}

#endif // COHORT_HASH_FILE_H
