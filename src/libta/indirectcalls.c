/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <stdio.h>
#include "tanger-stm-internal.h"

/* Number of targets */
uint32_t tanger_stm_indirect_nb_targets_max;
uint32_t tanger_stm_indirect_nb_targets;
/* Array with function pointer pairs: nontxnal, txnal, nontxnal, ... */
void** tanger_stm_indirect_target_pairs;

/**
 * Returns the transactional version of the function passed as argument.
 * If no transactional version has been registered, it aborts.
 */
void* tanger_stm_indirect_resolve(void *nontxnal_function)
{
	uint32_t i;
    for (i = 0; i < (tanger_stm_indirect_nb_targets << 1); i+=2) {
		if (tanger_stm_indirect_target_pairs[i] == nontxnal_function)
			return tanger_stm_indirect_target_pairs[i + 1];
	}
	fprintf(stderr, "Tanger STM support: cannot find txnal version for function %p\n", nontxnal_function);
	abort();
}

/**
 * Called before transactional versions are registered for nontransactional
 * functions.
 * The parameter returns the exact number of functions that will be registered.
 */
void tanger_stm_indirect_init(uint32_t number_of_call_targets)
{
    /*fprintf(stderr, "Tanger STM support: indirect call targets = %u\n", number_of_call_targets);*/
	tanger_stm_indirect_nb_targets_max = number_of_call_targets;
	tanger_stm_indirect_nb_targets = 0;
	tanger_stm_indirect_target_pairs = calloc(number_of_call_targets * 2, sizeof(void*));
}

/**
 * Registers a transactional versions for a nontransactional function.
 */
void tanger_stm_indirect_register(void* nontxnal, void* txnal)
{
    /*fprintf(stderr, "Tanger STM support: function part nontxnal/txnal = %p/%p\n", nontxnal, txnal);*/
	tanger_stm_indirect_target_pairs[tanger_stm_indirect_nb_targets * 2] = nontxnal;
	tanger_stm_indirect_target_pairs[tanger_stm_indirect_nb_targets * 2 + 1] = txnal;
	tanger_stm_indirect_nb_targets += 1;
}

static void
tanger_stm_indirect_uninit(void) __attribute__ ((destructor));

static void
tanger_stm_indirect_uninit()
{
    free(tanger_stm_indirect_target_pairs);
}

