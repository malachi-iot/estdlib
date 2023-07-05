/*
 *  @file
 *
 * Portion of type traits with no dependencies (except internal/platform.h)
 * TODO: Document why this is needed so that we can structure raw vs internal properly.
 * Until we do that, leave this in a half-refactored state so that we don't cause dependency
 * issues accidentally
 */

#pragma once

#include "platform.h"
#include "feature/type_traits.h"
#include "raw/type_traits.h"
#include "../port/type_traits.h"
#include "type_traits/is_assignable.h"
#include "type_traits/constructible.h"
#include "type_traits/is_empty.h"
#include "type_traits/is_convertible.h"
#include "type_traits/make_unsigned.h"


#include "../port/support_platform.h"
