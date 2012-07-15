#ifndef COHORT_UNIT_TEST_H
#define COHORT_UNIT_TEST_H

#include <iostream>
#include <exception>


#define test_check(expression) \
	do { \
		bool result = (expression); \
		if (!result) \
		{ \
			std::cout << "warning at " << __FILE__ << ":" << __LINE__ << " '" \
				#expression << "' returned false" << std::endl; \
		} \
	} while (0)

#define test_check_equal(expression, expected) \
	do { \
		auto result = (expression); \
		if (result != expected) \
		{ \
			std::cout << __FILE__ << "(" << __LINE__ << "): warning '" \
				<< #expression << "' returned " << result \
				<< " (expected " << expected << ")" << std::endl; \
		} \
	} while (0)

#define test_check_nothrow(expression) \
	try { (expression); } \
	catch (const std::exception &e) \
	{ \
		std::cout << "warning at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' threw exception: " \
			<< e.what() << std::endl; \
	}

#define test_check_throws(expression, expected) \
	try \
	{ \
		(expression); \
		std::cout << "warning at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' did not throw an exception" << std::endl; \
	} \
	catch (expected) {} \
	catch (const std::exception &e) \
	{ \
		std::cout << "warning at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' threw exception: " << e.what() \
			<< " (expected " << #expected << ")" << std::endl; \
	}

#define test_require(expression) \
	do { \
		bool result = (expression); \
		if (!result) \
		{ \
			std::cout << "error at " << __FILE__ << ":" << __LINE__ << " '" \
				<< #expression << "' returned false" << std::endl; \
			return -1; \
		} \
	} while (0)

#define test_require_equal(expression, expected) \
	do { \
		auto result = (expression); \
		if (result != expected) \
		{ \
			std::cout << "error at " << __FILE__ << ":" << __LINE__ << " '" \
				<< #expression << "' returned " << result \
				<< " (expected " << expected << ")" << std::endl; \
			return -1; \
		} \
	} while (0)

#define test_require_nothrow(expression) \
	try { (expression); } \
	catch (const std::exception &e) \
	{ \
		std::cout << "error at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' threw exception: " \
			<< e.what() << std::endl; \
		return -1; \
	}

#define test_require_throws(expression, expected) \
	try \
	{ \
		(expression); \
		std::cout << "error at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' did not throw an exception" << std::endl; \
	} \
	catch (expected) {} \
	catch (const std::exception &e) \
	{ \
		std::cout << "error at " << __FILE__ << ":" << __LINE__ << " '" \
			<< #expression << "' threw exception: " \
			<< e.what() << " (expected " << #expected << ")" << std::endl; \
		return -1; \
	}

#endif // COHORT_UNIT_TEST_H
