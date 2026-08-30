# OpenTimelineIO Javascript bindings

This repository is the home to the [OpenTimelineIO](https://github.com/AcademySoftwareFoundation/OpenTimelineIO)
Javascript bindings.

## Prerequisites

- Node 20 or higher ([https://nodejs.org/en/download](https://nodejs.org/en/download))
- [pnpm](https://pnpm.io/installation) 11 or higher
## Getting Started
Follow these steps to set up and run the JS Bindings project on your local machine:

1. Create a fork of this repository

2. Clone the repository
```bash
git clone --recurse-submodules https://github.com/{{YOUR_GITHUB_USERNAME}}/OpenTimelineIO-JS-Bindings.git
cd OpenTimelineIO-JS-Bindings
```

3. Install dependencies
```bash
make setup
```

4. Build project
```bash
make build
make install
```

Right now, files will be installed in the `./install` directory. This is hardcoded.

5. Run unit tests
```bash
pnpm test
```

[Jest](https://jestjs.io/) is used as a the test runner.
Additional arguments can be passed to Jest like this: `pnpm test <additional arguments>`.

## State of the project

This is still a work in progress for now, but the base is there. That is:

* `SerializableObject`, `SerializableObjectWithMetadata`, ability to subclass them and
  register custom schemas from Javascript.
* `Composable`, `Marker`, `Clip` (partial) and `SerializableCollection` (partial).
* Serialization and deserialization is mostly working, though it needs more work to
  cover everything.
* `opentime` is implemented, but more work is needed on the comparison operators.
* Imath `V2d` and `Box2d` are also available. We still can't access the `x` property
  of `v2d` for technical reasons.
* `AnyDictionary` is automatically converted from C++ to JS and from JS to C++.
* `AnyVector` is a work in progress.
* Object lifetimes are shared between JavaScript handles and OTIO's C++ retainers.
  Call `delete()` on JavaScript handles when they are no longer needed.
  For JavaScript subclasses, `delete()` releases the caller's reference but does
  not invalidate the handle while C++ still retains the object. In that case,
  `isDeleted()` remains `false` until the final C++ owner releases the object and
  its destructor runs. This behavior may change as part of the lifetime-management
  TODO below.
* `std::optional` is working.
* Tests live in the [tests](./tests) directory.
* Tests are run automatically on every push using GitHub Actions.

## TODO

Most TODOs are documented in the code. Search for `TODO:` and you will find everything that
still needs to be done. Other TODOs are bellow:

* AnyVector is still unhandled.
* AnyDictionary works but is not fully tested.
* Simplify JavaScript object lifetime management: keep `managing_ptr` as the
  adapter to OTIO's C++ retainers, replace owning raw-pointer returns with
  `managing_ptr` returns, and then remove the Python-inspired `KeepaliveMonitor`.
* Support `JSON.stringify`? (This would require to implement the `toJSON` method on objects).
* Support toString methods?
* Go through skipped tests.
* Implement an "equal" + "notEqual" method on everything?
* Class functions are not available on sub-classes (https://github.com/emscripten-core/emscripten/issues/18722).
