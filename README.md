# OpenTimelineIO Javascript bindings

This repository is the home to the [OpenTimelineIO](https://github.com/AcademySoftwareFoundation/OpenTimelineIO)
Javascript bindings.

## State of the project

This is still a work in progress for now, but the base is there. That is:

* `SerializableObject`, `SerializableObjectWithMetadata`, ability to subclass them and
  register custom schemas from Javascripts.
* Serialization and deserialization is mostly working, though it needs more work to
  cover everything.
* `opentime` is implemented, but more work is needed on the comparison operators.
* Imath `V2d` and `Box2d` are also available. We still can't access the `x` property
  of v2d` for technical reasons.
* `AnyDictionary` is automatically converted from C++ to JS and from JS to C++.
* `AnyVector` is a work in progress.
* Objects lifecycle needs more work. I think some instances are "leaked" (they stey
  alive while they should get deleted).
* Tests live in the [tests](./tests) directory.

## Build

```
make setup
make build
make install
```

Right now, files will be installed in the `./install` directory. This is hardcoded.

To run the tests, run `npm test`. [Jest](https://jestjs.io/) is used as a the test runner.
Additional arguments can be passed to Jest like this: `npm test -- <additional arguments>`.

## TODO

Most TODOs are documented in the code. Search for `TODO:` and you will find everything that
still needs to be done. Other TODOs are bellow:

* The delete method in JS doesn't trigger the destructors... managing_ptr + KeepaliveMonitor are
  a little bit too good at keeping instances alive.
* AnyVector is still unhandled.
* AnyDictionary works but is not fully tested.
* Support `JSON.stringify`? (This would require to implement the `toJSON` method on objects).
* Support toString methods?
* Go through skipped tests.
* Implement an "equal" + "notEqual" method on everything?
