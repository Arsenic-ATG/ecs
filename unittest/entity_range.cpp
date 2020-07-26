#include "catch.hpp"
#include <ecs/ecs.h>



TEST_CASE("entity_range ", "[entity]") {
    SECTION("iterator overflow test") {
        constexpr auto max = std::numeric_limits<ecs::entity_type>::max();
        ecs::entity_range r{max - 1, max};
        int64_t counter = 0;
        for (auto const ent : r)
            // end iterator becomes max+1, which is the same as std::numeric_limits<ecs::entity_type>::min()
            counter++;
        REQUIRE(counter == 2);
    }
    SECTION("intersection tests") {
        SECTION("No overlaps between ranges") {
            /// a: *****   *****   *****
            /// b:      ---     ---     ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {8, 12}, {16, 20}};
            std::vector<ecs::entity_range> vec_b{{5, 7}, {13, 15}, {21, 23}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(result.empty());
        }

        SECTION("Ranges in B are contained in ranges in A") {
            /// a: ***** ***** *****
            /// b:  ---   ---   ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {5, 9}, {10, 14}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {6, 8}, {11, 13}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{1, 3}.equals(result.at(0)));
            CHECK(ecs::entity_range{6, 8}.equals(result.at(1)));
            CHECK(ecs::entity_range{11, 13}.equals(result.at(2)));
        }

        SECTION("Ranges in A are contained in ranges in B") {
            /// a:  ---   ---   ---
            /// b: ***** ***** *****
            std::vector<ecs::entity_range> vec_a{{1, 3}, {6, 8}, {11, 13}};
            std::vector<ecs::entity_range> vec_b{{0, 4}, {5, 9}, {10, 14}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{1, 3}.equals(result.at(0)));
            CHECK(ecs::entity_range{6, 8}.equals(result.at(1)));
            CHECK(ecs::entity_range{11, 13}.equals(result.at(2)));
        }

        SECTION("Ranges in A overlap ranges in B") {
            /// a: *****  *****  *****
            /// b:     ---    ---    ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {7, 11}, {14, 18}};
            std::vector<ecs::entity_range> vec_b{{4, 6}, {11, 13}, {18, 20}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{4, 4} == result.at(0));
            CHECK(ecs::entity_range{11, 11} == result.at(1));
            CHECK(ecs::entity_range{18, 18} == result.at(2));
        }

        SECTION("Ranges in B overlap ranges in A") {
            /// a:     ---    ---    ---
            /// b: *****  *****  *****
            std::vector<ecs::entity_range> vec_a{{4, 6}, {11, 13}, {18, 20}};
            std::vector<ecs::entity_range> vec_b{{0, 4}, {7, 11}, {14, 18}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{4, 4} == result.at(0));
            CHECK(ecs::entity_range{11, 11} == result.at(1));
            CHECK(ecs::entity_range{18, 18} == result.at(2));
        }

        SECTION("Ranges in A overlap multiple ranges in B") {
            /// a: ********* *********
            /// b:  --- ---   --- ---
            std::vector<ecs::entity_range> vec_a{{0, 8}, {9, 17}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {5, 7}, {10, 12}, {14, 16}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(4 == result.size());
            CHECK(ecs::entity_range{1, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 7} == result.at(1));
            CHECK(ecs::entity_range{10, 12} == result.at(2));
            CHECK(ecs::entity_range{14, 16} == result.at(3));
        }

        SECTION("Ranges in B overlap multiple ranges in A") {
            /// a:  --- ---   --- ---
            /// b: ********* *********
            std::vector<ecs::entity_range> vec_a{{1, 3}, {5, 7}, {10, 12}, {14, 16}};
            std::vector<ecs::entity_range> vec_b{{0, 8}, {9, 17}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(4 == result.size());
            CHECK(ecs::entity_range{1, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 7} == result.at(1));
            CHECK(ecs::entity_range{10, 12} == result.at(2));
            CHECK(ecs::entity_range{14, 16} == result.at(3));
        }

        SECTION("One range in B overlaps two ranges in A") {
            /// a: *** ***
            /// b:  -----
            std::vector<ecs::entity_range> vec_a{{1, 3}, {5, 7}};
            std::vector<ecs::entity_range> vec_b{{2, 6}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(2 == result.size());
            CHECK(ecs::entity_range{2, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 6} == result.at(1));
        }

        SECTION("One range in A overlaps two ranges in B") {
            /// a:  -----
            /// b: *** ***
            std::vector<ecs::entity_range> vec_a{{2, 6}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {5, 7}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(2 == result.size());
            CHECK(ecs::entity_range{2, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 6} == result.at(1));
        }
    }
    SECTION("intersection difference") {
        SECTION("No overlaps between ranges") {
            /// a: *****   *****   *****
            /// b:      ---     ---     ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {8, 12}, {16, 20}};
            std::vector<ecs::entity_range> vec_b{{5, 7}, {13, 15}, {21, 23}};

            auto result = ecs::difference_ranges(vec_a, vec_b);

            REQUIRE(result == vec_a);
        }

        SECTION("Ranges in B are contained in ranges in A") {
            /// a: ***** ***** *****
            /// b:  ---   ---   ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {5, 9}, {10, 14}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {6, 8}, {11, 13}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{1, 3}.equals(result.at(0)));
            CHECK(ecs::entity_range{6, 8}.equals(result.at(1)));
            CHECK(ecs::entity_range{11, 13}.equals(result.at(2)));
        }

        SECTION("Ranges in A are contained in ranges in B") {
            /// a:  ---   ---   ---
            /// b: ***** ***** *****
            std::vector<ecs::entity_range> vec_a{{1, 3}, {6, 8}, {11, 13}};
            std::vector<ecs::entity_range> vec_b{{0, 4}, {5, 9}, {10, 14}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{1, 3}.equals(result.at(0)));
            CHECK(ecs::entity_range{6, 8}.equals(result.at(1)));
            CHECK(ecs::entity_range{11, 13}.equals(result.at(2)));
        }

        SECTION("Ranges in A overlap ranges in B") {
            /// a: *****  *****  *****
            /// b:     ---    ---    ---
            std::vector<ecs::entity_range> vec_a{{0, 4}, {7, 11}, {14, 18}};
            std::vector<ecs::entity_range> vec_b{{4, 6}, {11, 13}, {18, 20}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{4, 4} == result.at(0));
            CHECK(ecs::entity_range{11, 11} == result.at(1));
            CHECK(ecs::entity_range{18, 18} == result.at(2));
        }

        SECTION("Ranges in B overlap ranges in A") {
            /// a:     ---    ---    ---
            /// b: *****  *****  *****
            std::vector<ecs::entity_range> vec_a{{4, 6}, {11, 13}, {18, 20}};
            std::vector<ecs::entity_range> vec_b{{0, 4}, {7, 11}, {14, 18}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(3 == result.size());
            CHECK(ecs::entity_range{4, 4} == result.at(0));
            CHECK(ecs::entity_range{11, 11} == result.at(1));
            CHECK(ecs::entity_range{18, 18} == result.at(2));
        }

        SECTION("Ranges in A overlap multiple ranges in B") {
            /// a: ********* *********
            /// b:  --- ---   --- ---
            std::vector<ecs::entity_range> vec_a{{0, 8}, {9, 17}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {5, 7}, {10, 12}, {14, 16}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(4 == result.size());
            CHECK(ecs::entity_range{1, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 7} == result.at(1));
            CHECK(ecs::entity_range{10, 12} == result.at(2));
            CHECK(ecs::entity_range{14, 16} == result.at(3));
        }

        SECTION("Ranges in B overlap multiple ranges in A") {
            /// a:  --- ---   --- ---
            /// b: ********* *********
            std::vector<ecs::entity_range> vec_a{{1, 3}, {5, 7}, {10, 12}, {14, 16}};
            std::vector<ecs::entity_range> vec_b{{0, 8}, {9, 17}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(4 == result.size());
            CHECK(ecs::entity_range{1, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 7} == result.at(1));
            CHECK(ecs::entity_range{10, 12} == result.at(2));
            CHECK(ecs::entity_range{14, 16} == result.at(3));
        }

        SECTION("One range in B overlaps two ranges in A") {
            /// a: *** ***
            /// b:  -----
            std::vector<ecs::entity_range> vec_a{{1, 3}, {5, 7}};
            std::vector<ecs::entity_range> vec_b{{2, 6}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(2 == result.size());
            CHECK(ecs::entity_range{2, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 6} == result.at(1));
        }

        SECTION("One range in A overlaps two ranges in B") {
            /// a:  -----
            /// b: *** ***
            std::vector<ecs::entity_range> vec_a{{2, 6}};
            std::vector<ecs::entity_range> vec_b{{1, 3}, {5, 7}};

            auto result = ecs::intersect_ranges(vec_a, vec_b);

            REQUIRE(2 == result.size());
            CHECK(ecs::entity_range{2, 3} == result.at(0));
            CHECK(ecs::entity_range{5, 6} == result.at(1));
        }
    }

    SECTION("intersection merging") {
        auto constexpr tester = [](std::vector<ecs::entity_range> input, std::vector<ecs::entity_range> const expected) {
            auto constexpr merger = [](ecs::entity_range& a, ecs::entity_range const& b) {
                if (a.can_merge(b)) {
                    a = ecs::entity_range::merge(a, b);
                    return true;
                } else {
                    return false;
                }
            };
            combine_erase(input, merger);
            CHECK(input == expected);
        };

        // should combine to two entries {0, 3} {5, 8}
        tester({{0, 1}, {2, 3}, {5, 6}, {7, 8}}, {{0, 3}, {5, 8}});

        // should combine to single entry {0, 8}
        tester({{0, 1}, {2, 3}, {4, 6}, {7, 8}}, {{0, 8}});

        // should not combine
        tester({{0, 1}, {3, 4}, {6, 7}, {9, 10}}, {{0, 1}, {3, 4}, {6, 7}, {9, 10}});
    }
}
