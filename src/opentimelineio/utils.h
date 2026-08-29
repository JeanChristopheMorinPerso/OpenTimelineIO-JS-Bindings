// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_UTILS_H
#define JS_UTILS_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "any/any.hpp"
#include <emscripten/val.h>
#include <opentimelineio/any.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/anyVector.h>
#include <opentimelineio/serializableObject.h>
#include <opentimelineio/vectorIndexing.h>

#include "exceptions.h"

namespace ems = emscripten;

void _build_any_to_js_dispatch_table();

ems::val any_to_js(linb::any const& a, bool top_level);

linb::any js_to_any(ems::val const& item);

template <typename T>
T js_to_cpp(ems::val const& item);

OTIO_NS::AnyVector js_array_to_cpp(ems::val const& item);

OTIO_NS::AnyDictionary js_map_to_cpp(ems::val const& item);

void install_external_keepalive_monitor(
    OTIO_NS::SerializableObject* so,
    bool                         apply_now);

template <typename T>
struct managing_ptr
{

    managing_ptr()
        : _retainer(nullptr)
    {
        printf("Created managing_ptr (nullptr)\n");
    }

    explicit managing_ptr(T* ptr)
        : _retainer(ptr)
    {
        printf("Created managing_ptr (%s)\n", typeid(*_retainer.value).name());
        install_external_keepalive_monitor(ptr, false);
    }

    // Move constructor. Embind uses this when it puts a returned managing_ptr
    // into its heap-allocated wire storage. Transfer the Retainer's pointer and
    // clear the source instead of copying it: a copy increments OTIO's
    // intrusive reference count and can trigger the keepalive monitor.
    managing_ptr(managing_ptr&& other) noexcept
        : _retainer(nullptr)
    {
        _retainer.value       = other._retainer.value;
        other._retainer.value = nullptr;
    }

    // Move assignment operator. Release the object currently held by this
    // managing_ptr, then transfer the Retainer's pointer from other. As in the
    // move constructor, this avoids temporarily incrementing OTIO's intrusive
    // reference count and triggering the keepalive monitor.
    managing_ptr& operator=(managing_ptr&& other) noexcept
    {
        if (this != &other)
        {
            managing_ptr old(std::move(*this));
            _retainer.value       = other._retainer.value;
            other._retainer.value = nullptr;
        }
        return *this;
    }

    T* get() const { return _retainer.value; }

    OTIO_NS::SerializableObject::Retainer<T> _retainer;
};

namespace emscripten { namespace internal {

// Emscripten 6's default value binding copies returned class values into wire
// storage. Preserve managing_ptr's ownership-transfer semantics instead.
template <typename T>
struct BindingType<managing_ptr<T>> : GenericBindingType<managing_ptr<T>>
{
    using Base     = GenericBindingType<managing_ptr<T>>;
    using WireType = typename Base::WireType;
    using Base::toWireType;

    static WireType toWireType(managing_ptr<T>&& value, rvp::default_tag)
    {
        return new managing_ptr<T>(std::move(value));
    }
};

}} // namespace emscripten::internal

template <typename V, typename VALUE_TYPE = typename V::value_type>
struct JSMutableSequence : public V
{
    VALUE_TYPE
    at(int index)
    {
        V& v = static_cast<V&>(*this);
        // adjusted_vector_index allows to support nagative values.
        index = OTIO_NS::adjusted_vector_index(index, v);
        if (index < 0 || index >= int(v.size()))
        {
            throw IndexError("asd");
        }

        return v[index];
    }

    void set_item(int index, VALUE_TYPE value)
    {
        V& v = static_cast<V&>(*this);
        // adjusted_vector_index allows to support nagative values.
        index = OTIO_NS::adjusted_vector_index(index, v);
        if (index < 0 || index >= int(v.size()))
        {
            throw IndexError("asd");
        }
        v[index] = value;
    }

    void insert(int index, VALUE_TYPE value)
    {
        V& v = static_cast<V&>(*this);
        // adjusted_vector_index allows to support nagative values.
        index = OTIO_NS::adjusted_vector_index(index, v);
        if (size_t(index) >= v.size())
        {
            v.emplace_back(std::move(value));
        }
        else
        {
            v.insert(v.begin() + std::max(index, 0), std::move(value));
        }
    }

    void push(VALUE_TYPE value)
    {
        V& v = static_cast<V&>(*this);
        v.emplace_back(std::move(value));
    }

    void del_item(int index)
    {
        V& v = static_cast<V&>(*this);
        if (v.empty())
        {
            throw IndexError("asd");
        }

        // adjusted_vector_index allows to support nagative values.
        index = OTIO_NS::adjusted_vector_index(index, v);

        if (size_t(index) >= v.size())
        {
            v.pop_back();
        }
        else
        {
            v.erase(v.begin() + std::max(index, 0));
        }
    }

    int length() const { return static_cast<int>(this->size()); }

    static void define_js_class(const char* name)
    {
        typedef JSMutableSequence This;

        ems::class_<This>(name)
            .template constructor<>()
            .property("length", &This::length)
            .function("size", &This::length)
            .function("at", &This::at, ems::allow_raw_pointers())
            .function("push", &This::push, ems::allow_raw_pointers())
            // TODO: Support concat
            // TODO: Support entries
            // TODO: Support includes
            // TODO: Support indexOf
            // TODO: Support slice
            // TODO: Support splice
            // TODO: Support values
            .template iterable<ems::val>("size", "at");
    }
};

/**
 * Macro to register the destructor of TYPE. Emscripten needs to have public
 * destructors which are made available from JS, but OTIO's destructors are private.
 * @param TYPE Type name without namespace.
*/
#define REGISTER_DESTRUCTOR(TYPE)                                              \
    namespace emscripten { namespace internal {                                \
    template <>                                                                \
    void raw_destructor<TYPE>(TYPE * ptr)                                      \
    {                                                                          \
        ptr->possibly_delete();                                                \
    }                                                                          \
    }                                                                          \
    } // namespace emscripten::internal

/**
 * Macro that wraps EMSCRIPTEN_WRAPPER to register a wrapper for subclasses.
 * @param TYPE Type name without a namespace.
 * @param NAME Name to use for the wrapper.
*/
#define REGISTER_WRAPPER(TYPE, NAME)                                           \
    struct NAME : public ems::wrapper<TYPE>                                    \
    {                                                                          \
        EMSCRIPTEN_WRAPPER(NAME);                                              \
    };

#endif // JS_UTILS_H
