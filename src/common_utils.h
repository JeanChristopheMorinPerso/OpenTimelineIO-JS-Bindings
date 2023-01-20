// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_COMMON_UTILS_H
#define JS_COMMON_UTILS_H

#include <emscripten.h>

/**
 * Adds the toStringTag property so that objects are rendered as [object TYPE].
 * @param TYPE Type of the object.
*/
#define ADD_TO_STRING_TAG_PROPERTY(TYPE)                                       \
    EM_ASM(                                                                    \
        Object.defineProperty(Module[#TYPE].prototype, Symbol.toStringTag, {   \
            value : #TYPE,                                                     \
            writable : false,                                                  \
            configurable : false,                                              \
            enumerable : false                                                 \
        }););

#endif // JS_COMMON_UTILS_H
