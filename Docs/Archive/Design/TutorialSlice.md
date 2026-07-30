# Tutorial Slice

Stand: 2026-04-15

## Scope

Sprint 1 liefert einen kompakten 10-15-Minuten-Loop:

- Spawn / Safe Hub
- Guide / Smith / Trainer
- Traversal-Korridor
- versteckter Seitenpfad
- kleiner Combat-Arena-Bereich
- proto Hidden Scenario

## Wichtige Dateien

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

## Aktueller Map-Ansatz

Wichtig und ehrlich:

- Sprint 1 verwendet noch `C:\Users\david\Desktop\Fantasy Frontier\Content\ThirdPerson\Lvl_ThirdPerson.umap`
- die Tutorial-Slice-Geometrie und Actor-Platzierung werden zur Laufzeit durch `AFantasyFrontierTutorialDirector` aufgebaut
- `C:\Users\david\Desktop\Fantasy Frontier\Content\FantasyFrontier\Maps\Tutorial\` ist bereits als Zielstruktur angelegt, enthaelt aber in Sprint 1 noch keine voll authorierte dedizierte `.umap`

## Enthaltene Slice-Bereiche

### Safe Hub

- Spielerstart
- Guide NPC
- Smith Setup
- erster ruhiger Raum fuer Orientierung

### Traversal Route

- klar lesbarer Hauptpfad
- vertikale Ruinen- und Baum-Silhouetten
- erster Bewegungsfluss zwischen Hub und Arena

### Hidden Side Path

- sichtbarer, aber nicht plakativer Abzweig
- separates Ruinensignal
- vorbereitet fuer spaetere Scenario Chains

### Combat Arena

- zwei gegensaetzliche Threat-Archetypen
- genug Raum fuer Dodge / Dash / Spacing

## NPCs

### 1. Guide

Rolle:

- fuehrt in die Frontier-Stimmung ein
- gibt den Hinweis fuer den Hidden Scenario Trigger

### 2. Smith

Rolle:

- proto Gear-Upgrader / Crafting Hook
- heilt aktuell den Spieler und markiert die kuenftige Equipment-Station

### 3. Trainer

Rolle:

- schaltet die erste Skill-Fusion frei
- fuehrt den Spieler auf Dash -> Heavy Attack

## Wildlife

### 1. Lumen Moth

- ambient, harmlos
- leuchtend
- frontier/mystic Stimmung

### 2. Mossback Grazer

- passiv
- ruhiger Layer fuer die Welt

## Gegner

### 1. Ridge Stalker

- schneller Predator
- kurze Telegraphs
- aggressiver Nahkampf

### 2. Ruin Mystic

- Distanzgegner
- gut lesbare Cyan-Telegraphs
- Arc-Bolt Projektil

## Unique Scenario Prototype

Datei:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierHiddenScenarioTrigger.cpp`

Aktuelles Verhalten:

- kein Quest-Ausrufer
- braucht Guide + Trainer als Vorbedingungen
- wird ueber Exploration + Momentum ausgelost
- schaltet `Swiftstep Covenant` frei

Reward-Richtung:

- weniger Max-Health
- guenstigeres Dodge-/Stamina-Verhalten
- mehr Build-Identitaet fuer mobile Spielweise

## Deferred nach Sprint 2

- dedizierte Tutorial-Map-Asset-Datei
- persistente NPC-Dialoge / Interact UI
- richtiger Smith-/Gear-Upgrade-Screen
- tierische AI statt ambienter Loop-Bewegung
- weitere Hidden Scenarios und Scenario Chains
- erster Elite-/Apex-Threat im Slice
