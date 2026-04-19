# Import QA Rules

A newly imported asset must pass Import QA before gameplay integration.

## Required checks
- asset path resolves
- skeletal mesh exists
- skeleton exists
- physics asset exists if expected
- material slots are assigned
- textures are assigned
- no default/fallback material overrides
- preview in Unreal looks close enough to source

## Fail conditions
- missing mesh
- missing skeleton
- missing materials
- wrong textures
- visible seams caused by wrong setup
- obvious color mismatch caused by wrong material hookup

## Enforcement
If Import QA fails:
- stop integration
- fix import/material setup first
