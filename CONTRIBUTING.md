# Contributing

Thanks for building, printing, flashing or breaking something. This project is
hardware-first: a contribution does not have to be C++.

The repository has two firmware systems and a coding-agent skill:

- `firmware/voice/` — MIT voice firmware;
- `firmware/duckclaw/` — CC BY-NC-SA 4.0 local agent;
- `skills/flashduck-dev/` — environment checks and secondary-development recipes.

Do not move code between the two firmware projects without checking the licenses.

## Good first contributions

- A new duck action or eye expression (voice firmware).
- A new DuckClaw action or prompt (DuckClaw).
- A shell/3D-print remix, wiring note or build photo.
- A beginner recipe or troubleshooting note for the skill.
- A documentation fix that makes the two-system split clearer.

## Before opening a PR

1. Read `README.md` and `docs/architecture.md`.
2. Read `skills/flashduck-dev/SKILL.md`.
3. Change the smallest owning layer.
4. Build the affected firmware.
5. Run host tests if the voice project changed:

```bash
cd firmware/voice
python3 -m unittest discover -s scripts/tests -v
```

6. State what was built and what still needs real hardware.

## Hardware changes

- Preserve existing board pins. Add a new board/variant for a new revision.
- Keep mouth **45–60°** and neck **45–180°**.
- Include photos or a video for shell/action changes if possible.
- A compile is not a hardware test.

## Commit and PR guidance

- Keep patches focused.
- Do not commit `build/`, `managed_components/`, `sdkconfig*`, generated assets or secrets.
- Explain the user-visible behavior, not only the files changed.
- If a change affects both firmware systems, split it into two PRs or explain the boundary.

## License

- Voice firmware: MIT.
- DuckClaw: CC BY-NC-SA 4.0.
- By contributing, you agree your contribution is provided under the license of the directory you modify.
