// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASH_FILE_H
#define COHORT_HASH_FILE_H

#include <vector>
#include <iostream>


namespace cohort {

	class hash_file {
		private:
			std::vector<unsigned char> buffer;

		public:
			hash_file(const char *filename) {}

			void resize(size_t length)
			{
				buffer.resize(length);
			}
			const unsigned char* read(size_t offset) const
			{
				return &buffer[offset];
			}
			unsigned char* write(size_t offset)
			{
				return &buffer[offset];
			}

		private: // disable assignment/copy
			const hash_file& operator=(const hash_file&);
			hash_file(const hash_file&);
	};

}

#endif // COHORT_HASH_FILE_H
