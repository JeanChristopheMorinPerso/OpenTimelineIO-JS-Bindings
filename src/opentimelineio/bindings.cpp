// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "emscripten.h"
#include <ImathBox.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/clip.h>
#include <opentimelineio/composable.h>
#include <opentimelineio/composition.h>
#include <opentimelineio/deserialization.h>
#include <opentimelineio/effect.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/freezeFrame.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/generatorReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/item.h>
#include <opentimelineio/linearTimeWarp.h>
#include <opentimelineio/marker.h>
#include <opentimelineio/mediaReference.h>
#include <opentimelineio/missingReference.h>
#include <opentimelineio/serializableCollection.h>
#include <opentimelineio/serializableObject.h>
#include <opentimelineio/serialization.h>
#include <opentimelineio/stack.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeEffect.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>
#include <opentimelineio/transition.h>
#include <opentimelineio/typeRegistry.h>
#include <opentimelineio/unknownSchema.h>

#include "common_utils.h"
#include "errorStatusHandler.h"
#include "js_anyDictionary.h"
#include "js_optional.h"
#include "utils.h"

namespace ems = emscripten;

REGISTER_DESTRUCTOR(OTIO_NS::SerializableObject);
REGISTER_DESTRUCTOR(OTIO_NS::UnknownSchema);
REGISTER_DESTRUCTOR(OTIO_NS::SerializableObjectWithMetadata);
REGISTER_DESTRUCTOR(OTIO_NS::Marker);
REGISTER_DESTRUCTOR(OTIO_NS::SerializableCollection);
REGISTER_DESTRUCTOR(OTIO_NS::Composable);
REGISTER_DESTRUCTOR(OTIO_NS::Item);
REGISTER_DESTRUCTOR(OTIO_NS::Transition);
REGISTER_DESTRUCTOR(OTIO_NS::Gap);
REGISTER_DESTRUCTOR(OTIO_NS::Clip);
REGISTER_DESTRUCTOR(OTIO_NS::Composition);
REGISTER_DESTRUCTOR(OTIO_NS::Track);
REGISTER_DESTRUCTOR(OTIO_NS::Stack);
REGISTER_DESTRUCTOR(OTIO_NS::Timeline);
REGISTER_DESTRUCTOR(OTIO_NS::Effect);
REGISTER_DESTRUCTOR(OTIO_NS::TimeEffect);
REGISTER_DESTRUCTOR(OTIO_NS::LinearTimeWarp);
REGISTER_DESTRUCTOR(OTIO_NS::FreezeFrame);
REGISTER_DESTRUCTOR(OTIO_NS::MediaReference);
REGISTER_DESTRUCTOR(OTIO_NS::GeneratorReference);
REGISTER_DESTRUCTOR(OTIO_NS::MissingReference);
REGISTER_DESTRUCTOR(OTIO_NS::ExternalReference);
REGISTER_DESTRUCTOR(OTIO_NS::ImageSequenceReference);

// TODO: This is dirty but I don't know of any way to bind Clip::default_media_key.
// Maybe we would need to custom cast const char* something something?
// For inspiration: https://sourcegraph.com/search?q=context:global+lang:C%2B%2B+content:%5C.class_property%5C%28%5C%22.%2B%5C%29&patternType=regexp&case=yes&sm=1&groupBy=repo
// https://sourcegraph.com/github.com/rive-app/rive-wasm@97fb62582fe69bdae8ef16ac39f23fce620dd010/-/blob/wasm/src/bindings.cpp?L52:16-52:39
static const std::string DEFAULT_MEDIA_KEY = "DEFAULT_MEDIA";

// clang-format off
namespace {

    // template<typename T>
    // std::vector<T*> vector_or_default(optional<std::vector<T*>> item) {
    //     if (item.has_value()) {
    //         return item.value();
    //     }

    //     return std::vector<T*>();
    // }

    template<typename T, typename U>
    bool find_children(T* t, ems::val descended_from_type, std::optional<OTIO_NS::TimeRange> const& search_range, bool shallow_search, std::vector<OTIO_NS::SerializableObject*>& l) {
        // TODO: Bad!
        if (true)
        {
            for (const auto& child : t->template find_children<U>(ErrorStatusHandler(), search_range, shallow_search)) {
                l.push_back(child.value);
            }
            return true;
        }
        return false;
    }

    template<typename T>
    std::vector<OTIO_NS::SerializableObject*> find_children(T* t, ems::val descended_from_type, std::optional<OTIO_NS::TimeRange> const& search_range, bool shallow_search = false) {
        std::vector<OTIO_NS::SerializableObject*> l;
        if (find_children<T, OTIO_NS::Clip>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Composition>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Gap>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Item>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Stack>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Timeline>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Track>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, OTIO_NS::Transition>(t, descended_from_type, search_range, shallow_search, l)) ;
        else
        {
            for (const auto& child : t->template find_children<OTIO_NS::Composable>(ErrorStatusHandler(), search_range, shallow_search)) {
                l.push_back(child.value);
            }
        }
        return l;
    }

    template<typename T>
    std::vector<OTIO_NS::SerializableObject*> find_clips(T* t, std::optional<OTIO_NS::TimeRange> const& search_range, bool shallow_search = false) {
        std::vector<OTIO_NS::SerializableObject*> l;
        for (const auto& clip : t->find_clips(ErrorStatusHandler(), search_range, shallow_search)) {
            l.push_back(clip.value);
        }
        return l;
    }
} // namespace
// clang-format on

REGISTER_WRAPPER(OTIO_NS::SerializableObject, SerializableObjectWrapper);
REGISTER_WRAPPER(
    OTIO_NS::SerializableObjectWithMetadata,
    SerializableObjectWithMetadataWrapper);

namespace emscripten {
template <typename T>
struct smart_ptr_trait<managing_ptr<T>>
{
    typedef managing_ptr<T> pointer_type;
    typedef T               element_type;

    static sharing_policy get_sharing_policy()
    {
        // TODO: Is this the right policy? This is undocumented so...
        return sharing_policy::INTRUSIVE;
    }

    static T* get(const managing_ptr<T>& p) { return p.get(); }

