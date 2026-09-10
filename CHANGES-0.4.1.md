# NCAS 0.4.1

- Integer coefficients render by juxtaposition: `2*x` becomes **2x**, including powers such as **3x²**. Explicit multiplication remains in the algebra source. Products of numbers, decimals and multi-letter identifiers retain ×. Cursor movement skips the hidden multiplication token; DEL beside the variable removes a coefficient digit.
- UP/DOWN leave the input editor at a structural boundary and browse the worksheet. DOWN can return from a recalled input to its answer. Tall objects are traversed before selecting the next region.
- An unfinished edit stays in RAM while its last calculated answer or other rows are inspected. Return to its input and press LEFT/RIGHT to resume, or EXE to apply it. Finish or clear that one pending edit before starting another. MENU reopening restores it as the active input using the existing compatible memory format.
- In input PAN mode, arrows move the cursor and the viewport follows. Result panning is bounded to the selected object. The inherited flashing cursor is disabled while NCAS draws and waits for keys.
- Error notifications slide in and out above the worksheet. Neither their appearance nor animation changes the calculation area's size or scroll position. A key press dismisses the notification and is returned to normal OS handling.
- NCAS suppresses the OS status-header drawing while it owns the screen. The original header no longer replaces it when pressing SHIFT. Yellow S and red A indicators sit immediately after the battery; settings move right, with the same header height. Alpha lock adds an underline to A.
- The app adapter evaluates the first new calculation as well as later appends; it no longer takes a zero-index bypass around the evaluator.
- Saving remains restricted to the MENU app-exit hook. No storage writes were added to edits, navigation, EXE, settings or notification animation.

The matched native add-in and companion have been rebuilt. Portable checks and the illustrated LaTeX guide were updated. Physical keyboard/header behavior, the notification key queue and MCS saving still need testing on an fx-CG50; see VALIDATION.md.
