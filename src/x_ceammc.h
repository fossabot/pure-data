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

#ifndef PD_X_CEAMMCH_
#define PD_X_CEAMMCH_

#include <stdlib.h>

#include "m_pd.h"

#if defined(__cplusplus)
extern "C" {
#endif

size_t pd_ceammc_gensym_hash_size();
size_t pd_ceammc_gensym_hash_max_chain();
t_symbol** pd_ceammc_gensym_hash_table();
size_t pd_ceammc_gensym_hash_table_size();

#if defined(__cplusplus)
}
#endif

#endif // PD_X_CEAMMCH_
