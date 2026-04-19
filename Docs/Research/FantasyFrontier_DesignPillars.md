# Fantasy Frontier Design Pillars

Stand: 2026-04-15

## Zielbild fuer Sprint 1

Sprint 1 baut keinen MMO-Backbone, sondern einen lokal spielbaren Vertical Slice mit:

- Title Screen -> Character Creator -> Tutorial Slice
- einer klar lesbaren Anime-Fantasy-Richtung
- modularer Character-/Equipment-Basis
- einem kleinen, originalen Hidden-Scenario-System
- lesbarer Nahkampf-/Dash-Action
- persistenten Grafikoptionen

## Referenzrahmen

Die folgenden Quellen wurden als Richtungsgeber genutzt, aber nicht kopiert:

- [Shangri-La Frontier Official Anime Site](https://anime.shangrilafrontier.com/en/)
- [Unreal Engine Nanite Documentation](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)
- [UGameUserSettings API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/GameFramework/UGameUserSettings)
- [Lyra Scalability and Device Profiles](https://dev.epicgames.com/documentation/en-us/unreal-engine/scalability-and-device-profiles-in-lyra-sample-game-for-unreal-engine)
- [Fab Marketplace](https://www.fab.com/en-US/)

## Design-Schlussfolgerungen aus der Recherche

### 1. Character Pipeline

FFXIV ist fuer uns vor allem Pipeline-Referenz, nicht Art-Kopie:

- persistenter Base Body unter Gear
- Creator-Preview fuer Base Body / Starter Gear / Origin Style
- Appearance Presets muessen speicherbar bleiben
- Gear muss spaeter modular ueber einen stabilen Body-Layer wachsen

### 2. Shangri-La-Frontier-Gefuehl

Aus der offiziellen Einfuehrung und Charakterbeschreibung wurde fuer unser Originalprojekt abgeleitet:

- keine generische Quest-Symbol-Schleife
- NPCs als Systemtraeger statt nur als Flavor
- Spielidentitaet ueber Exploration, Reaktion, Trigger und Build-Entscheidungen
- starke Lesbarkeit von Bewegung, Danger, Telegraphs und seltenen Triggern

Die konkrete Sprint-1-Umsetzung daraus:

- Guide NPC
- Smith NPC
- Skill-Trainer NPC
- proto Unique Scenario ueber Explorations- + Momentum-Trigger
- Risk/Reward-Modifikator `Swiftstep Covenant`
- Dash -> Heavy Attack als erste Skill-Fusion-Mechanik

### 3. Unreal-Technik

Aus den offiziellen Unreal-Quellen wurde fuer Sprint 1 festgelegt:

- Nanite wird fuer statische Weltgeometrie empfohlen, nicht blind fuer deformierende Character-Pipelines
- Skeletal Meshes bleiben fuer Spieler, NPCs, Feinde und Wildlife ueber klassische LOD-/Rig-Pfade angelegt
- Grafikoptionen laufen ueber `UGameUserSettings` und muessen zwischen Sessions persistent bleiben

## Sinnvolle Unreal/Fab-Asset-Richtung fuer das Projekt

Diese Recherche ist bewusst kuratiert. Nicht jedes gefundene Asset ist fuer Shipping geeignet.

### Direkt nuetzlich fuer schnelle Prototyping-/Loot-Paesse

- [QMS - Fantasy Sword Pack FREE](https://www.fab.com/listings/30e5e745-d601-4d7b-b2da-56f977769638)
  - sinnvoll fuer fruehe Lancer-/Melee-Prototypen
  - 2048er PBR, mehrere Varianten, geringe Tri-Zahlen
- [Free Fantasy Scythe Pack](https://www.fab.com/listings/5be8634a-4679-45bc-bd54-a9e1632790f4)
  - interessant fuer Elite-/Boss-/Dark-Ruin-Enemy-Prototyping
  - explizit High-Poly- und Low-Poly-Versionen
- [GanzSe FREE Weapons - Fantasy Low Poly Pack](https://www.fab.com/listings/8d570eec-44d7-40eb-b02d-c77600146600)
  - nur als Temp-Loot-/Inventory-Prototyp
  - nicht fuer finalen Hero-Look

### Nur als Evaluationskandidaten fuer Sprint 2+

- [Lofty Robot's Customizable Fantasy Environment Kit](https://www.fab.com/listings/2b0e17e1-c71d-4b27-9506-1aa4617c290d)
  - interessant wegen Nanite- und modularen Material-Workflows
  - erst nach Budget-/Lizenzcheck
- [Black Forest PCG Environment](https://www.fab.com/listings/6335039a-21a5-4511-bf61-705c0d02fef2)
  - interessant fuer spaetere PCG-/Traversal-Richtung
  - aktuell nur als Referenzpfad, nicht in Sprint 1 eingebaut

### Character-Asset-Einschaetzung

Es gibt freie Character-Packs auf Fab, aber fuer unser Projekt ist Vorsicht wichtig:

- fremde Hero-Charaktere beschleunigen zwar die Optik
- sie ziehen aber sofort Stil- und IP-Risiken in die Base Pipeline
- fuer Sprint 1 wurde deshalb bewusst eine original ausgerichtete, production-minded Placeholder-Basis auf vorhandenen Epic-Mannequin-Assets + eigenen Preview-Layern gebaut

## Was in Sprint 1 bewusst nicht endlos recherchiert wurde

- kein tiefer Vollabgleich aller Shangri-La-Frontier-Manga-/Anime-Systeme
- kein Marketplace-Shopping ohne sauberen Prototyp-Unterbau
- kein sofortiger Vollersatz der Character-Bodies durch ein externes Tool- oder Fab-Character-Pack

Begruendung:

- der vorhandene Projektstand war bereits nah genug an einem spielbaren Vertical Slice
- Sprint 1 priorisiert einen stabilen Build, Creator-Flow, Tutorial-Loop und System-Hooks

## Sprint-2-Fokus aus der Recherche

- dedizierter Hero-Body-/Face-/Hair-Asset-Pass
- echte modulare Starter-Gear-Meshes statt Preview-Formen
- dedizierte Tutorial-Map-Asset-Datei statt rein runtime-generierter Slice-Struktur
- erweiterte Scenario-Definitionen und Apex-Threat-Archetypen
- saubere Fab-/Eigenproduktions-Mischpipeline fuer Environment, Weapons und NPC-Sets
