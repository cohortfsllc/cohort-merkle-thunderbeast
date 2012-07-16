// vim: ts=8 sw=2 smarttab

#include "hashtree.h"
#include "unit_test.h"


using namespace cohort;

namespace
{
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
		test_check_equal(tree_size<255>()(9), 17948489581465697281);
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

	int test_find_leaf()
	{
		// width=2
		test_check_equal(hash_tree<2>().find_leaf(0), 0);
		test_check_equal(hash_tree<2>().find_leaf(1), 1);
		test_check_equal(hash_tree<2>().find_leaf(2), 3);
		test_check_equal(hash_tree<2>().find_leaf(3), 4);
		test_check_equal(hash_tree<2>().find_leaf(4), 7);
		test_check_equal(hash_tree<2>().find_leaf(5), 8);
		test_check_equal(hash_tree<2>().find_leaf(6), 10);
		test_check_equal(hash_tree<2>().find_leaf(7), 11);
		test_check_equal(hash_tree<2>().find_leaf(0xF), 26);
		test_check_equal(hash_tree<2>().find_leaf(0xFF), 502);
		test_check_equal(hash_tree<2>().find_leaf(0xFFF), 8178);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFF), 131054);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFF), 2097130);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFF), 33554406);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFFF), 536870882);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFFFF), 8589934558);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFFFFF), 137438953434);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFFFFFF), 2199023255510);
		test_check_equal(hash_tree<2>().find_leaf(0xFFFFFFFFFFF), 35184372088786);
		test_check_equal(hash_tree<2>().find_leaf(0x7FFFFFFFFFFF), 281474976710607);
		// width=3
		test_check_equal(hash_tree<3>().find_leaf(0), 0);
		test_check_equal(hash_tree<3>().find_leaf(1), 1);
		test_check_equal(hash_tree<3>().find_leaf(2), 2);
		test_check_equal(hash_tree<3>().find_leaf(3), 4);
		test_check_equal(hash_tree<3>().find_leaf(4), 5);
		test_check_equal(hash_tree<3>().find_leaf(5), 6);
		test_check_equal(hash_tree<3>().find_leaf(6), 8);
		test_check_equal(hash_tree<3>().find_leaf(7), 9);
		test_check_equal(hash_tree<3>().find_leaf(0xF), 21);
		test_check_equal(hash_tree<3>().find_leaf(0xFF), 381);
		test_check_equal(hash_tree<3>().find_leaf(0xFFF), 6138);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFF), 98297);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFF), 1572856);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFF), 25165818);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFF), 402653173);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFF), 6442450931);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFF), 103079215094);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFF), 1649267441649);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFF), 26388279066608);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFFF), 422212465065970);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFFFF), 6755399441055724);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFFFFF), 108086391056891883);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFFFFFF), 1729382256910270440);
		test_check_equal(hash_tree<3>().find_leaf(0xFFFFFFFFFFFFFFFF), 9223372036854775783);
		// width=4
		test_check_equal(hash_tree<4>().find_leaf(0), 0);
		test_check_equal(hash_tree<4>().find_leaf(1), 1);
		test_check_equal(hash_tree<4>().find_leaf(2), 2);
		test_check_equal(hash_tree<4>().find_leaf(3), 3);
		test_check_equal(hash_tree<4>().find_leaf(4), 5);
		test_check_equal(hash_tree<4>().find_leaf(5), 6);
		test_check_equal(hash_tree<4>().find_leaf(6), 7);
		test_check_equal(hash_tree<4>().find_leaf(7), 8);
		test_check_equal(hash_tree<4>().find_leaf(0xF), 18);
		test_check_equal(hash_tree<4>().find_leaf(0xFF), 336);
		test_check_equal(hash_tree<4>().find_leaf(0xFFF), 5454);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFF), 87372);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFF), 1398090);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFFF), 22369608);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFFFF), 357913926);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFFFFF), 5726623044);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFFFFFF), 91625968962);
		test_check_equal(hash_tree<4>().find_leaf(0xFFFFFFFFFF), 1466015503680);
		test_check_equal(hash_tree<4>().find_leaf(0x7FFFFFFFFFF), 11728124029588);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	test_require_equal(test_tree_size(), 0);
	test_require_equal(test_find_leaf(), 0);
	return 0;
}
