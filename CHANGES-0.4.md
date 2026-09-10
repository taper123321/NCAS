# NCAS 0.4.0 — 10 September 2026

- Save the worksheet and settings only when MENU switches to another app, using compressed MCS main memory. Restore the worksheet and unfinished input on reopening. No new USB worksheet files or save-on-EXE behavior; unchanged contents are not rewritten.
- Retain the newest 32 calculations, silently evicting the oldest on the next commit while keeping its algebra state.
- Restore a checkpoint and replay only the edited row and later rows. Follow current input, large matrices and replay results through the viewport.
- Add 2×2, 3×3, 3×1 and 2×1 matrix presets, an m×n dimensions popup, and natural matrix results.
- Add polar r∠θ complex display and algebraic normalisation of exact surd-containing results.
- Add natural sums, products, limits, nth derivatives, exponentials, indexed roots, conjugates, determinants, norms, inverse/transpose powers, identity matrices, combinations and vector layouts.
- Fix the indexed-root shortcut, direct e^x entry, nested cursor traversal and deletion of wholly empty templates.
- Delete the last calculation immediately from EDIT; Undo restores it. Clearing the whole sheet requires popup confirmation.
- Display timed red error notifications below the header, with temporary space reserved above input.
- Use one boxed status line, adaptive battery bars and a distinct NCAS badge. Preserve toolbar size with symbols or large abbreviations; root page cycling does not enter a Tools section.
- Replace the launcher graphic with the supplied matrix-node design and leave the caption area blank. Put the bold author credit left of the startup mark, with KhiCAS/Giac and MicroPython credits.
- Ask before switching to KhiCAS and explain how to return.
- Recognise saved graphs and route VIEW to the plot viewer. Four 2D reference cases pass; physical graph behavior and 3D remain unverified.
- Update the editable LaTeX guide, PDF, screenshots and native installation pair.

This development build passes portable checks and native linking. On-device keys, MCS saving/relaunch, memory limits and graph viewing still need validation. See VALIDATION.md.
