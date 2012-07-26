// vim: ts=2 sw=2 smarttab

#include "hash_tree.h"
#include "unit_test.h"


using namespace cohort;

namespace
{
	// hash_tree::depth()
	int test_tree_depth()
	{
		for (int i = 0; i < 64; i++)
		{
			test_check_equal(hash_tree(2).depth(math::powi(2, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(2).depth(math::powi(2, i)), i + 1);
		}
		for (int i = 0; i < 32; i++)
		{
			test_check_equal(hash_tree(4).depth(math::powi(4, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(4).depth(math::powi(4, i)), i + 1);
		}
		for (int i = 0; i < 22; i++)
		{
			test_check_equal(hash_tree(8).depth(math::powi(8, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(8).depth(math::powi(8, i)), i + 1);
		}
		for (int i = 0; i < 16; i++)
		{
			test_check_equal(hash_tree(16).depth(math::powi(16, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(16).depth(math::powi(16, i)), i + 1);
		}
		for (int i = 0; i < 13; i++)
		{
			test_check_equal(hash_tree(32).depth(math::powi(32, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(32).depth(math::powi(32, i)), i + 1);
		}
		for (int i = 0; i < 11; i++)
		{
			test_check_equal(hash_tree(64).depth(math::powi(64, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(64).depth(math::powi(64, i)), i + 1);
		}
		for (int i = 0; i < 10; i++)
		{
			test_check_equal(hash_tree(128).depth(math::powi(128, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(128).depth(math::powi(128, i)), i + 1);
		}
		return 0;
	}

	// hash_tree::size()
	int test_tree_size()
	{
		// k=2
		test_check_equal(hash_tree(2).size(0), 0);
		test_check_equal(hash_tree(2).size(1), 1);
		test_check_equal(hash_tree(2).size(2), 3);
		test_check_equal(hash_tree(2).size(3), 7);
		test_check_equal(hash_tree(2).size(4), 15);
		test_check_equal(hash_tree(2).size(5), 31);
		test_check_equal(hash_tree(2).size(6), 63);
		test_check_equal(hash_tree(2).size(7), 127);
		test_check_equal(hash_tree(2).size(8), 255);
		test_check_equal(hash_tree(2).size(9), 511);
		test_check_equal(hash_tree(2).size(10), 1023);
		// k=3
		test_check_equal(hash_tree(3).size(0), 0);
		test_check_equal(hash_tree(3).size(1), 1);
		test_check_equal(hash_tree(3).size(2), 4);
		test_check_equal(hash_tree(3).size(3), 13);
		test_check_equal(hash_tree(3).size(4), 40);
		test_check_equal(hash_tree(3).size(5), 121);
		test_check_equal(hash_tree(3).size(6), 364);
		test_check_equal(hash_tree(3).size(7), 1093);
		test_check_equal(hash_tree(3).size(8), 3280);
		test_check_equal(hash_tree(3).size(9), 9841);
		test_check_equal(hash_tree(3).size(10), 29524);
		// k=4
		test_check_equal(hash_tree(4).size(0), 0);
		test_check_equal(hash_tree(4).size(1), 1);
		test_check_equal(hash_tree(4).size(2), 5);
		test_check_equal(hash_tree(4).size(3), 21);
		test_check_equal(hash_tree(4).size(4), 85);
		test_check_equal(hash_tree(4).size(5), 341);
		test_check_equal(hash_tree(4).size(6), 1365);
		test_check_equal(hash_tree(4).size(7), 5461);
		test_check_equal(hash_tree(4).size(8), 21845);
		test_check_equal(hash_tree(4).size(9), 87381);
		test_check_equal(hash_tree(4).size(10), 349525);
		// k=16
		test_check_equal(hash_tree(16).size(0), 0);
		test_check_equal(hash_tree(16).size(1), 1);
		test_check_equal(hash_tree(16).size(2), 17);
		test_check_equal(hash_tree(16).size(3), 273);
		test_check_equal(hash_tree(16).size(4), 4369);
		test_check_equal(hash_tree(16).size(5), 69905);
		test_check_equal(hash_tree(16).size(6), 1118481);
		test_check_equal(hash_tree(16).size(7), 17895697);
		test_check_equal(hash_tree(16).size(8), 286331153);
		// results over 32 bits
		test_check_equal(hash_tree(16).size(9), 4581298449);
		test_check_equal(hash_tree(16).size(10), 73300775185);
		test_check_equal(hash_tree(16).size(11), 1172812402961);
		test_check_equal(hash_tree(16).size(12), 18764998447377);
		test_check_equal(hash_tree(16).size(13), 300239975158033);
		test_check_equal(hash_tree(16).size(14), 4803839602528529);
		test_check_equal(hash_tree(16).size(15), 76861433640456465);
		test_check_equal(hash_tree(16).size(16), 1229782938247303441);
		// k=255
		test_check_equal(hash_tree(255).size(0), 0);
		test_check_equal(hash_tree(255).size(1), 1);
		test_check_equal(hash_tree(255).size(2), 256);
		test_check_equal(hash_tree(255).size(3), 65281);
		test_check_equal(hash_tree(255).size(4), 16646656);
		test_check_equal(hash_tree(255).size(5), 4244897281);
		// results over 32 bits
		test_check_equal(hash_tree(255).size(6), 1082448806656);
		test_check_equal(hash_tree(255).size(7), 276024445697281);
		test_check_equal(hash_tree(255).size(8), 70386233652806656);
		test_check_equal(hash_tree(255).size(9), 17948489581465697281ULL);
		return 0;
	}

	// hash_tree::root()
	int test_tree_root()
	{
		// k=2
		test_check_equal(hash_tree(2).root(1), 0);
		test_check_equal(hash_tree(2).root(2), 1);
		test_check_equal(hash_tree(2).root(3), 3);
		test_check_equal(hash_tree(2).root(4), 7);
		test_check_equal(hash_tree(2).root(5), 15);
		// k=3
		test_check_equal(hash_tree(3).root(1), 0);
		test_check_equal(hash_tree(3).root(2), 1);
		test_check_equal(hash_tree(3).root(3), 4);
		test_check_equal(hash_tree(3).root(4), 13);
		test_check_equal(hash_tree(3).root(5), 40);
		// k=4
		test_check_equal(hash_tree(4).root(1), 0);
		test_check_equal(hash_tree(4).root(2), 1);
		test_check_equal(hash_tree(4).root(3), 5);
		test_check_equal(hash_tree(4).root(4), 21);
		test_check_equal(hash_tree(4).root(5), 85);
		// k=16
		test_check_equal(hash_tree(16).root(1), 0);
		test_check_equal(hash_tree(16).root(2), 1);
		test_check_equal(hash_tree(16).root(3), 17);
		test_check_equal(hash_tree(16).root(4), 273);
		test_check_equal(hash_tree(16).root(5), 4369);
		// k=255
		test_check_equal(hash_tree(255).root(1), 0);
		test_check_equal(hash_tree(255).root(2), 1);
		test_check_equal(hash_tree(255).root(3), 256);
		test_check_equal(hash_tree(255).root(4), 65281);
		test_check_equal(hash_tree(255).root(5), 16646656);
		return 0;
	}

	// hash_tree::child_index()
	int test_tree2_child_index()
	{
		// depth=2
		test_check_equal(hash_tree(2).child_index(1, 0, 1, 0), 0);
		test_check_equal(hash_tree(2).child_index(1, 1, 1, 0), 2);
		test_check_equal(hash_tree(2).child_index(4, 0, 1, 0), 5);
		test_check_equal(hash_tree(2).child_index(4, 1, 1, 0), 6);
		test_check_equal(hash_tree(2).child_index(28, 0, 1, 0), 29);
		test_check_equal(hash_tree(2).child_index(28, 1, 1, 0), 30);
		test_check_equal(hash_tree(2).child_index(42, 0, 1, 0), 43);
		test_check_equal(hash_tree(2).child_index(42, 1, 1, 0), 44);
		// depth=3
		test_check_equal(hash_tree(2).child_index(3, 0, 3, 1), 1);
		test_check_equal(hash_tree(2).child_index(3, 1, 3, 1), 4);
		test_check_equal(hash_tree(2).child_index(8, 0, 3, 1), 9);
		test_check_equal(hash_tree(2).child_index(8, 1, 3, 1), 12);
		test_check_equal(hash_tree(2).child_index(17, 0, 3, 1), 18);
		test_check_equal(hash_tree(2).child_index(17, 1, 3, 1), 21);
		test_check_equal(hash_tree(2).child_index(41, 0, 3, 1), 42);
		test_check_equal(hash_tree(2).child_index(41, 1, 3, 1), 45);
		// depth=4
		test_check_equal(hash_tree(2).child_index(7, 0, 7, 3), 3);
		test_check_equal(hash_tree(2).child_index(7, 1, 7, 3), 8);
		test_check_equal(hash_tree(2).child_index(16, 0, 7, 3), 17);
		test_check_equal(hash_tree(2).child_index(16, 1, 7, 3), 24);
		test_check_equal(hash_tree(2).child_index(33, 0, 7, 3), 34);
		test_check_equal(hash_tree(2).child_index(33, 1, 7, 3), 41);
		test_check_equal(hash_tree(2).child_index(48, 0, 7, 3), 49);
		test_check_equal(hash_tree(2).child_index(48, 1, 7, 3), 56);
		// depth=5
		test_check_equal(hash_tree(2).child_index(15, 0, 15, 7), 7);
		test_check_equal(hash_tree(2).child_index(15, 1, 15, 7), 16);
		test_check_equal(hash_tree(2).child_index(32, 0, 15, 7), 33);
		test_check_equal(hash_tree(2).child_index(32, 1, 15, 7), 48);
		// depth=6
		test_check_equal(hash_tree(2).child_index(31, 0, 31, 15), 15);
		test_check_equal(hash_tree(2).child_index(31, 1, 31, 15), 32);
		return 0;
	}

	int test_tree3_child_index()
	{
		// depth=2
		test_check_equal(hash_tree(3).child_index(1, 0, 1, 0), 0);
		test_check_equal(hash_tree(3).child_index(1, 1, 1, 0), 2);
		test_check_equal(hash_tree(3).child_index(1, 2, 1, 0), 3);
		test_check_equal(hash_tree(3).child_index(23, 0, 1, 0), 24);
		test_check_equal(hash_tree(3).child_index(23, 1, 1, 0), 25);
		test_check_equal(hash_tree(3).child_index(23, 2, 1, 0), 26);
		test_check_equal(hash_tree(3).child_index(32, 0, 1, 0), 33);
		test_check_equal(hash_tree(3).child_index(32, 1, 1, 0), 34);
		test_check_equal(hash_tree(3).child_index(32, 2, 1, 0), 35);
		// depth=3
		test_check_equal(hash_tree(3).child_index(4, 0, 4, 1), 1);
		test_check_equal(hash_tree(3).child_index(4, 1, 4, 1), 5);
		test_check_equal(hash_tree(3).child_index(4, 2, 4, 1), 9);
		test_check_equal(hash_tree(3).child_index(14, 0, 4, 1), 15);
		test_check_equal(hash_tree(3).child_index(14, 1, 4, 1), 19);
		test_check_equal(hash_tree(3).child_index(14, 2, 4, 1), 23);
		test_check_equal(hash_tree(3).child_index(27, 0, 4, 1), 28);
		test_check_equal(hash_tree(3).child_index(27, 1, 4, 1), 32);
		test_check_equal(hash_tree(3).child_index(27, 2, 4, 1), 36);
		// depth=4
		test_check_equal(hash_tree(3).child_index(13, 0, 13, 4), 4);
		test_check_equal(hash_tree(3).child_index(13, 1, 13, 4), 14);
		test_check_equal(hash_tree(3).child_index(13, 2, 13, 4), 27);
		return 0;
	}

	int test_tree4_child_index()
	{
		// depth=2
		test_check_equal(hash_tree(4).child_index(1, 0, 1, 0), 0);
		test_check_equal(hash_tree(4).child_index(1, 1, 1, 0), 2);
		test_check_equal(hash_tree(4).child_index(1, 2, 1, 0), 3);
		test_check_equal(hash_tree(4).child_index(1, 3, 1, 0), 4);
		test_check_equal(hash_tree(4).child_index(28, 0, 1, 0), 29);
		test_check_equal(hash_tree(4).child_index(28, 1, 1, 0), 30);
		test_check_equal(hash_tree(4).child_index(28, 2, 1, 0), 31);
		test_check_equal(hash_tree(4).child_index(28, 3, 1, 0), 32);
		// depth=3
		test_check_equal(hash_tree(4).child_index(5, 0, 5, 1), 1);
		test_check_equal(hash_tree(4).child_index(5, 1, 5, 1), 6);
		test_check_equal(hash_tree(4).child_index(5, 2, 5, 1), 11);
		test_check_equal(hash_tree(4).child_index(5, 3, 5, 1), 16);
		test_check_equal(hash_tree(4).child_index(43, 0, 5, 1), 44);
		test_check_equal(hash_tree(4).child_index(43, 1, 5, 1), 49);
		test_check_equal(hash_tree(4).child_index(43, 2, 5, 1), 54);
		test_check_equal(hash_tree(4).child_index(43, 3, 5, 1), 59);
		// depth=4
		test_check_equal(hash_tree(4).child_index(21, 0, 21, 5), 5);
		test_check_equal(hash_tree(4).child_index(21, 1, 21, 5), 22);
		test_check_equal(hash_tree(4).child_index(21, 2, 21, 5), 43);
		test_check_equal(hash_tree(4).child_index(21, 3, 21, 5), 64);
		return 0;
	}

	int test_tree_child_index()
	{
		test_require_equal(test_tree2_child_index(), 0);
		test_require_equal(test_tree3_child_index(), 0);
		test_require_equal(test_tree4_child_index(), 0);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	test_require_equal(test_tree_depth(), 0);
	test_require_equal(test_tree_size(), 0);
	test_require_equal(test_tree_root(), 0);
	test_require_equal(test_tree_child_index(), 0);
	return 0;
}
