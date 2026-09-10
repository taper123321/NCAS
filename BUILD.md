# Building NCAS

## Windows build

Requirements: Node.js 20 or newer, Windows `tar`, and approximately 1 GB of temporary space. Build inputs are fetched from the upstream author and PrizmSDK release and verified against `dependencies.lock.json`. No system-wide SDK installation, OS modification or environment-variable change is required.

From this directory:

```powershell
node tools/prepare.cjs
node tools/build.cjs
```

The prepared dependencies live under `.build`. To place them elsewhere, use the same absolute `--workspace` path with both scripts:

```powershell
node tools/prepare.cjs --workspace C:/path/to/build-directory
node tools/build.cjs --workspace C:/path/to/build-directory
```

The pair in `calculator/` is generated from one ELF link. Never combine a `.g3a` from one link with an `.ac2` from another. The build also writes a link map, ELF and intermediate objects in the chosen build directory.

## What is rebuilt

The default build recompiles the complete **new** editor, renderer, worksheet, toolbar, mathematical symbols, settings/calendar, memory codec, guided catalogue and calculator adapter. It also recompiles the upstream entry point, English catalogue and console with small integration changes. It links these with the engine objects and MicroPython archive already supplied in the pinned `giacbf.tgz` upstream release. It does **not** pretend to be a fresh source rebuild of the entire upstream engine or support libraries.

The original engine sources, makefiles and objects are included in that archive. The source archives for the modified upstream calculator library, USTL, TomMath and MicroPython ports are also provided. To rebuild those upstream components completely, use their own makefiles and the compiler/runtime expected by the upstream author, following the developer section of the KhiCAS documentation. The provided integration builder specifically supports the supplied Windows PrizmSDK 0.6 compiler and the pinned support binaries.

## Integration changes

- Enter `naturalcas_run()` before each original console input cycle; returning to the original workspace preserves its application menus.
- Load the companion filename `natcas.ac2` through the inherited RAM loader.
- Hook the OS quit handler to `naturalcas_save()`: save the worksheet, preferences and Giac checkpoints to MCS when MENU switches applications. Skip the inherited automatic file save once NCAS is active.
- Retain `natcas` naming for inherited explicit session-file operations; read old `natcas.cfg` preferences without writing them.
- Bypass the initial QR/overclock prompt and first-run syntax choice; the new front end uses Giac syntax internally.
- Preserve the upstream exam-mode check, OS check and RAM loader.
- Resolve upstream symlinks as regular files on Windows (`iostream`, `fxcg/rtc.h`).
- Align the USTL `ssize_t` typedef with the support C headers and set the standard integer-limit macros for the newer compiler.
- Keep native GetKey for ordinary input, disable the inherited flashing cursor and OS automatic header while NCAS owns the screen, and draw Shift/Alpha indicators from setup field 0x14. Notification animation polls briefly and returns an interrupting key to the OS queue.
- Place the new UI objects in the upstream companion region via their `z` object-name prefix.

## Portable editor tests

The core requires only C headers and no calculator headers. Compile with any suitable C++11 compiler:

```text
c++ -std=c++11 -O1 -fno-exceptions -fno-rtti src/editor.cpp src/screen.cpp src/catalog.cpp tests/editor_test.cpp -o editor-test
```

Run `editor-test`. An additional argument writes expression round-trip data into the current directory. Compile the worksheet suite separately:

```text
c++ -std=c++11 -O1 -fno-exceptions -fno-rtti src/editor.cpp src/screen.cpp src/catalog.cpp src/worksheet.cpp src/toolbar.cpp src/settings.cpp src/symbols.cpp src/memory.cpp tests/worksheet_test.cpp -o worksheet-test
```

Run `worksheet-test`. Add an argument such as `screenshots` to export PPM example screens in the current directory. The PPM files can be converted to PNG losslessly. They are host renders of the same display code, not physical-device captures.

`tests/reference.cjs` compares worked examples and serialised expressions with the separately downloaded, pinned upstream Giac JavaScript reference. It writes `tests/reference-results.json` and `tests/graph-reference-results.json`. Four 2D plot checks require drawing elements in generated SVG. Its comments describe required input locations. It is a reference check, not a SuperH emulator.

## Manual

Compile `manual/NCAS-Guide.tex` with Tectonic or a current LaTeX installation. The `images` directory and `operations.tex` must stay alongside the source. The delivered PDF was compiled with Tectonic 0.17.0 and visually checked after rendering.

## Dependency integrity

`prepare.cjs` refuses a changed checksum instead of accepting a newer upstream archive silently. If a URL changes, use the bundled source archive where available or explicitly update the lock file after reviewing the new upstream version. Dependency source archives retain their original names and contents. The included support headers have the two documented Windows compatibility fixes.

Sources: [KhiCAS developer documentation](https://www-fourier.univ-grenoble-alpes.fr/~parisse/casio/khicasioen.html), [PrizmSDK 0.6](https://github.com/Jonimoose/libfxcg/releases/tag/v0.6).
