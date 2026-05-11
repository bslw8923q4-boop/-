# Ironcliffe UE5 Prototype

Ironcliffe is a dark fantasy action RPG and kingdom-builder prototype based on `Ironcliffe_GDD_v01.docx`.

This repo is now an Unreal Engine 5.7 C++ project built on Epic's Third Person template. The template gives us a playable camera, character, input, combat variant content, and starter maps. The Ironcliffe layer adds the first gameplay backbone for:

- Sirius Vael's stamina, morale, combo, parry, dodge, and tactical order loop.
- The rumor system that brings companions and strangers to the settlement gate.
- Settlement upgrades for the longhouse, barracks, forge, market, stables, sanctuary, infirmary, and chronicler tower.
- Castle-on-the-cliff progression from claimed cliff to coronation.
- Ending pressure for Conqueror, Ally, and Shadow paths.

## Open

Use Unreal Engine 5.7:

```powershell
&"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Admin\OneDrive\Documents\New project 4\Ironcliffe.uproject"
```

## C++ Entry Points

- `Source/Ironcliffe/Combat/IroncliffeCombatComponent.*`
- `Source/Ironcliffe/Systems/IroncliffeWorldSubsystem.*`
- `Source/Ironcliffe/Systems/IroncliffeTypes.h`
- `Source/Ironcliffe/IroncliffeCharacter.*`

## First Blueprint Tasks

- Add Enhanced Input actions for light attack, heavy attack, parry, dodge, and tactical order wheel.
- Create a UMG HUD bound to `UIroncliffeCombatComponent` and `UIroncliffeWorldSubsystem`.
- Place settlement interactables that call `UpgradeBuilding`, `RaidLocation`, `NegotiateLocation`, and `AdvanceCastlePhase`.
- Create Blueprint visual states for each castle phase.

## Current Prototype Controls

- `E`: interact with the nearest labeled Ironcliffe marker.
- `R`: raid Westmir.
- `N`: negotiate with Westmir.
- `B`: upgrade the longhouse.
- `C`: advance the castle phase.
- `T`: pass one day and advance rumors.
- `J`: light attack.
- `K`: heavy attack.
- `Q`: parry.
- `Left Shift`: dodge.
- `1-4`: tactical orders.
