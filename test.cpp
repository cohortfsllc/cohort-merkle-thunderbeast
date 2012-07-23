// vim: ts=2 sw=2 smarttab

#include "hash_tree.h"
#include "unit_test.h"


using namespace cohort;

namespace
{
	int test_depth()
	{
		// width=2
		test_check_equal(hash_tree(2).depth(1), 1);
		test_check_equal(hash_tree(2).depth(2), 2);
		test_check_equal(hash_tree(2).depth(3), 3);
		test_check_equal(hash_tree(2).depth(4), 3);
		test_check_equal(hash_tree(2).depth(5), 4);
		test_check_equal(hash_tree(2).depth(8), 4);
		test_check_equal(hash_tree(2).depth(9), 5);
		test_check_equal(hash_tree(2).depth(16), 5);
		test_check_equal(hash_tree(2).depth(17), 6);
		test_check_equal(hash_tree(2).depth(32), 6);
		test_check_equal(hash_tree(2).depth(64), 7);
		test_check_equal(hash_tree(2).depth(128), 8);
		test_check_equal(hash_tree(2).depth(0x40000000), 31);
		test_check_equal(hash_tree(2).depth(0x40000001), 32);
		test_check_equal(hash_tree(2).depth(0x80000000), 32);
		test_check_equal(hash_tree(2).depth(0x80000001), 33);

		for (int i = 0; i < 64; i++)
		{
			test_check_equal(hash_tree(2).depth(math::powi(2, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(2).depth(math::powi(2, i)), i + 1);
		}
		for (int i = 0; i < 41; i++)
		{
			test_check_equal(hash_tree(3).depth(math::powi(3, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(3).depth(math::powi(3, i)), i + 1);
		}
		for (int i = 0; i < 32; i++)
		{
			test_check_equal(hash_tree(4).depth(math::powi(4, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(4).depth(math::powi(4, i)), i + 1);
		}
		for (int i = 0; i < 16; i++)
		{
			test_check_equal(hash_tree(8).depth(math::powi(8, i - 1) + 1), i + 1);
			test_check_equal(hash_tree(8).depth(math::powi(8, i)), i + 1);
		}
		return 0;
	}

	int test_tree_size()
	{
		// width=2
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
		// width=3
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
		// width=4
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
		// width=16
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
		// width=255
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

	int test_tree_root()
	{
		// width=2
		test_check_equal(hash_tree(2).root(1), 0);
		test_check_equal(hash_tree(2).root(2), 2);
		test_check_equal(hash_tree(2).root(3), 6);
		test_check_equal(hash_tree(2).root(4), 14);
		test_check_equal(hash_tree(2).root(5), 30);
		test_check_equal(hash_tree(2).root(6), 62);
		test_check_equal(hash_tree(2).root(7), 126);
		test_check_equal(hash_tree(2).root(8), 254);
		test_check_equal(hash_tree(2).root(16), 0xFFFE);
		test_check_equal(hash_tree(2).root(32), 0xFFFFFFFE);
		test_check_equal(hash_tree(2).root(64), 0xFFFFFFFFFFFFFFFE);
		// width=3
		test_check_equal(hash_tree(3).root(1), 0);
		test_check_equal(hash_tree(3).root(2), 3);
		test_check_equal(hash_tree(3).root(3), 12);
		test_check_equal(hash_tree(3).root(4), 39);
		test_check_equal(hash_tree(3).root(5), 120);
		test_check_equal(hash_tree(3).root(6), 363);
		test_check_equal(hash_tree(3).root(7), 1092);
		test_check_equal(hash_tree(3).root(8), 3279);
		test_check_equal(hash_tree(3).root(16), 21523359);
		test_check_equal(hash_tree(3).root(32), 926510094425919ULL);
		test_check_equal(hash_tree(3).root(47), 12751446407874480372ULL);
		// width=4
		test_check_equal(hash_tree(4).root(1), 0);
		test_check_equal(hash_tree(4).root(2), 4);
		test_check_equal(hash_tree(4).root(3), 20);
		test_check_equal(hash_tree(4).root(4), 84);
		test_check_equal(hash_tree(4).root(5), 340);
		test_check_equal(hash_tree(4).root(6), 1364);
		test_check_equal(hash_tree(4).root(7), 5460);
		test_check_equal(hash_tree(4).root(8), 21844);
		test_check_equal(hash_tree(4).root(16), 0x55555554);
		test_check_equal(hash_tree(4).root(32), 0x5555555555555554);
		// width=16
		test_check_equal(hash_tree(16).root(1), 0);
		test_check_equal(hash_tree(16).root(2), 16);
		test_check_equal(hash_tree(16).root(3), 272);
		test_check_equal(hash_tree(16).root(4), 4368);
		test_check_equal(hash_tree(16).root(5), 0x11110);
		test_check_equal(hash_tree(16).root(6), 0x111110);
		test_check_equal(hash_tree(16).root(7), 0x1111110);
		test_check_equal(hash_tree(16).root(8), 0x11111110);
		test_check_equal(hash_tree(16).root(16), 0x1111111111111110);
		// width=255
		test_check_equal(hash_tree(255).root(1), 0);
		test_check_equal(hash_tree(255).root(2), 255);
		test_check_equal(hash_tree(255).root(3), 65280);
		test_check_equal(hash_tree(255).root(4), 16646655);
		test_check_equal(hash_tree(255).root(5), 4244897280);
		test_check_equal(hash_tree(255).root(6), 1082448806655UL);
		test_check_equal(hash_tree(255).root(7), 276024445697280ULL);
		test_check_equal(hash_tree(255).root(8), 70386233652806655ULL);
		test_check_equal(hash_tree(255).root(9), 17948489581465697280ULL);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	test_require_equal(test_depth(), 0);
	test_require_equal(test_tree_size(), 0);
	test_require_equal(test_tree_root(), 0);
	return 0;
}
