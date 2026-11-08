# API surface report

From the repository root, with Python 3 (`venv`/`pip`) and a native C++ toolchain installed:

```sh
make build
make install
pnpm api-coverage
```

Open `api-coverage/index.html`. If bindings are already built and installed,
only run `pnpm api-coverage`.

`scripts/api-coverage.html` is the source template; commit it. Generated files
under `api-coverage/` are ignored. The pinned Python API inventory is cached.
