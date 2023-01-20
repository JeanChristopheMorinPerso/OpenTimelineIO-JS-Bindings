# OpenTimelineIO Javascript bindings

## Build

```
make setup
make build
make install
```

Right now, files will be installed in the `./install` directory. This is hardcoded.

## TODO

* The delete method in JS doesn't trigger the destructors... managing_ptr + Monitoralive are
  a little bit too good at keeping references alive.
* AnyVector is still unhandled.
* AnyDictionary works but is not fully tested.
* Support `JSON.stringify`? (This would require to implement the `toJSON` method on objects).
* Support toString methods?
* Go through skipped tests.
* Implement an "equal" + "notEqual" method on everything?
