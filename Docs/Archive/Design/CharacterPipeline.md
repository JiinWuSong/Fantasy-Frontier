# Character Pipeline

Stand: 2026-04-15

## Ziel

Der Character-Flow soll spaeter ein echtes MMORPG-Pipeline-Rueckgrat bilden. Sprint 1 liefert dafuer die erste produktionsnahe Basis:

- Race -> Gender -> Customization -> Identity
- Preview fuer Base Body / Starter Gear / Origin Style
- Appearance Preset Save/Load
- Gameplay-Spawn mit uebertragenem Draft

## Implementiert

### 1. Draft-Datenmodell

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorTypes.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierVerticalSliceTypes.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierAppearancePresetSaveGame.h`

Aktive Hooks:

- `Race`
- `Gender`
- `HeightScale`
- `Build`
- `Musculature`
- `SkinTone`
- `ScarIntensity`
- `TattooIntensity`
- `HairStyle`
- `HairColor`
- `FaceVariant`
- `EyeColor`
- `PreviewMode`
- `OriginStyle`
- `CharacterName`
- `CharacterClass`

### 2. Character Creator UI

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorWidget.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterCreatorWidget.cpp`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\TP_ThirdPersonPlayerController.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\TP_ThirdPersonPlayerController.cpp`

Aktive Creator-Funktionen:

- Race Rail (aktuell live nur `Human`)
- Male/Female Auswahl
- Preview Rotation
- Preview Zoom
- Body-Slider
- Skin / Scar / Tattoo Controls
- Hair / Face Switching
- Preview Modes
- Preset Save / Load

### 3. Preview Actor

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterPreviewActor.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierCharacterPreviewActor.cpp`

Sprint-1-Ansatz:

- verwendet vorhandene Epic-Mannequin-basierte Skeletal Meshes als stabile Gameplay-/Preview-Basis
- zeigt eigene Preview-Layer fuer:
  - Underwear/Base Body
  - Starter Gear
  - Origin Style
  - Tattoo/Scar Marker
  - Hair/Face Accent Hooks

### 4. Gameplay-Anwendung des Drafts

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierPlayableCharacter.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierPlayableCharacter.cpp`

Der Creator-Draft wird beim Bestaetigen auf den Spielcharakter angewendet:

- male/female mesh switch
- skalierte Capsule
- vorbereitete Hook-Struktur fuer spaetere modulare Gear-Layer

## Production-minded Placeholder Status

### Ehrlich fertig

- Creator-Flow
- Preview-Mode-Umschaltung
- Save/Load Presets
- Gameplay-uebergabe
- skalierbare Datenstruktur fuer Gear-/Origin-/Scenario-Hooks

### Noch Placeholder

- Hero-Meshes sind noch keine final neu modellierten Original-Heroes
- Hair ist noch ein modularer Preview-Layer und kein finaler Groom-/Card-Pass
- Tattoos/Scars laufen aktuell ueber Preview-Marker statt finalen Body-Decals oder Materialmasken
- Starter Gear wird im Creator bereits lesbar gezeigt, ist aber noch kein final authoriertes Outfit-Set

## Asset-Pfade, die aktuell als Basis dienen

- `C:\Users\david\Desktop\Fantasy Frontier\Content\Character\Mesh\SK_Mannequin.uasset`
- `C:\Users\david\Desktop\Fantasy Frontier\Content\Character\Mesh\SK_Mannequin_Female.uasset`
- `C:\Users\david\Desktop\Fantasy Frontier\Content\Character\Materials\M_Male_Body.uasset`
- `C:\Users\david\Desktop\Fantasy Frontier\Content\Character\Materials\MI_Female_Body.uasset`
- `C:\Users\david\Desktop\Fantasy Frontier\Content\Characters\Mannequins\Meshes\SKM_Manny_Simple.uasset`
- `C:\Users\david\Desktop\Fantasy Frontier\Content\Characters\Mannequins\Meshes\SKM_Quinn_Simple.uasset`

## Deferred nach Sprint 2

- final original sculpted male/female hero bodies
- saubere authorierte fantasy underwear meshes/materials
- echte eyebrow / eyelash / hairstyle modular sets
- face presets ueber Head-/Face-Mesh-Varianten statt nur leichter Preview-Variation
- runtime gear-equipping ueber richtige modular skeletal parts
- dedizierte origin/race body-part hooks fuer ears / horns / stag features / draconic features
