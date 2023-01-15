// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <optional>
#include <string>
#include <vector>

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

#include "errorStatusHandler.h"
#include "js_anyDictionary.h"
#include "utils.h"

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

REGISTER_DESTRUCTOR(SerializableObject);
REGISTER_DESTRUCTOR(UnknownSchema);
REGISTER_DESTRUCTOR(SerializableObjectWithMetadata);
REGISTER_DESTRUCTOR(Marker);
REGISTER_DESTRUCTOR(SerializableCollection);
REGISTER_DESTRUCTOR(Composable);
REGISTER_DESTRUCTOR(Item);
REGISTER_DESTRUCTOR(Transition);
REGISTER_DESTRUCTOR(Gap);
REGISTER_DESTRUCTOR(Clip);
REGISTER_DESTRUCTOR(Composition);
REGISTER_DESTRUCTOR(Track);
REGISTER_DESTRUCTOR(Stack);
REGISTER_DESTRUCTOR(Timeline);
REGISTER_DESTRUCTOR(Effect);
REGISTER_DESTRUCTOR(TimeEffect);
REGISTER_DESTRUCTOR(LinearTimeWarp);
REGISTER_DESTRUCTOR(FreezeFrame);
REGISTER_DESTRUCTOR(MediaReference);
REGISTER_DESTRUCTOR(GeneratorReference);
REGISTER_DESTRUCTOR(MissingReference);
REGISTER_DESTRUCTOR(ExternalReference);
REGISTER_DESTRUCTOR(ImageSequenceReference);

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
    bool find_children(T* t, ems::val descended_from_type, optional<TimeRange> const& search_range, bool shallow_search, std::vector<SerializableObject*>& l) {
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
    std::vector<SerializableObject*> find_children(T* t, ems::val descended_from_type, optional<TimeRange> const& search_range, bool shallow_search = false) {
        std::vector<SerializableObject*> l;
        if (find_children<T, Clip>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Composition>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Gap>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Item>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Stack>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Timeline>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Track>(t, descended_from_type, search_range, shallow_search, l)) ;
        else if (find_children<T, Transition>(t, descended_from_type, search_range, shallow_search, l)) ;
        else
        {
            for (const auto& child : t->template find_children<Composable>(ErrorStatusHandler(), search_range, shallow_search)) {
                l.push_back(child.value);
            }
        }
        return l;
    }

    template<typename T>
    std::vector<SerializableObject*> find_clips(T* t, optional<TimeRange> const& search_range, bool shallow_search = false) {
        std::vector<SerializableObject*> l;
        for (const auto& clip : t->find_clips(ErrorStatusHandler(), search_range, shallow_search)) {
            l.push_back(clip.value);
        }
        return l;
    }
} // namespace
// clang-format on

