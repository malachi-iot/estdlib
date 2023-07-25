        SECTION("aligned_storage_array")
        {
            SECTION("simple integers")
            {
                experimental::aligned_storage_array<int, 10> a;
                int* a2 = a;

                int& value = a[0];

                value = 5;

                REQUIRE(a[0] == 5);
                REQUIRE(*a2 == 5);

                int counter = 0;

                for(auto& i : a)
                    i = counter++;

                REQUIRE(a[0] == 0);
                REQUIRE(a2[9] == 9);
            }
        }

