# Master Orchestration Rules

Use the Master Orchestrator after:
- a step implementation attempt
- a failed QA result
- a blocked route
- a completed step
- any point where the next action is unclear

The Master Orchestrator decides:
- continue current step
- switch route
- run Asset Scout
- run Tooling Classifier
- run Import QA
- move to next step
- mark blocked
- activate next sprint

The Master Orchestrator must generate exactly one next working prompt.
