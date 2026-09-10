# NCAS for the Casio fx-CG50

A native C++ natural-input worksheet for the full CG50 KhiCAS/Giac engine..

**Development release 0.4.1. Cross-compiled and host-tested; this binary has not been tested here on a physical calculator or emulator.**

Copy **both** `calculator/NCAS.g3a` and `calculator/natcas.ac2` to the calculator drive's root, safely eject it, then open **NCAS**. Always replace the matched pair together. Remove an old `NaturalCAS.g3a` launcher when upgrading; the companion retains its historical filename for loader compatibility.

## Save only through MENU

Typing, calculating, deleting, Undo and preference changes stay in RAM. **Press MENU and select another application to save.** The OS app-exit hook then writes a compressed record to calculator main memory (MCS). Simply displaying MENU and returning may not invoke that hook. Unchanged contents are not rewritten.

Reopening NCAS restores the retained inputs and answers, unfinished input and selected field, settings and algebra checkpoints. It creates no separate worksheet files in USB storage. Work since the last MENU app switch is unsaved and can be lost on a reset or crash. The clock editor sets the hardware clock immediately; its first-use confirmation is retained at the next exit save.

The MCS record is `@NATCAS/WORKSHT`, bounded to 48 KiB compressed and 512 KiB before compression. MCS space is shared with other calculator data. Very large algebra values or full main memory can prevent a save; an error is shown. The previous record is not deleted before requesting an OS overwrite. The old `natcas.cfg` can supply legacy preferences but is no longer written. Explicit file operations in inherited KhiCAS applications retain their original behavior.

## Worksheet controls

- The newest **32 calculations** remain in a continuous scrollable sheet. A 33rd commit silently removes the oldest row while preserving its algebra state as the new baseline.
- UP/DOWN browse input and result regions. LEFT/RIGHT start editing the selected input. An edit restores the preceding checkpoint and recalculates **only that row and those below it**, following the results down the sheet.
- At the edge of a recalled input, DOWN returns to its answer. An unfinished edit can be browsed away from and resumed with LEFT/RIGHT; its old answer stays unchanged until EXE. Finish or clear that edit before editing another input.
- The current input stays visible when it fits. Larger expressions follow the active field; tall inputs and answers scroll independently. **JUMP > PAN** provides two-axis inspection of results; within an input, it moves the cursor and the viewport follows.
- Fractions, powers, indexed roots, sums, products, limits, derivatives, integrals and common matrix/complex operations use natural layouts. Integer coefficients display as `2x` and `3x²`; other multiplication uses ×. Division is ÷; the fraction button inserts stacked fields. Brackets open and close manually.
- MAT offers **2×2, 3×3, 3×1 and 2×1**, plus an **m×n** dimensions popup. Cells are edited in the main worksheet. Inline grids support 1–6 rows and columns; matrix results render as grids.
- **AC** clears the whole current input. EXIT does the same at the root toolbar and backs out of submenus. DEL removes a wholly empty fraction or other template. **EDIT > FIELD** clears one field.
- **EDIT > DELETE** immediately removes the last calculation; Undo restores it during the current launch. **CLEAR** asks before clearing the whole sheet and cannot be undone.
- The single-line header boxes angle, output, domain and digits. An adaptive battery icon stays leftmost; yellow **S** and red **A** show Shift/Alpha without replacing the NCAS header; date/time uses DD/MM HH:MM. A divider separates the NCAS badge. Red notifications slide over the worksheet below the header at the right, leaving the viewport unchanged. They dismiss after a brief pause or a key press; that key is then handled normally.
- Toolbar pages keep their 24-pixel height, with large abbreviations or mathematical symbols. F6 cycles both root pages without a Tools heading; submenu names appear in the header.

English, degrees, real domain and exact output are defaults. **SHIFT + MENU** or **F6 > SETUP** opens full-screen Settings. Domain options are **REAL**, **A+Bi** and **r∠θ**. Functions never silently switch to radians. Exact surd-containing answers are algebraically normalised for rationalised display.

First launch asks for local date and 24-hour time. The clock advances in hardware; its displayed value refreshes on interaction. Correct it manually after a daylight-saving change or battery reset. Battery bars indicate approximate voltage bands, not a precise charge percentage.

## Graphs and KhiCAS

**GRAPH** opens guided plotting templates and the inherited viewer. A plot result is identified as **Graph (VIEW)**; choose VIEW to reopen it. Four 2D graph examples pass reference checks; actual calculator viewing and the 3D path still need device verification.

**APPS** asks before switching to the original KhiCAS interface, with Cancel selected initially. To return, EXIT to its calculation prompt, AC to clear, then type **0 EXE**. NCAS reopens at the next console input cycle. A separate standalone KhiCAS add-in is left through MENU > NCAS.

All original engine functions remain linked; 71 common operations have guided templates. Specialist applications retain their original interfaces and syntax. Commands with external side effects are best kept in that workspace because worksheet replay can repeat those effects. Start a fresh sheet to adopt externally changed variables.

## Documentation and source

The detailed illustrated guide is `manual/NCAS-Guide.pdf`. Its editable LaTeX source is `manual/NCAS-Guide.tex`, with `operations.tex` and `images/` alongside it.

- `src/`: C++ editor, renderer, worksheet, toolbar, memory codec and engine adapter.
- `assets/`: matrix-node vector mark and icons with reserved launcher-caption space.
- `BUILD.md`: integration build and dependency provenance.
- `VALIDATION.md`: evidence, limits and outstanding device checks.
- `CHANGES-0.4.1.md`: latest fixes; `CHANGES-0.4.md`: earlier changes.
- `tests/`: portable checks and recorded Giac reference results.
- `third_party/`: original source archives, support components and notices.

New code is GPL-3.0-or-later. Retain inherited component licences. NCAS is not an official Casio product.
