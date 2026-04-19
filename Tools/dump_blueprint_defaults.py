import unreal

ASSETS = [
    "/Game/Variant_Combat/Blueprints/BP_CombatCharacter.BP_CombatCharacter_C",
    "/Game/Variant_Combat/Blueprints/BP_CombatGameMode.BP_CombatGameMode_C",
    "/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C",
    "/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController.BP_ThirdPersonPlayerController_C",
    "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C",
    "/Game/Variant_Platforming/Blueprints/BP_PlatformingCharacter.BP_PlatformingCharacter_C",
]

PROPERTIES = [
    "jump_action",
    "move_action",
    "look_action",
    "mouse_look_action",
    "combo_attack_action",
    "charged_attack_action",
    "toggle_camera_action",
    "dash_action",
    "combo_attack_montage",
    "charged_attack_montage",
    "dash_montage",
    "default_mapping_contexts",
    "mobile_excluded_mapping_contexts",
    "default_pawn_class",
    "player_controller_class",
]

OUTPUT_PATH = r"C:\Users\david\Desktop\Fantasy Frontier\Saved\blueprint_defaults.txt"


def main() -> None:
    lines = []
    for asset_path in ASSETS:
        obj = unreal.load_object(None, asset_path)
        lines.append(f"ASSET {asset_path}")
        if not obj:
            lines.append("  <failed to load>")
            continue

        for prop in PROPERTIES:
            try:
                value = obj.get_editor_property(prop)
            except Exception:
                continue
            lines.append(f"  {prop}: {value}")

    with open(OUTPUT_PATH, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))
    unreal.log(f"Wrote blueprint defaults to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
