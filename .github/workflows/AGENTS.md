# GitHub Actions

- Use version tags in workflows; commits are pinned in `.github/workflows/actions.lock`.
- After changing action dependencies, run `gh actions-lock` and `gh actions-lock --verify` from the repository root.
  Install the extension if needed: `gh extension install github/gh-actions-lock`.
- Include workflow and lockfile changes together. Never edit the lockfile by hand.
