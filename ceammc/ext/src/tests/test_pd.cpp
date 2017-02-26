/*****************************************************************************
 * Copyright 2017 Serge Poltavsky. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "x_ceammc.h"

#include <stdio.h>

TEST_CASE("PD", "[PureData]")
{
    SECTION("hash table size")
    {
        REQUIRE(pd_ceammc_gensym_hash_table() != 0);
        REQUIRE(pd_ceammc_gensym_hash_table_size() == 1024);
        REQUIRE(pd_ceammc_gensym_hash_size() == 0);

        gensym("test");
        REQUIRE(pd_ceammc_gensym_hash_size() == 1);
        REQUIRE(pd_ceammc_gensym_hash_max_chain() == 1);

        char buf[20];
        for (int i = 0; i < 20000; i++) {
            sprintf(buf, "%d", i);
            gensym(buf);
        }
        REQUIRE(pd_ceammc_gensym_hash_size() == 20001);
        REQUIRE(pd_ceammc_gensym_hash_max_chain() == 38);
    }
}
