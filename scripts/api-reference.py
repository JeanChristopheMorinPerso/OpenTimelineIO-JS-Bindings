"""Export the pinned Python binding surface without constructing OTIO objects."""

import inspect
import json

import opentimelineio as otio


def export_surface():
    rows = []
    seen = {}

    def add_class(cls, group, name):
        if cls in seen:
            seen[cls]["aliases"].append(f"{group}.{name}")
            return
        row = {
            "group": group, "name": name, "kind": "class", "aliases": [],
            "origin": cls.__module__, "doc": inspect.getdoc(cls) or "",
            "constructorDoc": inspect.getdoc(cls.__init__) or "",
        }
        seen[cls] = row
        rows.append(row)
        members = {}
        for base in reversed(cls.__mro__[:-1]):
            if base.__module__.startswith("opentimelineio"):
                members.update({key: (base, value) for key, value in vars(base).items()})
        for member, (owner, descriptor) in sorted(members.items()):
            protocol = member.startswith("__") and member.endswith("__")
            if member.startswith("_") and not protocol:
                continue
            if protocol and member in {
                "__module__", "__dict__", "__weakref__", "__doc__", "__init__",
                "__new__", "__del__", "__getattribute__", "__setattr__",
                "__reduce__", "__reduce_ex__", "__getstate__", "__setstate__",
                "__init_subclass__", "__subclasshook__",
            }:
                continue
            value = inspect.getattr_static(cls, member)
            if inspect.isclass(value):
                add_class(value, group, f"{name}.{member}")
                continue
            kind = "constant"
            if isinstance(value, property) or inspect.isgetsetdescriptor(value):
                kind = "property"
                if type(value).__name__ == "pybind11_static_property":
                    kind = "static property"
            elif isinstance(value, staticmethod):
                kind = "static method"
            elif callable(value) or inspect.ismethoddescriptor(value):
                kind = "method"
            rows.append({
                "group": group, "name": f"{name}.{member}", "className": name,
                "member": member, "kind": kind, "protocol": protocol,
                "owner": owner.__name__, "origin": owner.__module__,
                "writable": bool(value.fset) if isinstance(value, property) else None,
                "doc": inspect.getdoc(descriptor) or "",
            })

    # Public core/schema/time APIs and algorithms, plus the native bundle API.
    # Plugins, adapters, CLI, exceptions and private implementation helpers are
    # not a C++ binding completeness baseline.
    groups = {
        "opentime": otio.opentime, "core": otio.core, "schema": otio.schema,
        "algorithms": otio.algorithms, "bundle": otio._otio.bundle,
    }
    functions = {}
    for group, module in groups.items():
        for name, value in sorted(vars(module).items()):
            if name.startswith("_") or inspect.ismodule(value):
                continue
            if inspect.isclass(value):
                if not issubclass(value, BaseException):
                    add_class(value, group, name)
            elif callable(value):
                if value in functions:
                    functions[value]["aliases"].append(f"{group}.{name}")
                    continue
                row = {
                    "group": group, "name": name, "kind": "function", "aliases": [],
                    "origin": getattr(value, "__module__", ""),
                    "doc": inspect.getdoc(value) or "",
                }
                functions[value] = row
                rows.append(row)
    return {"version": otio.__version__ if hasattr(otio, "__version__") else "development",
            "rows": rows}


if __name__ == "__main__":
    print(json.dumps(export_surface()))
