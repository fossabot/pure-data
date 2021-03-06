#include "mod_env.h"
#include "env_concat.h"
#include "env_env.h"
#include "env_mix.h"
#include "env_to_array.h"
#include "env_to_vline.h"
#include "env_tscale.h"
#include "env_tshift.h"
#include "env_vscale.h"

extern "C" void setup_env0x2eadsr_tilde();
extern "C" void setup_env0x2ear_tilde();
extern "C" void setup_env0x2efollow_tilde();

void ceammc_env_setup()
{
    setup_env0x2eadsr_tilde();
    setup_env0x2ear_tilde();
    setup_env0x2efollow_tilde();
    setup_envelope();
    setup_env_to_vline();
    setup_env_to_array();
    setup_env_tscale();
    setup_env_tshift();
    setup_env_mix();
    setup_env_vscale();
    setup_env_concat();
}
