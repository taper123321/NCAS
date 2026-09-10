# NCAS 0.4.1 validation

Completed on 10 September 2026. This revision is cross-compiled and host-tested. User photographs show an earlier build running on a physical fx-CG50; they do not validate this new binary.

## Automated results

- **42,247 editor checks passed**, including 21,000 deterministic mixed editing actions, structural invariants, indexed roots, natural templates, cursor traversal, serialisation and capacity guards. The data-export argument was supplied.
- **1,154 worksheet/toolbar/settings/memory checks passed** before screenshot export. They cover suffix-only assignment/Ans replay, append-only evaluation, failure/pending states, 32-entry rolling retention, delete/undo, incomplete-fraction and selected-field restoration, tall/wide scrolling, 71-operation reachability, fixed toolbar geometry, date/BCD transport and battery bands. Compression stress covers 80 random/repetitive inputs up to 78,764 bytes, truncated streams, invalid references and tiny output buffers.
- **41/41 expected-answer checks passed** against the pinned upstream Giac JavaScript reference. **66 further non-graph probes** are recorded observations, not independent expected-answer assertions.
- **4/4 graph reference checks passed**: y=x², a parametric circle, a polar cardioid and an implicit circle produce SVG drawing elements. These checks do not execute the native CG50 viewer. 3D is unverified.
- Full SuperH add-in/companion linking succeeded with the new sources and pinned upstream engine objects.
- **58 example/atlas screens** were generated from the shared 384 × 216 C++ renderer and visually reviewed. Known example answers are supplied by fixtures; screenshots are not calculator execution captures.
- The editable LaTeX guide compiled to **27 pages**, all visually checked after rendering. There are no overfull boxes. One underfull paragraph warning remains without visible overflow.

## Linker memory use

| Region | Used | Capacity |
|---|---:|---:|
| Add-in ROM | 2,026,260 B | 2,065,152 B |
| Static RAM | 425,696 B | 442,368 B |
| Companion region | 2,365,962 B | 2,559,996 B |

These are linker-region figures, not available heap or measured runtime speed. The worksheet model is 382,448 bytes on the host layout; each editor is 36,972 bytes. Giac values, per-row context checkpoints and inherited applications require additional heap. Size-optimised SH4A code uses the upstream software-float calling convention. No hardware performance benchmark is claimed.

## This revision's interaction checks

Portable tests compare the rendered pixels for `2*x`/`2x`, signed integer coefficients and variable powers while preserving explicit source multiplication. They check digit deletion and strictly advancing visible cursor positions across a coefficient. Quoted strings, decimal/scientific coefficients and multi-letter identifiers are excluded from this display rule.

Repeated input-to-answer transitions, suspended edit restoration, protection against replacing a pending edit, and cursor visibility during matrix panning pass. Notification tests compare every pixel outside the overlay across all animation offsets and assert unchanged viewport geometry and scroll. Shift, Alpha and Alpha-lock indicator colours are checked in the shared renderer.

The calculator adapter retains native GetKey for normal keyboard, MENU and power handling. It disables automatic header drawing with EnableDisplayHeader and disables the OS status area using EnableStatusArea(3), whose argument is not a boolean. It switches off the inherited cursor flash. A short, bounded notification animation uses nonblocking key polling and requeues an interrupting event for GetKey. This adapter is cross-compiled, not exercised by the portable renderer tests.

The header integration follows first-hand WikiPrizm syscall documentation: [EnableStatusArea](https://prizm.cemetech.net/Syscalls/EnableStatusArea/), [EnableDisplayHeader](https://prizm.cemetech.net/Syscalls/EnableDisplayHeader/), [GetKey](https://prizm.cemetech.net/Syscalls/Keyboard/GetKey/) and [GetKeyWait_OS](https://prizm.cemetech.net/Syscalls/Keyboard/GetKeyWait_OS/). The pinned upstream console also identifies setup field 0x14 values 4/8/0x84/0x88 as Alpha states.

## MENU-only saving

Source inspection confirms the NCAS storage writer is called only from the upstream OS quit handler. There are no worksheet writes on typing, EXE, deletion, Undo or setting changes. The native record `@NATCAS/WORKSHT` uses MCS, not USB worksheet files. Identical content skips the write. The 4-byte-aligned record has a magic/version marker, lengths, checksum and bounded compression; load validates these before parsing. An old `natcas.cfg` may be read but is not written.

Portable checks validate worksheet serialisation, draft cursor restoration and the codec. They **do not validate the MCS syscalls, native Giac value/context round trips or OS exit timing on hardware**. MCS is shared, and NCAS caps the record at 48 KiB compressed / 512 KiB raw. Main-memory exhaustion is reported; the old record is not explicitly deleted before an overwrite request. Exotic Giac objects use a printed-symbolic fallback and require additional validation. Undo is not persisted.

## Device checks still required

1. Install both files and check the launcher caption, startup credit and date setup.
2. Check physical SHIFT and ALPHA keep the NCAS header, with yellow S/red A and Alpha-lock underline. Check SHIFT+MENU, MENU/power handling, indexed-root, exponential, DEL, AC, EXIT and all arrows in nested expressions. Move from a recalled input to its answer with DOWN, then resume a pending edit.
3. Enter assignments and Ans dependencies; edit a middle row and confirm earlier random/stateful rows stay unchanged. Check marker movement and interruption.
4. Enter 33 calculations, delete the last, undo, and verify retained dependencies and scrolling. Confirm whole-sheet Clear asks first.
5. Leave an incomplete fraction or matrix cell; use MENU to select another app and reopen NCAS. Verify input, field, settings, matrices, exact values and a subsequent historical edit. Repeat with near-capacity data and a backed-up device when checking memory-full behavior.
6. Check polar output, rationalised surds, matrix answers, graph opening/VIEW and KhiCAS confirmation/return. Trigger an error: its animation must leave the worksheet in place; immediately press a digit, arrow, SHIFT and MENU on separate attempts and confirm no lost or duplicate key.
7. Check battery bands, clock after relaunch and midnight rollover. The header refreshes on interaction, not during an indefinite GetKey wait; daylight-saving changes are manual.

The live editor has 511 usable nodes, depth 24, 2,048-byte input/rendered-result source buffers and inline grids up to 6 × 6. Specialised or oversized results use the retained full viewer. Tests do not establish real-device speed or complete compatibility for every inherited engine object.

## Reproduction

See BUILD.md. The worksheet test exports PPM screenshots with an extra argument. The mathematics and graph JSON records identify the separate reference engine. `calculator/SHA256SUMS.txt` identifies the matched installation pair; always replace both files. Original dependency archives and licences accompany the source package.
