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
#include "../array/array_p2s.h"
#include "base_extension_test.h"
#include "ceammc_pd.h"

#include "catch.hpp"

typedef TestExtension<ArrayPhaseToSample> ArrayPhaseToSampleTest;

using namespace ceammc;

#define ON_FLOAT_REQUIRE(in, t, out)        \
    {                                       \
        WHEN_SEND_FLOAT_TO(0, t, in);       \
        REQUIRE_FLOAT_AT_OUTLET(0, t, out); \
    }

#define ON_LIST_REQUIRE(in, t, out)        \
    {                                      \
        WHEN_SEND_LIST_TO(0, t, in);       \
        REQUIRE_LIST_AT_OUTLET(0, t, out); \
    }

static CanvasPtr cnv = PureData::instance().createTopCanvas("test_canvas");

TEST_CASE("array.p2s", "[externals]")
{

    SECTION("test create with:")
    {
        SECTION("empty arguments")
        {
            cnv->createArray("array1", 10);

            ArrayPhaseToSampleTest t("array.p2s");
            REQUIRE(t.numInlets() == 1);
            REQUIRE(t.numOutlets() == 1);

            ON_FLOAT_REQUIRE(0, t, 0);
            ON_FLOAT_REQUIRE(0.5, t, 0);
            ON_FLOAT_REQUIRE(1, t, 0);

            ON_LIST_REQUIRE(L3(0.1f, 0.2f, 0.3f), t, L3(0.f, 0.f, 0.f));
        }

        SECTION("symbol arguments")
        {
            ArrayPhaseToSampleTest t("array.p2s", A("array1"));
            ON_FLOAT_REQUIRE(0, t, 0);
            ON_FLOAT_REQUIRE(0.5, t, 5);
            ON_FLOAT_REQUIRE(1, t, 10);

            ON_FLOAT_REQUIRE(-1, t, 0);
            ON_FLOAT_REQUIRE(2, t, 10);

            ON_LIST_REQUIRE(L3(0.1f, 0.2f, 0.3f), t, L3(1, 2, 3));
            ON_LIST_REQUIRE(L3(-1, -20, 40), t, L3(0.f, 0.f, 10));
        }
    }
}
