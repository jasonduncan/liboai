<h1>Maintainer Notes</h1>
<p>This doc summarizes where PRs live now and how to review them efficiently.</p>

<h3>API modernization</h3>
<p>The <a href="./API_MODERNIZATION_PLAN.md">OpenAI API modernization plan</a> records the October 17, 2026 coverage baseline, implementation phases, compatibility rules, and acceptance criteria. The root <a href="../../ROADMAP.md">roadmap</a> summarizes the current priorities.</p>

<h3>Repositories and PR numbering</h3>
<ul>
  <li>Canonical repo (accept PRs here): <a href="https://github.com/jasonduncan/liboai">jasonduncan/liboai</a>.</li>
  <li>Upstream repo (archived): <a href="https://github.com/D7EAD/liboai">D7EAD/liboai</a>.</li>
  <li>PR numbers are per-repository, so PR <code>#1</code> in our repo is unrelated to PR <code>#1</code> upstream.</li>
</ul>

<h3>Common gh commands</h3>
<pre><code>gh pr list --repo jasonduncan/liboai

gh pr view 1 --repo jasonduncan/liboai

gh pr diff 1 --repo jasonduncan/liboai

gh pr checkout 1 --repo jasonduncan/liboai
</code></pre>

<p>Upstream PRs are read-only history but can be useful for reference:</p>
<pre><code>gh pr list --repo D7EAD/liboai
</code></pre>

<h3>Regression tests</h3>
<p>Regression tests are opt-in and use the same cURL and nlohmann-json dependencies as the library. No API key or external service is needed.</p>
<pre><code>cmake -S . -B build -DLIBOAI_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
</code></pre>
<p>For memory-safety checks with Clang or GCC, configure a Debug build with <code>-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"</code>.</p>

<h3>Remotes</h3>
<pre><code>git remote -v
</code></pre>
<p>Expected remotes:</p>
<ul>
  <li><code>origin</code>: jasonduncan/liboai</li>
  <li><code>upstream</code>: D7EAD/liboai</li>
</ul>

<h3>Review checklist (build / CMake changes)</h3>
<ul>
  <li>Verify minimum CMake version compatibility.</li>
  <li>Confirm dependency targets exist for both <code>find_package</code> and vendored targets.</li>
  <li>Test top-level build + install, and <code>add_subdirectory</code> usage in a parent project.</li>
  <li>Ensure new options are documented and defaults preserve existing behavior.</li>
</ul>

<h3>Modern API usage and evidence</h3>

- [Decisions and Skills extension plan](./DECISIONS_SKILLS_PLAN.md)
- [Decisions usage](../extensions/DECISIONS.md) and [Skills usage](../extensions/SKILLS.md)
- [Cross-platform CI and local reproduction](./CI.md)
- [Release procedure](./RELEASING.md)
- [Coverage and validation](./API_COVERAGE.md)
- [Live validation results and limitations](./LIVE_VALIDATION.md)
- [Opt-in live regression suite](./LIVE_TESTS.md)
- [Request options and diagnostics](./REQUEST_OPTIONS.md)
- [Modern component methods](./MODERN_COMPONENTS.md)
- [Supporting resources](../resources/resources.md)
- [Webhook verification](./WEBHOOK_VERIFICATION.md)
- [Historical interfaces](./HISTORICAL_APIS.md)

The HTTP fixture suite uses Python 3's standard library only, as test infrastructure. `BUILD_EXAMPLES=ON` compiles current and explicitly historical examples without running live API requests.