    static managing_ptr<T> share(const managing_ptr<T>& r, T* ptr)
    {
        return managing_ptr<T>(ptr);
    }

    static pointer_type* construct_null() { return new pointer_type; }
};
} // namespace emscripten

template <class T, typename... Targs>
managing_ptr<T>
make_managing_ptr(Targs&&... args)
{
    return managing_ptr<T>(new T(std::forward<Targs>(args)...));
}

template <typename CONTAINER>
class ContainerIterator
{
public:
    ContainerIterator(CONTAINER* container)
        : _container(container)
        , _it(0)
    {}

    ContainerIterator* iter() { return this; }

    ems::val next()
    {
        printf("ContainerIterator::next start\n");
        ems::val result = ems::val::object();
        if (_it == _container->children().size())
        {
            result.set("done", true);
            printf("ContainerIterator::next done\n");
            return result;
        }

        result.set("value", _container->children()[_it++].value);
        printf("ContainerIterator::next returning value\n");
        return result;
    }

private:
    CONTAINER* _container;
    size_t     _it;
};

EMSCRIPTEN_BINDINGS(opentimelineio)
{
    ems::class_<OTIO_NS::SerializableObject>("SerializableObject")
        .smart_ptr_constructor(
            "SerializableObject",
            &make_managing_ptr<OTIO_NS::SerializableObject>)
        .allow_subclass<
            SerializableObjectWrapper,
            managing_ptr<SerializableObjectWrapper>>(
            "SerializableObjectWrapper",
            "SerializableObjectWrapperPtr")
        .function(
            "_get_dynamic_fields",
            &OTIO_NS::SerializableObject::dynamic_fields)
        .function(
            "_set_dynamic_fields",
            ems::optional_override(
                [](OTIO_NS::SerializableObject&  so,
                   OTIO_NS::AnyDictionary const& dynamic_fields) {
                    OTIO_NS::AnyDictionary& old_fields = so.dynamic_fields();
                    old_fields                         = dynamic_fields;
                }))
        .function(
            "is_equivalent_to",
            &OTIO_NS::SerializableObject::is_equivalent_to)
        // Don't override Emscripten's own clone method. Emscripten's clone
        // is not a copy, it creates a references which points to the same C++ object.
        // clone is similar to when compiling OTIO with INSTANCING_SUPPORT I guess? Not sure.
        // TODO: This crashes. Stacktrace can be seen when cimpiling in Debug mode
        // and running the tests.
        .function(
            "clone_otio",
            ems::optional_override([](OTIO_NS::SerializableObject* so) {
                return so->clone(ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function(
            "to_json_string",
            ems::optional_override([](OTIO_NS::SerializableObject const& so) {
                return so.to_json_string(ErrorStatusHandler(), {}, 4);
            }))
        .function(
            "to_json_string",
            ems::optional_override(
                [](OTIO_NS::SerializableObject const& so, int indent) {
                    return so.to_json_string(ErrorStatusHandler(), {}, indent);
                }))
        .function(
            "to_json_file",
            ems::optional_override([](OTIO_NS::SerializableObject const& so,
                                      std::string file_name) {
                return so.to_json_file(file_name, ErrorStatusHandler(), {}, 4);
            }))
        .function(
            "to_json_file",
            ems::optional_override([](OTIO_NS::SerializableObject const& so,
                                      std::string file_name,
                                      int         indent) {
                return so
                    .to_json_file(file_name, ErrorStatusHandler(), {}, indent);
            }))
        .class_function(
            "from_json_string",
            ems::optional_override([](std::string input) {
                auto result = OTIO_NS::SerializableObject::from_json_string(
                    input,
                    ErrorStatusHandler());
                return managing_ptr<OTIO_NS::SerializableObject>(result);
            }))
        .class_function(
            "from_json_file",
            ems::optional_override([](std::string file_name) {
                auto result = OTIO_NS::SerializableObject::from_json_file(
                    file_name,
                    ErrorStatusHandler());
                return managing_ptr<OTIO_NS::SerializableObject>(result);
            }))
        .function("schema_name", &OTIO_NS::SerializableObject::schema_name)
        .function(
            "schema_version",
            &OTIO_NS::SerializableObject::schema_version)
        .property(
            "is_unknown_schema",
            &OTIO_NS::SerializableObject::is_unknown_schema);

    ADD_TO_STRING_TAG_PROPERTY(SerializableObject);

    ems::class_<OTIO_NS::UnknownSchema, ems::base<OTIO_NS::SerializableObject>>(
        "UnknownSchema")
        .constructor<std::string, int>()
        .property(
            "original_schema_name",
            &OTIO_NS::UnknownSchema::original_schema_name)
        .property(
            "original_schema_version",
            &OTIO_NS::UnknownSchema::original_schema_version);

    ADD_TO_STRING_TAG_PROPERTY(UnknownSchema);

    ems::class_<
        OTIO_NS::SerializableObjectWithMetadata,
        ems::base<OTIO_NS::SerializableObject>>(
        "SerializableObjectWithMetadata")
        // To be able to overload smpart_prt constructors, we need to call "smart_ptr"
        // and then we can simply overload using `constructor(<smart pointer here>)`.
        // This is badly documented. Found at https://github.com/emscripten-core/emscripten/issues/11170#issuecomment-634113785.
        // And if we want to actually have a custom function in the constructor,
        // it's like usual, but we need to return a "managing_ptr" instead of the usual raw pointer
        // to the object. This last bit was figured out by me, all by myself.
        .smart_ptr<managing_ptr<OTIO_NS::SerializableObjectWithMetadata>>(
            "SerializableObjectWithMetadata")
        .constructor(
            &make_managing_ptr<OTIO_NS::SerializableObjectWithMetadata>)
        .constructor(&make_managing_ptr<
                     OTIO_NS::SerializableObjectWithMetadata,
                     std::string>)
        .constructor(
            ems::optional_override([](std::string name, ems::val metadata) {
                OTIO_NS::AnyDictionary d = js_map_to_cpp(metadata);
                return managing_ptr<OTIO_NS::SerializableObjectWithMetadata>(
                    new OTIO_NS::SerializableObjectWithMetadata(name, d));
            }))
        .allow_subclass<
            SerializableObjectWithMetadataWrapper,
            managing_ptr<SerializableObjectWithMetadataWrapper>>(
            "SerializableObjectWithMetadataWrapper",
            "SerializableObjectWithMetadataWrapperPtr")
        .property(
            "name",
            &OTIO_NS::SerializableObjectWithMetadata::name,
            &OTIO_NS::SerializableObjectWithMetadata::set_name)
        .function(
            "get_metadata",
            // TODO: Should we instead return the reference? AFAIK we can't
            // to ems::select_overload<AnyDictionary&() noexcept>
            // Also, how will we override metadata? For example so.metadata?
            ems::select_overload<OTIO_NS::AnyDictionary() const noexcept>(
                &OTIO_NS::SerializableObjectWithMetadata::metadata))
        // TODO: This dirty, but so far I didn't find a nicer method.
        // Binded getters seem to be read-only (it cannot return a pointer or a reference).
        // Additionally, it seems impossible to accept a pointer or reference for the "this"
        // in setters.
        .function(
            "set_metadata",
            ems::optional_override(
                [](OTIO_NS::SerializableObjectWithMetadata& so,
                   OTIO_NS::AnyDictionary                   metadata) {
                    std::cout << "Size before swap: "
                                     + std::to_string(so.metadata().size())
                                     + "\n";
                    OTIO_NS::AnyDictionary& old_metadata = so.metadata();
                    old_metadata                         = metadata;
                    std::cout << "Size after swap: "
                                     + std::to_string(so.metadata().size())
                                     + "\n";
                }));

    ADD_TO_STRING_TAG_PROPERTY(SerializableObjectWithMetadata);

    ems::class_<
        OTIO_NS::Marker,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>("Marker")
        .smart_ptr<managing_ptr<OTIO_NS::Marker>>("Marker")
        .constructor(&make_managing_ptr<OTIO_NS::Marker>)
        .constructor(&make_managing_ptr<OTIO_NS::Marker, std::string>)
        .constructor(&make_managing_ptr<
                     OTIO_NS::Marker,
                     std::string,
                     OTIO_NS::TimeRange>)
        .constructor(&make_managing_ptr<
                     OTIO_NS::Marker,
                     std::string,
                     OTIO_NS::TimeRange,
                     std::string>)
        .constructor(
            ems::optional_override([](std::string const&        name,
                                      OTIO_NS::TimeRange const& marked_range,
                                      std::string const&        color,
                                      ems::val const&           metadata) {
                return managing_ptr<OTIO_NS::Marker>(new OTIO_NS::Marker(
                    name,
                    marked_range,
                    color,
                    js_map_to_cpp(metadata)));
            }))
        .property("color", &OTIO_NS::Marker::color, &OTIO_NS::Marker::set_color)
        .property(
            "marked_range",
            &OTIO_NS::Marker::marked_range,
            &OTIO_NS::Marker::set_marked_range);

    ADD_TO_STRING_TAG_PROPERTY(Marker);

    // TODO: Should this be an enum or maybe something else?
    ems::class_<OTIO_NS::Marker::Color>("MarkerColor");
    ADD_TO_STRING_TAG_PROPERTY(MarkerColor);

    // TODO: Use custom unmarshaling? When I tried, it wasn't even compiling.
    ems::register_vector<OTIO_NS::SerializableObject*>("SOVector");

    using SerializableCollectionIterator =
        ContainerIterator<OTIO_NS::SerializableCollection>;

    ems::class_<SerializableCollectionIterator>(
        "SerializableCollectionIterator")
        .function("next", &SerializableCollectionIterator::next);

    // TODO: Implement and continue tests.
    ems::class_<
        OTIO_NS::SerializableCollection,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>(
        "SerializableCollection")
        .smart_ptr<managing_ptr<OTIO_NS::SerializableCollection>>(
            "SerializableCollection")
        .constructor(&make_managing_ptr<OTIO_NS::SerializableCollection>)
        .constructor(
            &make_managing_ptr<OTIO_NS::SerializableCollection, std::string>)
        .constructor(
            ems::optional_override(
                [](std::string const&                        name,
                   std::vector<OTIO_NS::SerializableObject*> children) {
                    return managing_ptr<OTIO_NS::SerializableCollection>(
                        new OTIO_NS::SerializableCollection(
                            name,
                            children,
                            OTIO_NS::AnyDictionary()));
                }),
            ems::allow_raw_pointers())
        .constructor(ems::optional_override(
            [](std::string const&                        name,
               std::vector<OTIO_NS::SerializableObject*> children,
               ems::val                                  metadata) {
                return new OTIO_NS::SerializableCollection(
                    name,
                    children,
                    js_map_to_cpp(metadata));
            }))
        .property(
            "length",
            ems::optional_override(
                [](OTIO_NS::SerializableCollection const& sc) {
                    return sc.children().size();
                }))
        .function(
            "@@iterator",
            ems::optional_override([](OTIO_NS::SerializableCollection* sc) {
                return new SerializableCollectionIterator(sc);
            }),
            ems::allow_raw_pointers())
        .function(
            "get_children",
            ems::optional_override(
                [](OTIO_NS::SerializableCollection const& sc) {
                    std::vector<OTIO_NS::SerializableObject*> l;
                    for (const auto& child: sc.children())
                    {
                        l.push_back(child);
                    }
                    return l;
                }))
        .function(
            "set_children",
            &OTIO_NS::SerializableCollection::set_children,
            ems::allow_raw_pointers())
        .function(
            "clear_children",
            &OTIO_NS::SerializableCollection::clear_children)
        .function(
            "insert_child",
            &OTIO_NS::SerializableCollection::insert_child,
            ems::allow_raw_pointers())
        .function(
            "set_child",
            ems::optional_override([](OTIO_NS::SerializableCollection& sc,
                                      int                              index,
                                      OTIO_NS::SerializableObject*     child) {
                return sc.set_child(index, child, ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function(
            "remove_child",
            ems::optional_override(
                [](OTIO_NS::SerializableCollection& sc, int index) {
                    return sc.remove_child(index, ErrorStatusHandler());
                }))
        .function(
            "find_clips",
            ems::optional_override([](OTIO_NS::SerializableCollection* sc) {
                return find_clips(sc, nonstd::nullopt, false);
            }),
            ems::allow_raw_pointers())
        .function(
            "find_clips",
            ems::optional_override([](OTIO_NS::SerializableCollection* sc,
                                      OTIO_NS::TimeRange const& search_range) {
                return find_clips(sc, search_range, false);
            }),
            ems::allow_raw_pointers())
        .function(
            "find_clips",
            ems::optional_override([](OTIO_NS::SerializableCollection* sc,
                                      OTIO_NS::TimeRange const& search_range,
                                      bool shallow_search) {
                return find_clips(sc, nonstd::nullopt, shallow_search);
            }),
            ems::allow_raw_pointers())
        .function(
            "find_children",
            ems::optional_override([](OTIO_NS::SerializableCollection* sc,
                                      ems::val descended_from_type,
                                      OTIO_NS::TimeRange const& search_range,
                                      bool shallow_search) {
                return find_children(
                    sc,
                    descended_from_type,
                    search_range,
                    shallow_search);
            }),
            ems::allow_raw_pointers());

    ADD_TO_STRING_TAG_PROPERTY(SerializableCollection);

    ems::class_<
        OTIO_NS::Composable,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>("Composable")
        .smart_ptr<managing_ptr<OTIO_NS::Composable>>("Composable")
        .constructor(&make_managing_ptr<OTIO_NS::Composable>)
        .constructor(&make_managing_ptr<OTIO_NS::Composable, std::string>)
        .constructor(ems::optional_override(
            [](std ::string const& name, ems::val metadata) {
                return managing_ptr<OTIO_NS::Composable>(
                    new OTIO_NS::Composable(name, js_map_to_cpp(metadata)));
            }))
        .function(
            "parent",
            &OTIO_NS::Composable::parent,
            ems::allow_raw_pointers())
        .function("function", &OTIO_NS::Composable::visible)
        .function("overlapping", &OTIO_NS::Composable::overlapping);

    ADD_TO_STRING_TAG_PROPERTY(Composable);

    // TODO: This is not nice on the JS side.
    ems::register_vector<OTIO_NS::Effect*>("EffectVector");
    ems::register_vector<OTIO_NS::Marker*>("MarkerVector");

    using EffectVectorProxy = JSMutableSequence<
        std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Effect>>,
        OTIO_NS::Effect*>;
    using MarkerVectorProxy = JSMutableSequence<
        std::vector<OTIO_NS::SerializableObject::Retainer<OTIO_NS::Marker>>,
        OTIO_NS::Marker*>;

    EffectVectorProxy::define_js_class("EffectVectorProxy");
    MarkerVectorProxy::define_js_class("MarkerVectorProxy");

    ems::class_<OTIO_NS::Item, ems::base<OTIO_NS::Composable>>("Item")
        .smart_ptr<managing_ptr<OTIO_NS::Item>>("Item")
        .constructor(&make_managing_ptr<OTIO_NS::Item>)
        .constructor(&make_managing_ptr<OTIO_NS::Item, std::string>)
        .constructor(&make_managing_ptr<
                     OTIO_NS::Item,
                     std::string,
                     std::optional<OTIO_NS::TimeRange>>)
        .constructor(ems::optional_override(
            [](std ::string const&                      name,
               std::optional<OTIO_NS::TimeRange> const& source_range,
               std::vector<OTIO_NS::Effect*> const&     effects) {
                return managing_ptr<OTIO_NS::Item>(new OTIO_NS::Item(
                    name,
                    source_range,
                    OTIO_NS::AnyDictionary(),
                    effects,
                    std::vector<OTIO_NS::Marker*>(),
                    true));
            }))
        .constructor(ems::optional_override(
            [](std ::string const&                      name,
               std::optional<OTIO_NS::TimeRange> const& source_range,
               std::vector<OTIO_NS::Effect*> const&     effects,
               std::vector<OTIO_NS::Marker*> const&     markers) {
                return managing_ptr<OTIO_NS::Item>(new OTIO_NS::Item(
                    name,
                    source_range,
                    OTIO_NS::AnyDictionary(),
                    effects,
                    markers,
                    true));
            }))
        .constructor(ems::optional_override(
            [](std ::string const&                      name,
               std::optional<OTIO_NS::TimeRange> const& source_range,
               std::vector<OTIO_NS::Effect*> const&     effects,
               std::vector<OTIO_NS::Marker*> const&     markers,
               bool                                     enabled) {
                return managing_ptr<OTIO_NS::Item>(new OTIO_NS::Item(
                    name,
                    source_range,
                    OTIO_NS::AnyDictionary(),
                    effects,
                    markers,
                    enabled));
            }))
        .constructor(ems::optional_override(
            [](std ::string const&                      name,
               std::optional<OTIO_NS::TimeRange> const& source_range,
               std::vector<OTIO_NS::Effect*> const&     effects,
               std::vector<OTIO_NS::Marker*> const&     markers,
               bool                                     enabled,
               ems::val const&                          metadata) {
                return managing_ptr<OTIO_NS::Item>(new OTIO_NS::Item(
                    name,
                    source_range,
                    js_map_to_cpp(metadata),
                    effects,
                    markers,
                    enabled));
            }))
        .property(
            "enabled",
            &OTIO_NS::Item::enabled,
            &OTIO_NS::Item::set_enabled)
        .property(
            "source_range",
            &OTIO_NS::Item::source_range,
            &OTIO_NS::Item::set_source_range)
        .function(
            "get_effects",
            ems::optional_override([](OTIO_NS::Item const& item) {
                return ((EffectVectorProxy*) &item.effects());
            }),
            ems::allow_raw_pointers())
        .function(
            "get_markers",
            ems::optional_override([](OTIO_NS::Item const& item) {
                return ((MarkerVectorProxy*) &item.markers());
            }),
            ems::allow_raw_pointers())
        .function(
            "trimmed_range",
            ems::optional_override([](OTIO_NS::Item const& item) {
                return item.trimmed_range(ErrorStatusHandler());
            }));
    ADD_TO_STRING_TAG_PROPERTY(Item);

    ems::class_<OTIO_NS::Transition, ems::base<OTIO_NS::Composable>>(
        "Transition")
        .constructor(
            ems::optional_override([](std::string const&    name,
                                      std::string const&    transition_type,
                                      OTIO_NS::RationalTime in_offset,
                                      OTIO_NS::RationalTime out_offset,
                                      ems::val              metadata) {
                return new OTIO_NS::Transition(
                    name,
                    transition_type,
                    in_offset,
                    out_offset,
                    js_map_to_cpp(metadata));
            }))
        .property(
            "transition_type",
            &OTIO_NS::Transition::transition_type,
            &OTIO_NS::Transition::set_transition_type)
        .property(
            "in_offset",
            &OTIO_NS::Transition::in_offset,
            &OTIO_NS::Transition::set_in_offset)
        .property(
            "out_offset",
            &OTIO_NS::Transition::out_offset,
            &OTIO_NS::Transition::set_out_offset)
        .function(
            "duration",
            ems::optional_override([](OTIO_NS::Transition& t) {
                return t.duration(ErrorStatusHandler());
            }))
        .function(
            "range_in_parent",
            ems::optional_override([](OTIO_NS::Transition& t) {
                return t.range_in_parent(ErrorStatusHandler());
            }))
        .function(
            "trimmed_range_in_parent",
            ems::optional_override([](OTIO_NS::Transition& t) {
                return t.trimmed_range_in_parent(ErrorStatusHandler());
            }));

    ADD_TO_STRING_TAG_PROPERTY(Transition);

    // TODO: Implement
    ems::class_<OTIO_NS::Transition::Type>("TransitionType");
    ADD_TO_STRING_TAG_PROPERTY(TransitionType);

    // TODO: Implement
    ems::class_<OTIO_NS::Gap, ems::base<OTIO_NS::Item>>("Gap");
    ADD_TO_STRING_TAG_PROPERTY(Gap);

    // TODO: Test MediaReference as input
    ems::class_<OTIO_NS::Clip, ems::base<OTIO_NS::Item>>("Clip")
        .smart_ptr<managing_ptr<OTIO_NS::Clip>>("Clip")
        .constructor(&make_managing_ptr<OTIO_NS::Clip>)
        .constructor(&make_managing_ptr<OTIO_NS::Clip, std::string>)
        .constructor(make_managing_ptr<
                     OTIO_NS::Clip,
                     std::string,
                     OTIO_NS::MediaReference*>)
        .constructor(&make_managing_ptr<
                     OTIO_NS::Clip,
                     std::string,
                     OTIO_NS::MediaReference*,
                     std::optional<OTIO_NS::TimeRange>>)
        .constructor(
            ems::optional_override(
                [](std::string const&                name,
                   OTIO_NS::MediaReference*          media_reference,
                   std::optional<OTIO_NS::TimeRange> source_range,
                   ems::val                          metadata) {
                    return managing_ptr<OTIO_NS::Clip>(new OTIO_NS::Clip(
                        name,
                        media_reference,
                        source_range,
                        js_map_to_cpp(metadata),
                        OTIO_NS::Clip::default_media_key));
                }),
            ems::allow_raw_pointers())
        .constructor(
            ems::optional_override(
                [](std::string const&                name,
                   OTIO_NS::MediaReference*          media_reference,
                   std::optional<OTIO_NS::TimeRange> source_range,
                   ems::val                          metadata,
                   std::string const&                active_media_reference) {
                    return managing_ptr<OTIO_NS::Clip>(new OTIO_NS::Clip(
                        name,
                        media_reference,
                        source_range,
                        js_map_to_cpp(metadata),
                        active_media_reference));
                }),
            ems::allow_raw_pointers())
        .class_property("DEFAULT_MEDIA_KEY", &DEFAULT_MEDIA_KEY)

        // TODO: This should be a property, but embind doesn't support pointers in properties.
        // And on top of that we can't even use ems::optional_override to dereference the pointer
        // because that could require the copy constructor, which is implicitly deleted by SOWithMetadata.
        .function(
            "media_reference",
            &OTIO_NS::Clip::media_reference,
            ems::allow_raw_pointers())
        .function(
            "set_media_reference",
            &OTIO_NS::Clip::set_media_reference,
            ems::allow_raw_pointers())
        .property(
            "active_media_reference_key",
            &OTIO_NS::Clip::active_media_reference_key,
            ems::optional_override(
                [](OTIO_NS::Clip& clip, std::string const& new_active_key) {
                    clip.set_active_media_reference_key(
                        new_active_key,
                        ErrorStatusHandler());
                }))
        .function("media_references", &OTIO_NS::Clip::media_references)
        .function(
            "set_media_references",
            ems::optional_override(
                [](OTIO_NS::Clip*                        clip,
                   OTIO_NS::Clip::MediaReferences const& media_references,
                   std::string const&                    new_active_key) {
                    clip->set_media_references(
                        media_references,
                        new_active_key,
                        ErrorStatusHandler());
                }),
            ems::allow_raw_pointers());
    ADD_TO_STRING_TAG_PROPERTY(Clip);

    // TODO: Implement
    ems::class_<OTIO_NS::Composition, ems::base<OTIO_NS::Item>>("Composition")
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children,
               OTIO_NS::TimeRange                source_range,
               ems::val                          metadata) {
                OTIO_NS::Composition* c = new OTIO_NS::Composition(
                    name,
                    source_range,
                    js_map_to_cpp(metadata));
                c->set_children(children, ErrorStatusHandler());
                return c;
            }))
        // TODO: It works but the returned vector contains zero item.
        .function(
            "find_children",
            ems::optional_override([](OTIO_NS::Composition const& c) {
                std::vector<OTIO_NS::SerializableObject*> l;
                for (const auto& composable:
                     c.find_children(ErrorStatusHandler()))
                {
                    l.push_back(composable.value);
                }
                return l;
            }));

    ADD_TO_STRING_TAG_PROPERTY(Composition);

    ems::enum_<OTIO_NS::Track::NeighborGapPolicy>("TrackNeighborGapPolicy")
        .value(
            "around_transitions",
            OTIO_NS::Track::NeighborGapPolicy::around_transitions)
        .value("never", OTIO_NS::Track::NeighborGapPolicy::never);

    // TODO: Implement
    ems::class_<OTIO_NS::Track::Kind>("TrackKind");
    ADD_TO_STRING_TAG_PROPERTY(TrackKind);

    // TODO: Implement
    ems::class_<OTIO_NS::Track, ems::base<OTIO_NS::Composition>>("Track")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children) {
                OTIO_NS::Track* t = new OTIO_NS::Track(name);
                t->set_children(children, ErrorStatusHandler());
                return t;
            }))
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children,
               OTIO_NS::TimeRange const&         source_range) {
                OTIO_NS::Track* t = new OTIO_NS::Track(name, source_range);
                t->set_children(children, ErrorStatusHandler());
                return t;
            }))
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children,
               OTIO_NS::TimeRange const&         source_range,
               std::string const&                kind) {
                OTIO_NS::Track* t =
                    new OTIO_NS::Track(name, source_range, kind);
                t->set_children(children, ErrorStatusHandler());
                return t;
            }))
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children,
               OTIO_NS::TimeRange const&         source_range,
               std::string const&                kind,
               ems::val metadata = ems::val::object()) {
                OTIO_NS::Track* t = new OTIO_NS::Track(
                    name,
                    source_range,
                    kind,
                    js_map_to_cpp(metadata));
                t->set_children(children, ErrorStatusHandler());
                return t;
            }))
        .property("kind", &OTIO_NS::Track::kind, &OTIO_NS::Track::set_kind)
        .function(
            "neighbors_of",
            ems::optional_override(
                [](OTIO_NS::Track const&             t,
                   OTIO_NS::Composable&              item,
                   OTIO_NS::Track::NeighborGapPolicy policy) {
                    auto result =
                        t.neighbors_of(&item, ErrorStatusHandler(), policy);
                    return result;
                }));

    ADD_TO_STRING_TAG_PROPERTY(Track);

    // TODO: Implement
    ems::class_<OTIO_NS::Stack, ems::base<OTIO_NS::Composition>>("Stack")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const&                name,
               std::vector<OTIO_NS::Composable*> children,
               OTIO_NS::TimeRange const&         source_range,
               std::vector<OTIO_NS::Marker*>     markers,
               std::vector<OTIO_NS::Effect*>     effects,
               ems::val                          metadata) {
                OTIO_NS::Stack* s = new OTIO_NS::Stack(
                    name,
                    source_range,
                    js_map_to_cpp(metadata),
                    effects,
                    markers);
                s->set_children(children, ErrorStatusHandler());
                return s;
            }));

    ADD_TO_STRING_TAG_PROPERTY(Stack);

    // TODO: Implement
    ems::class_<
        OTIO_NS::Timeline,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>("Timeline");

    ADD_TO_STRING_TAG_PROPERTY(Timeline);

    ems::class_<
        OTIO_NS::Effect,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>("Effect")
        .smart_ptr<managing_ptr<OTIO_NS::Effect>>("Effect")
        .constructor(&make_managing_ptr<OTIO_NS::Effect>)
        .constructor(&make_managing_ptr<OTIO_NS::Effect, std::string>)
        .constructor(
            &make_managing_ptr<OTIO_NS::Effect, std::string, std::string>)
        .constructor(ems::optional_override([](std::string const& name,
                                               std::string const& effect_name,
                                               ems::val           metadata) {
            return managing_ptr<OTIO_NS::Effect>(new OTIO_NS::Effect(
                name,
                effect_name,
                js_map_to_cpp(metadata)));
        }))
        .property(
            "effect_name",
            &OTIO_NS::Effect::effect_name,
            &OTIO_NS::Effect::set_effect_name);

    ADD_TO_STRING_TAG_PROPERTY(Effect);

    ems::class_<OTIO_NS::TimeEffect, ems::base<OTIO_NS::Effect>>("TimeEffect")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, std::string>()
        .constructor(ems::optional_override([](std::string const& name,
                                               std::string const& effect_name,
                                               ems::val           metadata) {
            return new OTIO_NS::TimeEffect(
                name,
                effect_name,
                js_map_to_cpp(metadata));
        }));

    ADD_TO_STRING_TAG_PROPERTY(TimeEffect);

    ems::class_<OTIO_NS::LinearTimeWarp, ems::base<OTIO_NS::TimeEffect>>(
        "LinearTimeWarp")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const& name, double time_scalar) {
                return new OTIO_NS::LinearTimeWarp(
                    name,
                    "LinearTimeWarp",
                    time_scalar);
            }))
        .constructor(ems::optional_override(
            [](std::string const& name, double time_scalar, ems::val metadata) {
                return new OTIO_NS::LinearTimeWarp(
                    name,
                    "LinearTimeWarp",
                    time_scalar,
                    js_map_to_cpp(metadata));
            }))
        .property(
            "time_scalar",
            &OTIO_NS::LinearTimeWarp::time_scalar,
            &OTIO_NS::LinearTimeWarp::set_time_scalar);

    ADD_TO_STRING_TAG_PROPERTY(LinearTimeWarp);

    ems::class_<OTIO_NS::FreezeFrame, ems::base<OTIO_NS::LinearTimeWarp>>(
        "FreezeFrame")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const& name, ems::val metadata) {
                return new OTIO_NS::FreezeFrame(name, js_map_to_cpp(metadata));
            }));

    ADD_TO_STRING_TAG_PROPERTY(FreezeFrame);

    ems::class_<
        OTIO_NS::MediaReference,
        ems::base<OTIO_NS::SerializableObjectWithMetadata>>("MediaReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, OTIO_NS::TimeRange>()
        .constructor(
            ems::optional_override([](std::string const& name,
                                      OTIO_NS::TimeRange available_range,
                                      ems::val           metadata) {
                return new OTIO_NS::MediaReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata));
            }))
        .constructor(ems::optional_override(
            [](std::string const&  name,
               OTIO_NS::TimeRange  available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new OTIO_NS::MediaReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "available_range",
            &OTIO_NS::MediaReference::available_range,
            &OTIO_NS::MediaReference::set_available_range)
        .property(
            "available_image_bounds",
            &OTIO_NS::MediaReference::available_image_bounds,
            &OTIO_NS::MediaReference::set_available_image_bounds)
        .property(
            "is_missing_reference",
            &OTIO_NS::MediaReference::is_missing_reference);

    ADD_TO_STRING_TAG_PROPERTY(MediaReference);

    ems::class_<
        OTIO_NS::GeneratorReference,
        ems::base<OTIO_NS::MediaReference>>("GeneratorReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, std::string>()
        .constructor<std::string, std::string, OTIO_NS::TimeRange>()
        .constructor(
            ems::optional_override([](std::string const&        name,
                                      std::string const&        generator_kind,
                                      OTIO_NS::TimeRange const& available_range,
                                      ems::val                  parameters) {
                return new OTIO_NS::GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters));
            }))
        .constructor(
            ems::optional_override([](std::string const&        name,
                                      std::string const&        generator_kind,
                                      OTIO_NS::TimeRange const& available_range,
                                      ems::val                  parameters,
                                      ems::val                  metadata) {
                return new OTIO_NS::GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters),
                    js_map_to_cpp(metadata));
            }))
        .constructor(ems::optional_override(
            [](std::string const&        name,
               std::string const&        generator_kind,
               OTIO_NS::TimeRange const& available_range,
               ems::val                  parameters,
               ems::val                  metadata,
               Imath::Box2d const&       available_image_bounds) {
                return new OTIO_NS::GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters),
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "generator_kind",
            &OTIO_NS::GeneratorReference::generator_kind,
            &OTIO_NS::GeneratorReference::set_generator_kind)
        .property(
            "parameters",
            // TODO: Should we use const or the reference? (Using the reference results in a compilation error)
            ems::select_overload<OTIO_NS::AnyDictionary() const>(
                &OTIO_NS::GeneratorReference::parameters));

    ADD_TO_STRING_TAG_PROPERTY(GeneratorReference);

    ems::class_<OTIO_NS::MissingReference, ems::base<OTIO_NS::MediaReference>>(
        "MissingReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, OTIO_NS::TimeRange>()
        .constructor(
            ems::optional_override([](std::string const& name,
                                      OTIO_NS::TimeRange available_range,
                                      ems::val           metadata) {
                return new OTIO_NS::MissingReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata));
            }))
        .constructor(ems::optional_override(
            [](std::string const&  name,
               OTIO_NS::TimeRange  available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new OTIO_NS::MissingReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }));

    ADD_TO_STRING_TAG_PROPERTY(MissingReference);

    ems::class_<OTIO_NS::ExternalReference, ems::base<OTIO_NS::MediaReference>>(
        "ExternalReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, OTIO_NS::TimeRange>()
        .constructor(
            ems::optional_override([](std::string const& target_url,
                                      OTIO_NS::TimeRange available_range,
                                      ems::val           metadata) {
                return new OTIO_NS::ExternalReference(
                    target_url,
                    available_range,
                    js_map_to_cpp(metadata));
            }))
        .constructor(ems::optional_override(
            [](std::string const&  target_url,
               OTIO_NS::TimeRange  available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new OTIO_NS::ExternalReference(
                    target_url,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "target_url",
            &OTIO_NS::ExternalReference::target_url,
            &OTIO_NS::ExternalReference::set_target_url);

    ADD_TO_STRING_TAG_PROPERTY(ExternalReference);

    ems::enum_<OTIO_NS::ImageSequenceReference::MissingFramePolicy>(
        "MissingFramePolicy")
        .value(
            "error",
            OTIO_NS::ImageSequenceReference::MissingFramePolicy::error)
        .value(
            "hold",
            OTIO_NS::ImageSequenceReference::MissingFramePolicy::hold)
        .value(
            "black",
            OTIO_NS::ImageSequenceReference::MissingFramePolicy::black);

    ems::class_<
        OTIO_NS::ImageSequenceReference,
        ems::base<OTIO_NS::MediaReference>>("ImageSequenceReference")
        .constructor(ems::optional_override(
            [](std::string const& target_url_base,
               std::string const& name_prefix,
               std::string const& name_suffix,
               int                start_frame,
               int                frame_step,
               double const       rate,
               int                frame_zero_padding,
               OTIO_NS::ImageSequenceReference::MissingFramePolicy const
                                         missing_frame_policy,
               OTIO_NS::TimeRange const& available_range,
               ems::val                  metadata,
               Imath::Box2d const&       available_image_bounds) {
                return new OTIO_NS::ImageSequenceReference(
                    target_url_base,
                    name_prefix,
                    name_suffix,
                    start_frame,
                    frame_step,
                    rate,
                    frame_zero_padding,
                    missing_frame_policy,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "target_url_base",
            &OTIO_NS::ImageSequenceReference::target_url_base,
            &OTIO_NS::ImageSequenceReference::set_target_url_base)
        .property(
            "name_prefix",
            &OTIO_NS::ImageSequenceReference::name_prefix,
            &OTIO_NS::ImageSequenceReference::set_name_prefix)
        .property(
            "name_suffix",
            &OTIO_NS::ImageSequenceReference::name_suffix,
            &OTIO_NS::ImageSequenceReference::set_name_suffix)
        .property(
            "start_frame",
            &OTIO_NS::ImageSequenceReference::start_frame,
            &OTIO_NS::ImageSequenceReference::set_start_frame)
        .property(
            "frame_step",
            &OTIO_NS::ImageSequenceReference::frame_step,
            &OTIO_NS::ImageSequenceReference::set_frame_step)
        .property(
            "rate",
            &OTIO_NS::ImageSequenceReference::rate,
            &OTIO_NS::ImageSequenceReference::set_rate)
        .property(
            "frame_zero_padding",
            &OTIO_NS::ImageSequenceReference::frame_zero_padding,
            &OTIO_NS::ImageSequenceReference::set_frame_zero_padding)
        .property(
            "missing_frame_policy",
            &OTIO_NS::ImageSequenceReference::missing_frame_policy,
            &OTIO_NS::ImageSequenceReference::set_missing_frame_policy)
        .function("end_frame", &OTIO_NS::ImageSequenceReference::end_frame)
        .function(
            "number_of_images_in_sequence",
            &OTIO_NS::ImageSequenceReference::number_of_images_in_sequence)
        .function(
            "frame_for_time",
            ems::optional_override(
                [](OTIO_NS::ImageSequenceReference const& ref,
                   OTIO_NS::RationalTime                  time) {
                    return ref.frame_for_time(time, ErrorStatusHandler());
                }))
        .function(
            "target_url_for_image_number",
            ems::optional_override(
                [](OTIO_NS::ImageSequenceReference const& ref,
                   int                                    image_number) {
                    return ref.target_url_for_image_number(
                        image_number,
                        ErrorStatusHandler());
                }))
        .function(
            "presentation_time_for_image_number",
            ems::optional_override(
                [](OTIO_NS::ImageSequenceReference const& ref,
                   int                                    image_number) {
                    return ref.presentation_time_for_image_number(
                        image_number,
                        ErrorStatusHandler());
                }));

    ADD_TO_STRING_TAG_PROPERTY(ImageSequenceReference);

    // TODO: Test
    ems::function(
        "register_serializable_object_type",
        ems::optional_override([](ems::val           class_object,
                                  std::string const& schema_name,
                                  int                schema_version) {
            std::function<OTIO_NS::SerializableObject*()> create =
                [class_object]() {
                    ems::val js_so = class_object();
                    OTIO_NS::SerializableObject::Retainer<> r =
                        js_so.as<OTIO_NS::SerializableObject::Retainer<>>();

                    // we need to dispose of the reference to python_so now,
                    // while r exists to keep the object we just created alive.
                    // (If we let python_so be destroyed when we leave the function,
                    // then the C++ object we just created would be immediately
                    // destroyed then.)
                    // TODO: Is this comment valid?

                    js_so = ems::val::object();
                    return r.take_value();
                };

            OTIO_NS::TypeRegistry::instance().register_type(
                schema_name,
                schema_version,
                nullptr,
                create,
                schema_name);
        }));

    // TODO: Test
    ems::function(
        "set_type_record",
        ems::optional_override([](OTIO_NS::SerializableObject* so,
                                  std::string const&           schema_name) {
            OTIO_NS::TypeRegistry::instance().set_type_record(
                so,
                schema_name,
                ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    // TODO: Test
    ems::function(
        "instance_from_schema",
        ems::optional_override([](std::string const& schema_name,
                                  int                schema_version,
                                  ems::val           data) {
            OTIO_NS::AnyDictionary object_data = js_map_to_cpp(data);
            // TODO: We might need to use managing_ptr?
            return OTIO_NS::TypeRegistry::instance().instance_from_schema(
                schema_name,
                schema_version,
                object_data,
                ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    ems::function(
        "serialize_json_to_string",
        ems::optional_override([](OTIO_NS::SerializableObject* so) {
            // This is required because serialize_json_to_string needsa retainers.
            OTIO_NS::SerializableObject::Retainer<> retainer = so;

            return OTIO_NS::serialize_json_to_string(
                linb::any(retainer),
                nullptr,
                ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    // TODO: Figure out why when we use this, js_to_any returns an AnyDictionary...
    // ems::function(
    //     "serialize_json_to_string",
    //     ems::optional_override([](ems::val data) {
    //         return serialize_json_to_string(
    //             js_to_any(data),
    //             nullptr,
    //             ErrorStatusHandler());
    //     }));

    ems::function(
        "serialize_json_to_string",
        ems::optional_override(
            [](ems::val                           data,
               OTIO_NS::schema_version_map const& schema_version_targets) {
                return OTIO_NS::serialize_json_to_string(
                    js_to_any(data),
                    &schema_version_targets,
                    ErrorStatusHandler());
            }));

    ems::function(
        "serialize_json_to_string",
        ems::optional_override(
            [](ems::val                           data,
               OTIO_NS::schema_version_map const& schema_version_targets,
               int                                indent) {
                return OTIO_NS::serialize_json_to_string(
                    js_to_any(data),
                    &schema_version_targets,
                    ErrorStatusHandler(),
                    indent);
            }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override([](ems::val data, std::string filename) {
            return OTIO_NS::serialize_json_to_file(
                js_to_any(data),
                filename,
                nullptr,
                ErrorStatusHandler());
        }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override(
            [](ems::val                           data,
               std::string                        filename,
               OTIO_NS::schema_version_map const& schema_version_targets) {
                return OTIO_NS::serialize_json_to_file(
                    js_to_any(data),
                    filename,
                    &schema_version_targets,
                    ErrorStatusHandler());
            }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override(
            [](ems::val                           data,
               std::string                        filename,
               OTIO_NS::schema_version_map const& schema_version_targets,
               int                                indent) {
                return OTIO_NS::serialize_json_to_file(
                    js_to_any(data),
                    filename,
                    &schema_version_targets,
                    ErrorStatusHandler(),
                    indent);
            }));

    ems::function(
        "deserialize_json_from_string",
        ems::optional_override([](std::string input) {
            linb::any result;
            OTIO_NS::deserialize_json_from_string(
                input,
                &result,
                ErrorStatusHandler());
            return any_to_js(result, true);
        }));

    ems::function(
        "deserialize_json_from_file",
        ems::optional_override([](std::string filename) {
            linb::any result;
            OTIO_NS::deserialize_json_from_file(
                filename,
                &result,
                ErrorStatusHandler());
            // TODO: I think we need return a managing_ptr.
            return any_to_js(result, true);
        }));

    // TODO: Bind std::unordered_map
    ems::function("type_version_map", ems::optional_override([]() {
                      OTIO_NS::schema_version_map tmp;
                      OTIO_NS::TypeRegistry::instance().type_version_map(tmp);
                      return tmp;
                  }));

    ems::function("release_to_schema_version_map", ems::optional_override([]() {
                      return OTIO_NS::label_to_schema_version_map(
                          OTIO_NS::CORE_VERSION_MAP);
                  }));

    // TODO: Test
    ems::function(
        "flatten_stack",
        ems::optional_override([](OTIO_NS::Stack* stack) {
            return OTIO_NS::flatten_stack(stack, ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    // TODO: Test
    ems::function(
        "flatten_stack",
        ems::optional_override([](std::vector<OTIO_NS::Track*> tracks) {
            return OTIO_NS::flatten_stack(tracks, ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    // TODO: register_upgrade_function
    // TODO: register_downgrade_function
}
