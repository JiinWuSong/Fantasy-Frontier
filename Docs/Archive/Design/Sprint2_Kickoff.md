# Sprint 2 Kickoff

Stand: 2026-04-16

## Ziel fuer den Start

Sprint 2 startet auf dem jetzt stabil paketierbaren Sprint-1-Slice und verschiebt den Fokus von reiner Laufzeit-Stabilitaet auf datengetriebene Erweiterbarkeit.

Der erste Schritt ist bewusst risikoarm:

- keine Regression im spielbaren Paket-Build
- keine neue Placeholder-Logik im Kern-Flow
- Vorbereitung fuer Main Job / Sub Job / Hidden Scenario Ausbau

## In diesem Kickoff bereits angelegt

### 1. Data-driven Job Hooks

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierJobDefinition.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierJobDefinition.cpp`

Aktueller Zweck:

- Primary-Asset-Basis fuer spaetere Main Jobs / Sub Jobs / Life Skills
- Basisstats pro Job
- Risk/Reward-Modifier als Datenstruktur
- vorbereitete Referenzkette fuer empfohlene Sub Jobs

### 2. Data-driven Scenario Hooks

Dateien:

- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierScenarioDefinition.h`
- `C:\Users\david\Desktop\Fantasy Frontier\Source\TP_ThirdPerson\FantasyFrontierScenarioDefinition.cpp`

Aktueller Zweck:

- Primary-Asset-Basis fuer Hidden Scenarios und spaetere Scenario Chains
- Trigger-Bedingungen als Datenstruktur
- Reward-Definitionen fuer System Unlocks / Gear Hooks / Modifier / Lore
- UClass: `UFantasyFrontierScenarioDataAsset`

## Warum genau dieser Start

- passt direkt zur Shangri-La-Frontier-inspirierten Systemrichtung
- blockiert weder Character Creator noch Tutorial Slice
- ist production-minded und spaeter sauber mit Data Assets im Editor befuellbar
- schafft eine stabile Grundlage fuer Sprint-2-Inhalte, ohne den laufenden Paket-Build zu destabilisieren

## Naechste sinnvolle Sprint-2-Schritte

1. Erste echte Data Assets fuer `Lancer`, `Trailsmith` oder `Field Scholar` anlegen
2. `Swiftstep Covenant` vom Hardcode in ein `UFantasyFrontierScenarioDefinition`-Asset ueberfuehren
3. Creator-/Spawn-Flow an `UFantasyFrontierJobDefinition` anbinden
4. dedizierte Tutorial-Map authorieren statt reinem Runtime-Build
