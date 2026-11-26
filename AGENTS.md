## Code Guidelines

**Do not** bunch code up into un-readable formatted code.
This goes for doing functions on one line, always make code readable for anyone.
Don't use spaces for indenting, always tabs.

## Code comments

The default is **no comment**. Write code that explains itself through naming and structure instead.

A comment is only justified when it carries information that is *not recoverable from the code*, and that a
competent developer reading this file would otherwise get wrong. In practice that means:

- A non-obvious external constraint.
- A deliberate deviation: why the obvious approach was *not* taken, when the code alone would make a reader
  want to "fix" it.
- A reference that saves someone a research session: a spec section, an MSDN structure, a known-bug link.

Never write:

- Restatements of the code (`// increment the counter`, `// call the handler`, `// loop over the modules`).
- Section headers or banners inside a function (`// --- setup ---`, `// Step 3: cleanup`).
- Narration of your own edit or reasoning (`// changed to fix X`, `// this is safe because ...`, `// now uses Y`).
  This is talking to the reviewer, not to the next reader, and it is noise the moment the change is merged.
- Docstring-style headers that just spell out the signature in prose.
- Obvious type or scope information (`// pointer to the process`).

If you are unsure whether a comment qualifies, it does not. Leave it out. A change that adds zero comments is a
perfectly good change; a change that sprinkles comments over otherwise readable code will be rejected.
