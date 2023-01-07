// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include "errorStatusHandler.h"

#include "opentimelineio/any.h"
#include "opentimelineio/anyVector.h"
#include "opentimelineio/safely_typed_any.h"
#include <emscripten/bind.h>

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

EMSCRIPTEN_BINDINGS(opentimelineio_anyVector)
{

    ems::class_<AnyVector>("AnyVector")
        .constructor()
        .function(
            "push_back",
            ems::optional_override([](AnyVector& a, ems::val item) {
                std::string valueType = item.typeof().as<std::string>();
                if (valueType == "string")
                {
                    a.push_back(any(item.as<std::string>()));
                }
                a.push_back(any(item));
                return true;
            }))
        .function("get", ems::optional_override([](AnyVector& a, int index) {
                      if (index < a.size())
                      {
                          return ems::val(safely_cast_string_any(a[index]));
                      }
                      return ems::val::undefined();
                  }))
        .function(
            "set",
            ems::optional_override([](AnyVector& a, int index, ems::val value) {
                a[index] = any(value);
                return true;
            }));

    // .function("resize", &AnyVector::resize)
    // .function("size", &AnyVector::size);
    // .function("get", &internal::VectorAccess<VecType>::get)
    // .function("set", &internal::VectorAccess<VecType>::set);
}
