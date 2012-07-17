// vim: ts=2 sw=2 smarttab

#include "hashtree.h"
#include "unit_test.h"


using namespace cohort;

namespace
{
	int test_tree_depth()
	{
		// width=2
		test_check_equal(hash_tree<2>().tree_depth(1), 1);
		test_check_equal(hash_tree<2>().tree_depth(2), 2);
		test_check_equal(hash_tree<2>().tree_depth(3), 3);
		test_check_equal(hash_tree<2>().tree_depth(4), 3);
		test_check_equal(hash_tree<2>().tree_depth(5), 4);
		test_check_equal(hash_tree<2>().tree_depth(6), 4);
		test_check_equal(hash_tree<2>().tree_depth(7), 4);
		test_check_equal(hash_tree<2>().tree_depth(8), 4);
		return 0;
	}

	int test_tree_size()
	{
		// width=2
		test_check_equal(tree_size<2>()(0), 0);
		test_check_equal(tree_size<2>()(1), 1);
		test_check_equal(tree_size<2>()(2), 3);
		test_check_equal(tree_size<2>()(3), 7);
		test_check_equal(tree_size<2>()(4), 15);
		test_check_equal(tree_size<2>()(5), 31);
		test_check_equal(tree_size<2>()(6), 63);
		test_check_equal(tree_size<2>()(7), 127);
		test_check_equal(tree_size<2>()(8), 255);
		test_check_equal(tree_size<2>()(9), 511);
		test_check_equal(tree_size<2>()(10), 1023);
		// width=3
		test_check_equal(tree_size<3>()(0), 0);
		test_check_equal(tree_size<3>()(1), 1);
		test_check_equal(tree_size<3>()(2), 4);
		test_check_equal(tree_size<3>()(3), 13);
		test_check_equal(tree_size<3>()(4), 40);
		test_check_equal(tree_size<3>()(5), 121);
		test_check_equal(tree_size<3>()(6), 364);
		test_check_equal(tree_size<3>()(7), 1093);
		test_check_equal(tree_size<3>()(8), 3280);
		test_check_equal(tree_size<3>()(9), 9841);
		test_check_equal(tree_size<3>()(10), 29524);
		// width=4
		test_check_equal(tree_size<4>()(0), 0);
		test_check_equal(tree_size<4>()(1), 1);
		test_check_equal(tree_size<4>()(2), 5);
		test_check_equal(tree_size<4>()(3), 21);
		test_check_equal(tree_size<4>()(4), 85);
		test_check_equal(tree_size<4>()(5), 341);
		test_check_equal(tree_size<4>()(6), 1365);
		test_check_equal(tree_size<4>()(7), 5461);
		test_check_equal(tree_size<4>()(8), 21845);
		test_check_equal(tree_size<4>()(9), 87381);
		test_check_equal(tree_size<4>()(10), 349525);
		// width=16
		test_check_equal(tree_size<16>()(0), 0);
		test_check_equal(tree_size<16>()(1), 1);
		test_check_equal(tree_size<16>()(2), 17);
		test_check_equal(tree_size<16>()(3), 273);
		test_check_equal(tree_size<16>()(4), 4369);
		test_check_equal(tree_size<16>()(5), 69905);
		test_check_equal(tree_size<16>()(6), 1118481);
		test_check_equal(tree_size<16>()(7), 17895697);
		test_check_equal(tree_size<16>()(8), 286331153);
		// results over 32 bits
		test_check_equal(tree_size<16>()(9), 4581298449);
		test_check_equal(tree_size<16>()(10), 73300775185);
		test_check_equal(tree_size<16>()(11), 1172812402961);
		test_check_equal(tree_size<16>()(12), 18764998447377);
		test_check_equal(tree_size<16>()(13), 300239975158033);
		test_check_equal(tree_size<16>()(14), 4803839602528529);
		test_check_equal(tree_size<16>()(15), 76861433640456465);
		test_check_equal(tree_size<16>()(16), 1229782938247303441);
		// width=255
		test_check_equal(tree_size<255>()(0), 0);
		test_check_equal(tree_size<255>()(1), 1);
		test_check_equal(tree_size<255>()(2), 256);
		test_check_equal(tree_size<255>()(3), 65281);
		test_check_equal(tree_size<255>()(4), 16646656);
		test_check_equal(tree_size<255>()(5), 4244897281);
		// results over 32 bits
		test_check_equal(tree_size<255>()(6), 1082448806656);
		test_check_equal(tree_size<255>()(7), 276024445697281);
		test_check_equal(tree_size<255>()(8), 70386233652806656);
		test_check_equal(tree_size<255>()(9), 17948489581465697281ULL);
		// width=0x7FFFFFFF
		test_check_equal(tree_size<0x7FFFFFFF>()(0), 0);
		test_check_equal(tree_size<0x7FFFFFFF>()(1), 1);
		test_check_equal(tree_size<0x7FFFFFFF>()(2), 0x80000000);
		test_check_equal(tree_size<0x7FFFFFFF>()(3), 0x3FFFFFFF80000001);
		// width=0xFFFFFFFF
		test_check_equal(tree_size<0xFFFFFFFF>()(0), 0);
		test_check_equal(tree_size<0xFFFFFFFF>()(1), 1);
		test_check_equal(tree_size<0xFFFFFFFF>()(2), 0x100000000);
		test_check_equal(tree_size<0xFFFFFFFF>()(3), 0xFFFFFFFF00000001);
		return 0;
	}

