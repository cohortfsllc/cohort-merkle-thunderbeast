// vim: ts=2 sw=2 smarttab

#ifndef COHORT_BLOCK_READER_H
#define COHORT_BLOCK_READER_H

#include <vector>
#include <iostream>
#include <unistd.h>


namespace cohort {

	class block_reader {
		private:
			const int fd;
			const size_t filesize;
			std::vector<unsigned char> buffer;

		public:
			block_reader(int fd, size_t filesize, size_t blocksize)
				: fd(fd), filesize(filesize), buffer(blocksize)
			{
			}

			uint64_t blocksize() const
			{
				return buffer.size();
			}

			uint64_t count() const
			{
				size_t count = filesize / buffer.size();
				if (filesize % buffer.size())
					count++;
				return count;
			}

			// read a block and return a pointer into the buffer
			unsigned char* read(uint64_t block)
			{
				if (lseek64(fd, block * blocksize(), SEEK_SET) == -1)
				{
					std::cerr << "block_reader: lseek() failed with error " << errno << std::endl;
					return NULL;
				}
				ssize_t bytes = ::read(fd, buffer.data(), buffer.size());
				if (bytes == -1)
				{
					std::cerr << "block_reader: read() failed with error " << errno << std::endl;
					return NULL;
				}

				// zero-fill any bytes after what was read
				if (bytes < buffer.size())
					std::fill(buffer.data() + bytes, buffer.data() + buffer.size(), 0);

				return buffer.data();
			}
	};
}

#endif // COHORT_BLOCK_READER_H
