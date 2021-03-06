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
#include "../flow/flow_pass.h"
#include "base_extension_test.h"
#include "catch.hpp"
#include "ceammc_pd.h"

#include <stdio.h>

typedef TestExtension<FlowPass> FlowPassTest;

static CanvasPtr cnv = PureData::instance().createTopCanvas("test_canvas");

TEST_CASE("flow.pass", "[externals]")
{
    SECTION("init")
    {
        SECTION("empty")
        {
            FlowPassTest t("flow.pass");
            REQUIRE(t.numInlets() == 1);
            REQUIRE(t.numOutlets() == 1);

            REQUIRE_PROPERTY_LIST(t, @values, AtomList());
        }

        SECTION("args")
        {
            FlowPassTest t("flow.pass", L4(1, 2, "b", "@c"));
            REQUIRE_PROPERTY_LIST(t, @values, L4(1, 2, "b", "@c"));
        }

        SECTION("properties")
        {
            FlowPassTest t("flow.pass", L4(1, 2, "@values", "@c"));
            REQUIRE_PROPERTY_LIST(t, @values, L4(1, 2, "@values", "@c"));
        }
    }

    SECTION("float")
    {
        FlowPassTest t("flow.pass", L3(1, "a", "@c"));
        WHEN_SEND_BANG_TO(0, t);
        REQUIRE_BANG_AT_OUTLET(0, t);
    }

    SECTION("float")
    {
        FlowPassTest t("flow.pass", L3(1, 0.f, -1));

        WHEN_SEND_FLOAT_TO(0, t, 1.1);
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_FLOAT_TO(0, t, -1.1);
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_FLOAT_TO(0, t, 0.1f);
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_FLOAT_TO(0, t, 1);
        REQUIRE_FLOAT_AT_OUTLET(0, t, 1);

        WHEN_SEND_FLOAT_TO(0, t, -1);
        REQUIRE_FLOAT_AT_OUTLET(0, t, -1);

        WHEN_SEND_FLOAT_TO(0, t, 0.f);
        REQUIRE_FLOAT_AT_OUTLET(0, t, 0.f);
    }

    SECTION("symbol")
    {
        FlowPassTest t("flow.pass", L2("a", "b"));

        WHEN_SEND_SYMBOL_TO(0, t, "c");
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_SYMBOL_TO(0, t, "d");
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_SYMBOL_TO(0, t, "a");
        REQUIRE_SYMBOL_AT_OUTLET(0, t, "a");

        WHEN_SEND_SYMBOL_TO(0, t, "b");
        REQUIRE_SYMBOL_AT_OUTLET(0, t, "b");
    }

    SECTION("list")
    {
        // all lists are passed
        FlowPassTest t("flow.pass", L2("a", "b"));

        WHEN_SEND_LIST_TO(0, t, L2("a", "b"));
        REQUIRE_LIST_AT_OUTLET(0, t, L2("a", "b"));

        WHEN_SEND_LIST_TO(0, t, L2("c", "d"));
        REQUIRE_LIST_AT_OUTLET(0, t, L2("c", "d"));
    }

    SECTION("any")
    {
        FlowPassTest t("flow.pass", L2("c", "@d"));

        WHEN_SEND_ANY_TO(t, "a", L2(1, 2));
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_ANY_TO(t, "a", AtomList());
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_ANY_TO(t, "b", L2(1, 2));
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_ANY_TO(t, "b", AtomList());
        REQUIRE_NO_MESSAGES_AT_OUTLET(0, t);

        WHEN_SEND_ANY_TO(t, "c", L2(1, 2));
        REQUIRE_ANY_AT_OUTLET(0, t, L3("c", 1, 2));

        WHEN_SEND_ANY_TO(t, "c", AtomList());
        REQUIRE_ANY_AT_OUTLET(0, t, L1("c"));

        WHEN_SEND_ANY_TO(t, "@d", AtomList());
        REQUIRE_ANY_AT_OUTLET(0, t, L1("@d"));
    }

    SECTION("real")
    {
        setup_flow0x2epass();

        pd::External flow_pass("flow.pass", L3(1, "c", "@prop"));
        REQUIRE_FALSE(flow_pass.isNull());
    }
}