	int test_tree_root()
	{
		// width=2
		test_check_equal(hash_tree<2>().tree_root(1), 0);
		test_check_equal(hash_tree<2>().tree_root(2), 2);
		test_check_equal(hash_tree<2>().tree_root(3), 6);
		test_check_equal(hash_tree<2>().tree_root(4), 14);
		return 0;
	}

	int test_update2_full()
	{
		blockset blocks;
		for (uint64_t i = 0; i < 16; i++)
			blocks.insert(i);

		test_check_equal(hash_tree<2>().update(blocks, 16), 30);
		return 0;
	}

	int test_update2_sparse()
	{
		blockset blocks;
		blocks.insert(4);
		blocks.insert(5);
		blocks.insert(6);
		blocks.insert(19);
		blocks.insert(22);
		blocks.insert(23);

		test_check_equal(hash_tree<2>().update(blocks, 24), 17);
		return 0;
	}

	int test_update2_big()
	{
		blockset block0;
		block0.insert(0);
		test_check_equal(hash_tree<2>().update(block0, 0x80000000), 31);

		blockset block7FFFFFFF;
		block7FFFFFFF.insert(0x7FFFFFFF);
		test_check_equal(hash_tree<2>().update(block7FFFFFFF, 0x80000000), 31);

		blockset block80000000;
		block80000000.insert(0x80000000);
		test_check_equal(hash_tree<2>().update(block80000000, 0x80000001), 32);
		return 0;
	}

	int test_update2()
	{
		test_check_equal(test_update2_full(), 0);
		test_check_equal(test_update2_sparse(), 0);
		test_check_equal(test_update2_big(), 0);
		return 0;
	}

	int test_update4_full()
	{
		blockset blocks;
		for (uint64_t i = 0; i < 16; i++)
			blocks.insert(i);

		test_check_equal(hash_tree<4>().update(blocks, 16), 20);
		return 0;
	}

	int test_update4()
	{
		test_check_equal(test_update4_full(), 0);
		return 0;
	}

	int test_update()
	{
		test_check_equal(test_update2(), 0);
		test_check_equal(test_update4(), 0);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	test_require_equal(test_tree_depth(), 0);
	test_require_equal(test_tree_size(), 0);
	test_require_equal(test_tree_root(), 0);
	test_require_equal(test_update(), 0);
	return 0;
}
