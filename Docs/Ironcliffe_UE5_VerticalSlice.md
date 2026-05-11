# Ironcliffe UE5 Vertical Slice

## Player Fantasy

The player is Sirius Vael: a white knight chasing a crown on an unreachable cliff. The first slice should sell the price of ambition, not just the castle fantasy.

## Slice Goal

One playable region with:

- Third-person movement and camera.
- Light attack, heavy attack, parry, dodge, stamina, and army morale.
- A first controllable location such as Westmir.
- A settlement hub with several upgradeable buildings.
- A rumor gate that can attract Oren Gray, Marit Smoke, Brother Keor, or The Unknown.
- Castle phase progression from cliff claim to foundation.

## Core Loop

1. Leave settlement.
2. Raid or negotiate with a location.
3. Return with resources and reputation.
4. Upgrade settlement or castle.
5. Rumors attract visitors.
6. Accept, reject, or betray companions.
7. Ending pressure shifts toward Conqueror, Ally, or Shadow.

## UE5 Implementation Notes

- Keep systemic state in `UIroncliffeWorldSubsystem`.
- Keep combat state in `UIroncliffeCombatComponent`.
- Let Blueprints handle animation montages, VFX, UI, and map dressing.
- Use C++ functions as stable gameplay verbs exposed to UMG and interactables.
- Keep StateTree for enemy and companion behavior as the combat slice grows.

## Next Production Pass

- Build a custom `BP_SiriusVael` child of `AIroncliffeCharacter`.
- Add `IA_LightAttack`, `IA_HeavyAttack`, `IA_Parry`, `IA_Dodge`, and `IA_CommandWheel`.
- Add `WBP_IroncliffeHUD` with health, stamina, morale, resources, rumor visitors, and castle phase.
- Add settlement actors for each building and a cliff actor for castle phase visuals.
- Convert the copied Third Person map into `Lvl_Ironcliffe_Prototype`.