EMSCRIPTEN_BINDINGS(opentimelineio)
{
    ems::register_vector<SerializableObject*>("SOVector");

    ems::class_<SerializableObject>("SerializableObject")
        .constructor<>()
        .function("is_equivalent_to", &SerializableObject::is_equivalent_to)
        .function(
            "clone",
            ems::optional_override([](SerializableObject* so) {
                return so->clone(ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function(
            "to_json_string",
            ems::optional_override([](SerializableObject* so, int indent) {
                return so->to_json_string(ErrorStatusHandler(), {}, indent);
            }),
            ems::allow_raw_pointers())
        .function(
            "to_json_file",
            ems::optional_override(
                [](SerializableObject* so, std::string file_name, int indent) {
                    return so->to_json_file(
                        file_name,
                        ErrorStatusHandler(),
                        {},
                        indent);
                }),
            ems::allow_raw_pointers())
        .class_function(
            "from_json_string",
            ems::optional_override([](std::string input) {
                return SerializableObject::from_json_string(
                    input,
                    ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .class_function(
            "from_json_file",
            ems::optional_override([](std::string file_name) {
                return SerializableObject::from_json_file(
                    file_name,
                    ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function("schema_name", &SerializableObject::schema_name)
        .function("schema_version", &SerializableObject::schema_version)
        .property("is_unknown_schema", &SerializableObject::is_unknown_schema);

    ems::class_<UnknownSchema, ems::base<SerializableObject>>("UnknownSchema")
        .constructor<std::string, int>()
        .property("original_schema_name", &UnknownSchema::original_schema_name)
        .property(
            "original_schema_version",
            &UnknownSchema::original_schema_version);

    ems::class_<SerializableObjectWithMetadata, ems::base<SerializableObject>>(
        "SerializableObjectWithMetadata")
        .constructor<>()
        .constructor<std::string>()
        .constructor(
            ems::optional_override([](std::string name, ems::val metadata) {
                AnyDictionary d = js_map_to_cpp(metadata);
                return new SerializableObjectWithMetadata(name, d);
            }))
        .property(
            "name",
            &SerializableObjectWithMetadata::name,
            &SerializableObjectWithMetadata::set_name)
        .property(
            "metadata",
            // TODO: Should we instead return the reference? AFAIK we can't
            // to ems::select_overload<AnyDictionary&() noexcept>
            // Also, how will we override metadata? For example so.metadata?
            ems::select_overload<AnyDictionary() const noexcept>(
                &SerializableObjectWithMetadata::metadata))
        // TODO: This dirty, but so far I didn't find a nicer method.
        // Binded getters seem to be read-only (it cannot return a pointer or a reference).
        // Additionally, it seems impossible to accept a pointer or reference for the "this"
        // in setters.
        .function(
            "set_metadata",
            ems::optional_override([](SerializableObjectWithMetadata& so,
                                      AnyDictionary metadata) {
                std::cout << "Size before swap: "
                                 + std::to_string(so.metadata().size()) + "\n";
                AnyDictionary& old_metadata = so.metadata();
                old_metadata                = metadata;
                std::cout << "Size after swap: "
                                 + std::to_string(so.metadata().size()) + "\n";
            }));

    ems::class_<Marker, ems::base<SerializableObjectWithMetadata>>("Marker")
        .constructor(ems::optional_override([](std::string        name,
                                               TimeRange          marked_range,
                                               std::string const& color,
                                               ems::val           metadata) {
            return new Marker(
                name,
                marked_range,
                color,
                js_map_to_cpp(metadata));
        }))
        .property("color", &Marker::color, &Marker::set_color)
        .property(
            "marked_range",
            &Marker::marked_range,
            &Marker::set_marked_range);

    // TODO: Should this be an enum or maybe something else?
    ems::class_<Marker::Color>("MarkerColor");

    ems::class_<
        SerializableCollection,
        ems::base<SerializableObjectWithMetadata>>("SerializableCollection")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const&               name,
               std::vector<SerializableObject*> children) {
                return new SerializableCollection(
                    name,
                    children,
                    AnyDictionary());
            }))
        .constructor(
            ems::optional_override([](std::string const&               name,
                                      std::vector<SerializableObject*> children,
                                      ems::val metadata) {
                return new SerializableCollection(
                    name,
                    children,
                    js_map_to_cpp(metadata));
            }));

    ems::class_<Composable, ems::base<SerializableObjectWithMetadata>>(
        "Composable")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std ::string const& name, ems::val metadata) {
                return new Composable(name, js_map_to_cpp(metadata));
            }))
        .function("parent", &Composable::parent, ems::allow_raw_pointers());

    // TODO: Implement
    ems::class_<Item, ems::base<Composable>>("Item");

    ems::class_<Transition, ems::base<Composable>>("Transition")
        .constructor(
            ems::optional_override([](std::string const& name,
                                      std::string const& transition_type,
                                      RationalTime       in_offset,
                                      RationalTime       out_offset,
                                      ems::val           metadata) {
                return new Transition(
                    name,
                    transition_type,
                    in_offset,
                    out_offset,
                    js_map_to_cpp(metadata));
            }))
        .property(
            "transition_type",
            &Transition::transition_type,
            &Transition::set_transition_type)
        .property(
            "in_offset",
            &Transition::in_offset,
            &Transition::set_in_offset)
        .property(
            "out_offset",
            &Transition::out_offset,
            &Transition::set_out_offset)
        .function("duration", ems::optional_override([](Transition& t) {
                      return t.duration(ErrorStatusHandler());
                  }))
        .function("range_in_parent", ems::optional_override([](Transition& t) {
                      return t.range_in_parent(ErrorStatusHandler());
                  }))
        .function(
            "trimmed_range_in_parent",
            ems::optional_override([](Transition& t) {
                return t.trimmed_range_in_parent(ErrorStatusHandler());
            }));

    // TODO: Implement
    ems::class_<Transition::Type>("TransitionType");

    // TODO: Implement
    ems::class_<Gap, ems::base<Item>>("Gap");

    // TODO: Test MediaReference as input
    ems::class_<Clip, ems::base<Item>>("Clip")
        .constructor(
            ems::optional_override(
                [](std::string const& name,
                   MediaReference*    media_reference,
                   TimeRange          source_range,
                   ems::val           metadata,
                   std::string const& active_media_reference) {
                    return new Clip(
                        name,
                        media_reference,
                        source_range,
                        js_map_to_cpp(metadata),
                        active_media_reference);
                }),
            ems::allow_raw_pointers())
        .class_property("DEFAULT_MEDIA_KEY", &DEFAULT_MEDIA_KEY)

        // TODO: This should be a property, but embind doesn't support pointers in properties.
        // And on top of that we can't even use ems::optional_override to dereference the pointer
        // because that could require the copy constructor, which is implicitly deleted by SOWithMetadata.
        .function(
            "media_reference",
            &Clip::media_reference,
            ems::allow_raw_pointers())
        .function(
            "set_media_reference",
            &Clip::set_media_reference,
            ems::allow_raw_pointers())
        .property(
            "active_media_reference_key",
            &Clip::active_media_reference_key,
            ems::optional_override(
                [](Clip& clip, std::string const& new_active_key) {
                    clip.set_active_media_reference_key(
                        new_active_key,
                        ErrorStatusHandler());
                }))
        .function("media_references", &Clip::media_references)
        .function(
            "set_media_references",
            ems::optional_override(
                [](Clip*                        clip,
                   Clip::MediaReferences const& media_references,
                   std::string const&           new_active_key) {
                    clip->set_media_references(
                        media_references,
                        new_active_key,
                        ErrorStatusHandler());
                }),
            ems::allow_raw_pointers());

    // TODO: Implement
    ems::class_<Composition, ems::base<Item>>("Composition")
        .constructor(
            ems::optional_override([](std::string const&       name,
                                      std::vector<Composable*> children,
                                      TimeRange                source_range,
                                      ems::val                 metadata) {
                Composition* c = new Composition(
                    name,
                    source_range,
                    js_map_to_cpp(metadata));
                c->set_children(children, ErrorStatusHandler());
                return c;
            }))
        // TODO: It works but the returned vector contains zero item.
        .function(
            "find_children",
            ems::optional_override([](Composition const& c) {
                std::vector<SerializableObject*> l;
                for (const auto& composable:
                     c.find_children(ErrorStatusHandler()))
                {
                    l.push_back(composable.value);
                }
                return l;
            }));

    ems::enum_<Track::NeighborGapPolicy>("TrackNeighborGapPolicy")
        .value(
            "around_transitions",
            Track::NeighborGapPolicy::around_transitions)
        .value("never", Track::NeighborGapPolicy::never);

    // TODO: Implement
    ems::class_<Track, ems::base<Composition>>("Track")
        .constructor<>()
        .constructor<std::string>()
        .constructor(
            ems::optional_override([](std::string const&       name,
                                      std::vector<Composable*> children,
                                      TimeRange const&         source_range,
                                      std::string const&       kind,
                                      ems::val                 metadata) {
                Track* t = new Track(
                    name,
                    source_range,
                    kind,
                    js_map_to_cpp(metadata));
                t->set_children(children, ErrorStatusHandler());
                return t;
            }))
        .property("kind", &Track::kind, &Track::set_kind)
        .function(
            "neighbors_of",
            ems::optional_override([](Track const&             t,
                                      Composable&              item,
                                      Track::NeighborGapPolicy policy) {
                auto result =
                    t.neighbors_of(&item, ErrorStatusHandler(), policy);
                return result;
            }));

    // TODO: Implement
    ems::class_<Track::Kind>("TrackKind");

    // TODO: Implement
    ems::class_<Stack, ems::base<Composition>>("Stack")
        .constructor<>()
        .constructor<std::string>()
        .constructor(
            ems::optional_override([](std::string const&       name,
                                      std::vector<Composable*> children,
                                      TimeRange const&         source_range,
                                      std::vector<Marker*>     markers,
                                      std::vector<Effect*>     effects,
                                      ems::val                 metadata) {
                Stack* s = new Stack(
                    name,
                    source_range,
                    js_map_to_cpp(metadata),
                    effects,
                    markers);
                s->set_children(children, ErrorStatusHandler());
                return s;
            }));

    // TODO: Implement
    ems::class_<Timeline, ems::base<SerializableObjectWithMetadata>>(
        "Timeline");

    ems::class_<Effect, ems::base<SerializableObjectWithMetadata>>("Effect")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, std::string>()
        .constructor(ems::optional_override([](std::string const& name,
                                               std::string const& effect_name,
                                               ems::val           metadata) {
            return new Effect(name, effect_name, js_map_to_cpp(metadata));
        }))
        .property(
            "effect_name",
            &Effect::effect_name,
            &Effect::set_effect_name);

    ems::class_<TimeEffect, ems::base<Effect>>("TimeEffect")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, std::string>()
        .constructor(ems::optional_override([](std::string const& name,
                                               std::string const& effect_name,
                                               ems::val           metadata) {
            return new TimeEffect(name, effect_name, js_map_to_cpp(metadata));
        }));

    ems::class_<LinearTimeWarp, ems::base<TimeEffect>>("LinearTimeWarp")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const& name, double time_scalar) {
                return new LinearTimeWarp(name, "LinearTimeWarp", time_scalar);
            }))
        .constructor(ems::optional_override(
            [](std::string const& name, double time_scalar, ems::val metadata) {
                return new LinearTimeWarp(
                    name,
                    "LinearTimeWarp",
                    time_scalar,
                    js_map_to_cpp(metadata));
            }))
        .property(
            "time_scalar",
            &LinearTimeWarp::time_scalar,
            &LinearTimeWarp::set_time_scalar);

    ems::class_<FreezeFrame, ems::base<LinearTimeWarp>>("FreezeFrame")
        .constructor<>()
        .constructor<std::string>()
        .constructor(ems::optional_override(
            [](std::string const& name, ems::val metadata) {
                return new FreezeFrame(name, js_map_to_cpp(metadata));
            }));

    ems::class_<MediaReference, ems::base<SerializableObjectWithMetadata>>(
        "MediaReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, TimeRange>()
        .constructor(ems::optional_override([](std::string const& name,
                                               TimeRange available_range,
                                               ems::val  metadata) {
            return new MediaReference(
                name,
                available_range,
                js_map_to_cpp(metadata));
        }))
        .constructor(ems::optional_override(
            [](std::string const&  name,
               TimeRange           available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new MediaReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "available_range",
            &MediaReference::available_range,
            &MediaReference::set_available_range)
        .property(
            "available_image_bounds",
            &MediaReference::available_image_bounds,
            &MediaReference::set_available_image_bounds)
        .property(
            "is_missing_reference",
            &MediaReference::is_missing_reference);

    ems::class_<GeneratorReference, ems::base<MediaReference>>(
        "GeneratorReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, std::string>()
        .constructor<std::string, std::string, TimeRange>()
        .constructor(
            ems::optional_override([](std::string const& name,
                                      std::string const& generator_kind,
                                      TimeRange const&   available_range,
                                      ems::val           parameters) {
                return new GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters));
            }))
        .constructor(
            ems::optional_override([](std::string const& name,
                                      std::string const& generator_kind,
                                      TimeRange const&   available_range,
                                      ems::val           parameters,
                                      ems::val           metadata) {
                return new GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters),
                    js_map_to_cpp(metadata));
            }))
        .constructor(ems::optional_override(
            [](std::string const&  name,
               std::string const&  generator_kind,
               TimeRange const&    available_range,
               ems::val            parameters,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new GeneratorReference(
                    name,
                    generator_kind,
                    available_range,
                    js_map_to_cpp(parameters),
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "generator_kind",
            &GeneratorReference::generator_kind,
            &GeneratorReference::set_generator_kind)
        .property(
            "parameters",
            // TODO: Should we use const or the reference? (Using the reference results in a compilation error)
            ems::select_overload<AnyDictionary() const>(
                &GeneratorReference::parameters));

    ems::class_<MissingReference, ems::base<MediaReference>>("MissingReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, TimeRange>()
        .constructor(ems::optional_override([](std::string const& name,
                                               TimeRange available_range,
                                               ems::val  metadata) {
            return new MissingReference(
                name,
                available_range,
                js_map_to_cpp(metadata));
        }))
        .constructor(ems::optional_override(
            [](std::string const&  name,
               TimeRange           available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new MissingReference(
                    name,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }));

    ems::class_<ExternalReference, ems::base<MediaReference>>(
        "ExternalReference")
        .constructor<>()
        .constructor<std::string>()
        .constructor<std::string, TimeRange>()
        .constructor(ems::optional_override([](std::string const& target_url,
                                               TimeRange available_range,
                                               ems::val  metadata) {
            return new ExternalReference(
                target_url,
                available_range,
                js_map_to_cpp(metadata));
        }))
        .constructor(ems::optional_override(
            [](std::string const&  target_url,
               TimeRange           available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new ExternalReference(
                    target_url,
                    available_range,
                    js_map_to_cpp(metadata),
                    available_image_bounds);
            }))
        .property(
            "target_url",
            &ExternalReference::target_url,
            &ExternalReference::set_target_url);

    ems::enum_<ImageSequenceReference::MissingFramePolicy>("MissingFramePolicy")
        .value("error", ImageSequenceReference::MissingFramePolicy::error)
        .value("hold", ImageSequenceReference::MissingFramePolicy::hold)
        .value("black", ImageSequenceReference::MissingFramePolicy::black);

    ems::class_<ImageSequenceReference, ems::base<MediaReference>>(
        "ImageSequenceReference")
        .constructor(ems::optional_override(
            [](std::string const& target_url_base,
               std::string const& name_prefix,
               std::string const& name_suffix,
               int                start_frame,
               int                frame_step,
               double const       rate,
               int                frame_zero_padding,
               ImageSequenceReference::MissingFramePolicy const
                                   missing_frame_policy,
               TimeRange const&    available_range,
               ems::val            metadata,
               Imath::Box2d const& available_image_bounds) {
                return new ImageSequenceReference(
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
            &ImageSequenceReference::target_url_base,
            &ImageSequenceReference::set_target_url_base)
        .property(
            "name_prefix",
            &ImageSequenceReference::name_prefix,
            &ImageSequenceReference::set_name_prefix)
        .property(
            "name_suffix",
            &ImageSequenceReference::name_suffix,
            &ImageSequenceReference::set_name_suffix)
        .property(
            "start_frame",
            &ImageSequenceReference::start_frame,
            &ImageSequenceReference::set_start_frame)
        .property(
            "frame_step",
            &ImageSequenceReference::frame_step,
            &ImageSequenceReference::set_frame_step)
        .property(
            "rate",
            &ImageSequenceReference::rate,
            &ImageSequenceReference::set_rate)
        .property(
            "frame_zero_padding",
            &ImageSequenceReference::frame_zero_padding,
            &ImageSequenceReference::set_frame_zero_padding)
        .property(
            "missing_frame_policy",
            &ImageSequenceReference::missing_frame_policy,
            &ImageSequenceReference::set_missing_frame_policy)
        .function("end_frame", &ImageSequenceReference::end_frame)
        .function(
            "number_of_images_in_sequence",
            &ImageSequenceReference::number_of_images_in_sequence)
        .function(
            "frame_for_time",
            ems::optional_override(
                [](ImageSequenceReference const& ref, RationalTime time) {
                    return ref.frame_for_time(time, ErrorStatusHandler());
                }))
        .function(
            "target_url_for_image_number",
            ems::optional_override(
                [](ImageSequenceReference const& ref, int image_number) {
                    return ref.target_url_for_image_number(
                        image_number,
                        ErrorStatusHandler());
                }))
        .function(
            "presentation_time_for_image_number",
            ems::optional_override(
                [](ImageSequenceReference const& ref, int image_number) {
                    return ref.presentation_time_for_image_number(
                        image_number,
                        ErrorStatusHandler());
                }));

    ems::function(
        "serialize_json_to_string",
        ems::optional_override([](ems::val data) {
            return serialize_json_to_string(
                js_to_any(data),
                nullptr,
                ErrorStatusHandler());
        }));

    ems::function(
        "serialize_json_to_string",
        ems::optional_override(
            [](ems::val                  data,
               schema_version_map const& schema_version_targets) {
                return serialize_json_to_string(
                    js_to_any(data),
                    &schema_version_targets,
                    ErrorStatusHandler());
            }));

    ems::function(
        "serialize_json_to_string",
        ems::optional_override(
            [](ems::val                  data,
               schema_version_map const& schema_version_targets,
               int                       indent) {
                return serialize_json_to_string(
                    js_to_any(data),
                    &schema_version_targets,
                    ErrorStatusHandler(),
                    indent);
            }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override([](ems::val data, std::string filename) {
            return serialize_json_to_file(
                js_to_any(data),
                filename,
                nullptr,
                ErrorStatusHandler());
        }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override(
            [](ems::val                  data,
               std::string               filename,
               schema_version_map const& schema_version_targets) {
                return serialize_json_to_file(
                    js_to_any(data),
                    filename,
                    &schema_version_targets,
                    ErrorStatusHandler());
            }));

    ems::function(
        "serialize_json_to_file",
        ems::optional_override(
            [](ems::val                  data,
               std::string               filename,
               schema_version_map const& schema_version_targets,
               int                       indent) {
                return serialize_json_to_file(
                    js_to_any(data),
                    filename,
                    &schema_version_targets,
                    ErrorStatusHandler(),
                    indent);
            }));

    ems::function(
        "deserialize_json_from_string",
        ems::optional_override([](std::string input) {
            any result;
            deserialize_json_from_string(input, &result, ErrorStatusHandler());
            return any_to_js(result, true);
        }));

    ems::function(
        "deserialize_json_from_file",
        ems::optional_override([](std::string filename) {
            any result;
            deserialize_json_from_file(filename, &result, ErrorStatusHandler());
            return any_to_js(result, true);
        }));

    ems::function("type_version_map", ems::optional_override([]() {
                      schema_version_map tmp;
                      TypeRegistry::instance().type_version_map(tmp);
                      return tmp;
                  }));

    ems::function("release_to_schema_version_map", ems::optional_override([]() {
                      return label_to_schema_version_map(CORE_VERSION_MAP);
                  }));

    ems::function(
        "flatten_stack",
        ems::optional_override([](Stack* stack) {
            return flatten_stack(stack, ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());

    ems::function(
        "flatten_stack",
        ems::optional_override([](std::vector<Track*> tracks) {
            return flatten_stack(tracks, ErrorStatusHandler());
        }),
        ems::allow_raw_pointers());
    // TODO: register_serializable_object_type
    // TODO: set_type_record
    // TODO: register_upgrade_function
    // TODO: register_downgrade_function
    // TODO: instance_from_schema
}
