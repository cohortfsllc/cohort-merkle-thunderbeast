// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASHER_H
#define COHORT_HASHER_H

#include <openssl/sha.h>


namespace cohort {

	// wrapper class to encapsulate the openssl sha1 hashing algorithm
	class hasher {
		private:
			SHA_CTX context;

		public:
			typedef unsigned char* buffer_type;
			typedef const unsigned char* const_buffer_type;
			typedef unsigned long size_type;

			static const size_type DIGEST_SIZE = SHA_DIGEST_LENGTH;

			hasher()
			{
				SHA1_Init(&context);
			}

			void process(const_buffer_type data, size_type length)
			{
				SHA1_Update(&context, data, length);
			}

			void finish(buffer_type buffer)
			{
				SHA1_Final(buffer, &context);
			}
	};

}

#endif // COHORT_HASHER_H
