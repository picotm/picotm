/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/picotm-libm.h"
#include <picotm/picotm-module.h>
#include "module.h"

PICOTM_EXPORT
void
picotm_libm_save_fenv()
{
    fpu_module_save_fenv();
}

PICOTM_EXPORT
void
picotm_libm_save_fexcept()
{
    fpu_module_save_fexcept();
}
