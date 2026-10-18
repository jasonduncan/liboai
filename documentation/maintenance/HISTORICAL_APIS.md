# Historical interfaces and examples

Edits (`/edits`) and FineTunes (`/fine-tunes`) remain in C++ for source compatibility. Both hosted endpoints shut down on January 4, 2024. Their examples now carry a historical label and remain at their existing paths to preserve links and build targets. They compile under `BUILD_EXAMPLES=ON` but are not recommended as executable hosted workflows. Nothing silently redirects those calls to Chat, Responses, or newer fine-tuning endpoints.

The Completions, legacy image, and original chat examples also contain historical model names and older request conventions. Use the new `modern_*` examples and Responses workflows for current JSON request shapes; select a model your account supports. Historical code compiling successfully does not establish service or model availability.

The original Responses creation implementation is complete: raw JSON, common parameter serialization, asynchronous creation, and raw streaming callbacks all remain supported. Its earlier technical plan is a historical design record. The `seed` positional parameter remains a pass-through for compatibility; it is not a documented Responses creation option and new examples omit it.

Assistants and Videos are not expansion targets: their published retirement dates were August 26 and September 24, 2026. Evals and self-serve fine-tuning have wind-down schedules. Consult the [official deprecations page](https://developers.openai.com/api/docs/deprecations) before selecting replacement workflows or models. This reference was checked October 17, 2026.
