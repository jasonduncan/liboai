# Releases

Publish maintained-fork releases in [jasonduncan/liboai](https://github.com/jasonduncan/liboai/releases). Use semantic versions: patches for compatible fixes, minors for additive APIs, and majors for intentional breaking changes. The first maintained-fork release is 4.1.0, following the historical 4.0.1 library version.

Initial releases contain source archives and SHA-256 checksums. They do not contain prebuilt libraries or bundled dependencies. vcpkg publication is a separate future task.

## Prepare and qualify

1. Start from a clean, current `main`. Update `project(oai VERSION ...)` in `liboai/CMakeLists.txt`, `CHANGELOG.md`, and the README's pinned release example. Preserve upstream license attribution.
2. Commit and push the release preparation. Require all five [CI configurations](CI.md) to pass for the exact commit to be tagged. The checks build examples, compile live harnesses without executing them, run offline tests, and exercise installed and parent-project consumers. Do not load `.env` or run paid live tests as a release prerequisite.
3. Build the source archive locally, run its offline tests, install it, and verify `find_package(oai <version> EXACT CONFIG REQUIRED)` against the installation. Use `--parallel 1` on memory-constrained machines. Record the source commit and CI run URL in the GitHub release notes.
4. Review the changelog and qualification limits. In particular, Windows shared liboai exports and cross-toolchain binary compatibility are not qualified.

## Package and publish

Use an explicit repository in every GitHub CLI command; this checkout also has an upstream remote. Substitute the intended version and fully qualified commit below. Never reuse or move a published version tag, and never push all inherited upstream tags.

```sh
version=4.1.0
release_commit=$(git rev-parse HEAD)
release_dir=$(mktemp -d)
git archive --format=tar --prefix="liboai-$version/" "$release_commit" | gzip -n > "$release_dir/liboai-$version.tar.gz"
git archive --format=zip --prefix="liboai-$version/" "$release_commit" > "$release_dir/liboai-$version.zip"
(cd "$release_dir" && shasum -a 256 "liboai-$version.tar.gz" "liboai-$version.zip" > SHA256SUMS)
```

Archives must come from the committed tree, never a recursive copy of the working directory. Inspect their contents: include `LICENSE`, public headers, build files, tests, examples, and documentation; exclude credentials, local builds, and private live receipts. Validate the tarball after extraction and verify both checksums before upload.

After qualification, create only the new annotated tag, then create a draft with all assets before publishing:

```sh
git tag -a "v$version" "$release_commit" -m "liboai $version"
git push origin "refs/tags/v$version"
gh release create "v$version" --repo jasonduncan/liboai --verify-tag --draft \
  --title "liboai $version" --notes-file /path/to/release-notes.md \
  "$release_dir/liboai-$version.tar.gz" "$release_dir/liboai-$version.zip" "$release_dir/SHA256SUMS"
gh release view "v$version" --repo jasonduncan/liboai
gh release edit "v$version" --repo jasonduncan/liboai --draft=false --latest
```

Verify the published release URL, tag commit, and downloaded asset checksums. GitHub also provides automatic source downloads; `SHA256SUMS` covers the explicitly attached `liboai-<version>` archives. Keep published tags and assets stable; ship corrections as a new patch version. Release creation is intentionally manual; ordinary CI never publishes packages.
