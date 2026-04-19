\# Testing Instructions – Fantasy Frontier



\## Build

\- Build the project using existing build scripts or Unreal build system.



\## Packaging

\- Run BuildCookRun for Windows.

\- Ensure the packaged executable launches successfully.



\## Smoke Test Flow

1\. Launch the game

2\. Go Title Screen -> Start

3\. Open Character Creator

4\. Switch Male/Female

5\. Switch Preview Modes (Base Body / Starter Gear / Origin)

6\. Save and Load a Preset

7\. Start Game

8\. Spawn into Tutorial

9\. Move character

10\. Test dash + light/heavy attacks

11\. Interact with at least 1 NPC

12\. Trigger combat with 1 enemy



\## Fail Conditions

\- Crash

\- Character not spawning correctly

\- Creator breaking (mesh missing, wrong gender, etc.)

\- Map looks like default Unreal level

\- Systems not responding



\## If a failure occurs

\- Identify root cause

\- Fix the issue

\- Rebuild

\- Retest



Repeat until all steps pass or a blocker is found.

