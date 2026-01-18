<h1>Maintainer Notes</h1>
<p>This doc summarizes where PRs live now and how to review them efficiently.</p>

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
