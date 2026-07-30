# Sprint 1 Vertical Slice Changelog

Stand: 2026-04-15

## Neu implementiert

### Frontend / Options

- erweiterte Options-Controls fuer:
  - Overall Quality
  - Window Mode
  - Shadows
  - Anti-Aliasing
  - Post Process
  - View Distance
- Persistenz ueber `UGameUserSettings`

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierFrontEndWidget.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierFrontEndWidget.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\TP_ThirdPersonPlayerController.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\TP_ThirdPersonPlayerController.cpp`

### Character Creator / Presets

- Creator-Draft erweitert
- Preview Mode Toggle
- Save / Load Appearance Presets
- Male/Female stabil uebertragen
- Preview Rotation + Zoom

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorTypes.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorWidget.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorWidget.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierAppearancePresetSaveGame.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterPreviewActor.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterPreviewActor.cpp`

### Combat / Movement Readability

- stamina layer
- dodge / dash
- fusion hook:
  - Dash -> Heavy Attack
- risk/reward modifier:
  - `Swiftstep Covenant`

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierPlayableCharacter.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierPlayableCharacter.cpp`

### Tutorial Slice Runtime

- sprint-spezifischer GameMode / PlayerController
- runtime-built hub / path / ruins / hidden path / arena
- 3 funktionale NPCs
- 2 Wildlife-Archetypen
- 2 Enemy-Archetypen
- Hidden Scenario Trigger

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierSprint1GameMode.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierSprint1GameMode.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierSprint1PlayerController.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierSprint1PlayerController.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierTutorialDirector.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierTutorialDirector.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierFunctionalNpc.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierFunctionalNpc.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierAmbientCreature.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierAmbientCreature.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierEnemyBase.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierEnemyBase.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierStalkerEnemy.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierStalkerEnemy.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierRuinMysticEnemy.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierRuinMysticEnemy.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierArcBoltProjectile.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierArcBoltProjectile.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierHiddenScenarioTrigger.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierHiddenScenarioTrigger.cpp`

## Konfigurationsaenderungen

- `C:\Users\david\Desktop\Fantasy Frontier\Config\DefaultEngine.ini`
  - Standard-GameMode auf Sprint-1-Flow gesetzt
- `C:\Users\david\Desktop\Fantasy Frontier\Config\DefaultGame.ini`
  - Cook-Pfade auf lokale Projektinhalte umgestellt
- `C:\Users\david\Desktop\Fantasy Frontier\FantasyFrontier.uproject`
  - MetaHumanCharacter Plugin deaktiviert, um Packaging-/Cook-Risiken zu reduzieren

## Input Runtime / Packaging

Stand 2026-04-16:

- der beabsichtigte Runtime-Pfad fuer Core-Gameplay-Input laeuft ueber Enhanced Input Assets, nicht ueber Legacy-Keybind-Fallback
- die vier Core-InputActions muessen physisch unter `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\Actions\` liegen, weil ihre Package-Namen und die Mapping Contexts auf `/Game/Input/Actions/...` zeigen
- relevante Runtime-Datei:
  - `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierPlayableCharacter.cpp`
- relevante Content-Pfade:
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\Actions\IA_Jump.uasset`
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\Actions\IA_Move.uasset`
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\Actions\IA_Look.uasset`
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\Actions\IA_MouseLook.uasset`
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\IMC_Default.uasset`
  - `C:\Users\david\Desktop\Fantasy Frontier\Content\Input\IMC_MouseLook.uasset`
- der Legacy-Input bleibt nur als Safety-Layer aktiv, falls Enhanced Input Assets im Editor oder in einem defekten Build fehlen
- der Paket-Build gilt erst dann als sauber, wenn keine fehlenden `/Game/Input/IA_*`-Cook-/Load-Warnungen mehr auftauchen und der Player-Log bestaetigt, dass Enhanced Input aktiv gebunden wurde

## Production-minded Placeholder

- Hero Bodies sind noch keine final neu produzierten Original-Sculpts
- Tutorial Slice ist noch runtime-generiert statt final authoriertem Map-Asset
- Starter Gear Preview ist strukturell vorhanden, aber noch kein finaler Outfit-Pass
- NPC-/Wildlife-Visuals sind klar lesbar, aber stilistisch noch Blockout-nah

## Deferred nach Sprint 2

- dedizierte Tutorial-Map in `Content/FantasyFrontier/Maps/Tutorial/`
- final hero male / female meshes, hair, face variants
- echte modular equipbare Starter-Sets
- persistente Scenario-/Progression-Speicherung
- weitere Main Job / Sub Job Tabellen
- erster Elite-/World-Threat Hook
