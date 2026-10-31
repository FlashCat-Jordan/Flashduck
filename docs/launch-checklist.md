# Flashduck Launch Checklist

A practical checklist for a clean Flashduck release.

## Phase 0 — Private review

- [ ] README opens with one sentence that says what Flashduck is.
- [ ] README has the four-step replication flow and a detailed quick start.
- [ ] `llms.txt`, `AGENTS.md` and `CLAUDE.md` are in the repository root.
- [ ] Flashduck build passes.
- [ ] Host tests pass:
      `python3 -m unittest discover -s scripts/tests -v`.
- [ ] Images load on GitHub (relative paths under `docs/images/`).
- [ ] Repo description and topics are set.
- [ ] No private credentials, tokens or local paths are committed.
- [ ] A physical build has been photographed and, ideally, filmed walking.

## Phase 1 — Make it public

- [ ] Switch repository visibility to public when the owner is ready.
- [ ] Set a **Social preview** image in GitHub Settings
      (use `docs/images/flashduck-poster.webp`).
- [ ] Create a release, e.g. `v0.1.0`, with:
      `Flashduck-v0.1.0-merged.bin`, a short changelog and a photo.
- [ ] Pin the release and the “Show your build” issue if useful.
- [ ] Add the product page / assembly guide link to the repo homepage.

## Phase 2 — Make it readable by agents

- [ ] `llms.txt` has the canonical facts, build commands and limits.
- [ ] `README.md` has a visible “For AI agents” section.
- [ ] `AGENTS.md` explains build/test and what not to touch.
- [ ] `CLAUDE.md` gives coding agents the shortest safe path.
- [ ] The README comparison table is factual and does not disparage other
      projects.

## Phase 3 — Distribution (only after public)

- [ ] Publish a 30–60 s walking/eye video on Bilibili / 小红书 / YouTube Shorts.
- [ ] Post a build log or teardown with real photos.
- [ ] Submit to relevant Awesome lists when the category genuinely fits, e.g.
      `awesome-esp32`, `awesome-robotics`, `awesome-3d-printing`,
      `awesome-embedded`. Do not submit to unrelated lists.
- [ ] Write a “Show HN” or X thread with the hardware story, not just a link.
- [ ] Share the 3D model on MakerWorld with a link back to the repo.
- [ ] Invite makers to open Showcase issues with photos and parameters.

## Phase 4 — Community flywheel

- [ ] Reply to issues with real debugging, not canned answers.
- [ ] Label good first issues and showcase issues.
- [ ] Merge small, useful PRs; credit the contributor.
- [ ] Keep a changelog of gait/eye/action changes.

## What not to do

- Do not make false claims about hardware validation or production readiness.
- Do not submit the same link to unrelated Awesome lists or communities.
