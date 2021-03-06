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
#include "../env/env_tshift.h"
#include "base_extension_test.h"
#include "catch.hpp"
#include "ceammc_pd.h"

#include <stdio.h>

typedef TestExtension<EnvTimeShift> EnvTimeShiftTest;

static CanvasPtr cnv = PureData::instance().createTopCanvas("test_canvas");

#define REQUIRE_ENV_OUTPUT(t, env)                                   \
    {                                                                \
        REQUIRE_NEW_DATA_AT_OUTLET(0, t);                            \
        const DataTypeEnv* env0 = t.typedLastDataAt<DataTypeEnv>(0); \
        REQUIRE(env0 != 0);                                          \
        REQUIRE(*env0 == env);                                       \
        t.cleanAllMessages();                                        \
    }

TEST_CASE("env.tshift", "[externals]")
{
    SECTION("init")
    {
        setup_env_tshift();

        SECTION("empty")
        {
            EnvTimeShiftTest t("env.tshift");
            REQUIRE(t.numInlets() == 1);
            REQUIRE(t.numOutlets() == 1);
            REQUIRE_PROPERTY(t, @shift, 0.f);
        }

        SECTION("float")
        {
            EnvTimeShiftTest t("env.tshift", L1(2));
            REQUIRE_PROPERTY(t, @shift, 2);
        }

        SECTION("@prop")
        {
            EnvTimeShiftTest t("env.tshift", L2("@shift", 3));
            REQUIRE_PROPERTY(t, @shift, 3);
        }

        SECTION("invalid")
        {
            EnvTimeShiftTest t("env.tshift", L2("@shift", -3));
            REQUIRE_PROPERTY(t, @shift, -3);
        }
    }

    SECTION("on data")
    {
        EnvTimeShiftTest t("env", L2("@shift", 250));
        WHEN_SEND_TDATA_TO(0, t, DataTypeEnv());
        REQUIRE_ENV_OUTPUT(t, DataTypeEnv());

        DataTypeEnv env;
        env.setAR(1000 * 1000, 2000 * 1000);

        {
            WHEN_SEND_TDATA_TO(0, t, env);

            DataTypeEnv out;
            out.insertPoint(EnvelopePoint(250 * 1000, 0));
            out.insertPoint(EnvelopePoint(1250 * 1000, 1));
            out.insertPoint(EnvelopePoint(3250 * 1000, 0));
            REQUIRE_ENV_OUTPUT(t, out);
        }
    }
}
