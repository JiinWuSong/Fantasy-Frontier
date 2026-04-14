# Fantasy Frontier

`Fantasy Frontier` ist der saubere Unreal-Engine-5.7-Neustart fuer den Fantasy-MMO-Prototypen. Der aktuelle Fokus liegt auf einem starken ersten Eindruck: Startup-Intro, Title Screen und Main Menu mit lebhafter Anime-/Fantasy-Stimmung.

## Aktueller Stand

- Unreal-Project unter `FantasyFrontier.uproject`
- Startup-Movie unter `Content/Movies/FantasyFrontierIntro.mp4`
- prozedural generierter Fantasy-Hintergrund unter `Content/Slate/MenuBackground.png`
- C++/Slate-Frontend mit `Game Start`, `Optionen` und `Spiel verlassen`
- Optionen fuer Fenstermodus und Grafikqualitaet direkt im Frontend

## Struktur

- Projektdatei: `FantasyFrontier.uproject`
- Unreal-Konfiguration: `Config`
- Runtime-Code: `Source/TP_ThirdPerson`
- Frontend-Asset-Skripte: `Tools/Branding`

## Frontend-Flow

1. Beim Start spielt UE5 automatisch das Intro-Video aus `Content/Movies`.
2. Danach erscheint das Frontend ueber dem Projekt-Startlevel.
3. `Game Start` blendet ins spielbare Template-Level ueber.
4. `Optionen` oeffnet Fenstermodus- und Qualitaets-Umschalter.

## Hilfsskripte

Frontend neu rendern:

```powershell
powershell -ExecutionPolicy Bypass -File .\BuildGame.ps1
```

Frontend rendern und Editor bauen:

```powershell
powershell -ExecutionPolicy Bypass -File .\BuildGame.ps1 -BuildEditor
```

## Repository-Workflow

- Feature-Arbeit startet von `dev` in einem eigenen Branch.
- Pull Requests fuer neue Aenderungen sollten gegen `dev` geoeffnet werden.

## Naechste sinnvolle Schritte

- eigene Spielwelt-/Menu-Map statt Template-Level
- Audio, Partikel und Kamerafahrt im Intro weiter ausbauen
- Character-Select und erste Klassenidentitaet
- MMO-relevante Third-Person-Kamera und Netzwerkfundament

Mehr Planungsdetails stehen in `ROADMAP.md`.
