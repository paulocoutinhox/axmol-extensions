# Axmol Extensions

This repository has some extensions for [Axmol Game Engine](https://github.com/axmolengine/axmol).

Demo using WebAssembly:
https://axext.netlify.app/

## Extensions

- Splash
- Layer Pan Zoom
- Infinite Scroll
- Simple CheckBox

## Code of Conduct

- All code need be formatted with clang using `make format`.
- All code need be distributed with this license `MIT`.
- All extensions need have a sample scene using it and an item in startup menu.

## Utilities

General:

- Format source code: `make format`
- Clear trash: `make clean`

Build:

- Build for iOS: `make build-ios`
- Build for tvOS: `make build-tvos`
- Build for macOS: `make build-macos`
- Build for wasm: `make build-wasm`

Deploy:

- Deploy for iOS: `make deploy-ios`
- Deploy for tvOS: `make deploy-tvos`
- Deploy for Android: `make deploy-android`
- Deploy for wasm: `make deploy-wasm`

Server:

- Server for wasm demo: `python3 server.py`

## License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2023, Paulo Coutinho
