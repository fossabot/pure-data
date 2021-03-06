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
#include "../ui/ui_toggle.h"
#include "ui_external_test.h"

UI_COMPLETE_TEST_SETUP(Toggle)

TEST_CASE("ui.toggle", "[ui.toggle]")
{
    ui_test_init();

    SECTION("construct")
    {
        TestToggle t("ui.toggle");
        REQUIRE(t->numOutlets() == 1);
        REQUIRE(t->width() == 16);
        REQUIRE(t->height() == 16);
        REQUIRE(t->value() == 0);
        HAS_PROPERTY(t, "value");

        float f;
        t->getProperty(gensym("value"), f);
        REQUIRE(f == 0);

        t->onBang();
        t->getProperty(gensym("value"), f);
        REQUIRE(f == 1);

        t->onBang();
        t->getProperty(gensym("value"), f);
        REQUIRE(f == 0);
    }

    SECTION("process")
    {
        TestToggle t("ui.toggle");
        REQUIRE(t->value() == 0);

        t->onBang();
        REQUIRE(t->value() == 1);
        t->onBang();
        REQUIRE(t->value() == 0);

        t->onFloat(1);
        REQUIRE(t->value() == 1);
        t->onFloat(0.1);
        REQUIRE(t->value() == 1);
        t->onFloat(0.001);
        REQUIRE(t->value() == 1);
        t->onFloat(1000);
        REQUIRE(t->value() == 1);
        t->onFloat(-1000);
        REQUIRE(t->value() == 1);
        t->onFloat(0);
        REQUIRE(t->value() == 0);

        t->m_set(2);
        REQUIRE(t->value() == 1);
        t->m_set(0);
        REQUIRE(t->value() == 0);

        t->onMouseDown(0, t_pt(), 0);
        REQUIRE(t->value() == 1);
        t->onMouseDown(0, t_pt(), 0);
        REQUIRE(t->value() == 0);

        t->flip();
        REQUIRE(t->value() == 1);
        t->flip();
        REQUIRE(t->value() == 0);
    }

    SECTION("output")
    {
#define REQUIRE_OUTPUT_VALUE(v)                         \
    {                                                   \
        tgl.sendMessage(gensym("@value?"));             \
        REQUIRE(out.msg().listValue() == L1(float(v))); \
        out.reset();                                    \
    }

        pd::External tgl("ui.toggle");
        ExternalOutput out;
        REQUIRE(tgl.connectTo(0, out.object(), 0));

        tgl.bang();
        REQUIRE_OUTPUT_VALUE(1);

        tgl.bang();
        REQUIRE_OUTPUT_VALUE(0);

        tgl.sendFloat(1);
        REQUIRE_OUTPUT_VALUE(1);

        tgl.sendFloat(1);
        REQUIRE_OUTPUT_VALUE(1);

        tgl.sendFloat(0);
        REQUIRE_OUTPUT_VALUE(0);

        tgl.sendMessage(gensym("@value"), L1(1));
        REQUIRE_OUTPUT_VALUE(1);

        tgl.sendMessage(gensym("@value"), L1(0.f));
        REQUIRE_OUTPUT_VALUE(0);

        tgl.sendMessage(gensym("@size?"));
        REQUIRE(out.msg().listValue() == L2(16, 16));

        out.reset();
        tgl.sendMessage(gensym("set"), L1(1));
        REQUIRE(out.msg().isNone());
        tgl.bang();
        REQUIRE_OUTPUT_VALUE(0);
    }

    SECTION("presets")
    {
        TestToggle t("ui.toggle");
        t->setValue(1);
        t.call("store", L1(0.f));
        t->setValue(0);
        t.call("store", L1(1));
        t->setValue(1);
        t.call("store", L1(2));

        t.call("load", L1(0.f));
        REQUIRE(t->value() == 1);
        t.call("load", L1(1));
        REQUIRE(t->value() == 0);
        t.call("load", L1(2));
        REQUIRE(t->value() == 1);
    }

    SECTION("send test")
    {
        TestExtToggle t("ui.toggle", L2("@send", "r1"));
        t.addListener("r1");

        t << BANG;
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 1);
        t << BANG;
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 0);

        t << 20;
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 1);
        t << 20;
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 1);
        t << 0.f;
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 0);

        t.mouseDown(5, 5);
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 1);

        t.mouseDown(5, 5);
        REQUIRE_FLOAT_WAS_SEND(t, "r1", 0);
    }
}
