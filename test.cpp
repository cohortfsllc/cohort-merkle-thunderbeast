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
		test_check_equal(hash_tree(2).depth(6), 4);
		test_check_equal(hash_tree(2).depth(7), 4);
		test_check_equal(hash_tree(2).depth(8), 4);
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
		// width=3
		test_check_equal(hash_tree(3).root(1), 0);
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
