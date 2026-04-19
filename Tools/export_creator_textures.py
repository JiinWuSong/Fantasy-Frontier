import os
import unreal


EXPORT_ROOT = r"C:\Users\david\Desktop\Fantasy Frontier\Saved\ExportedTextures"
ASSET_PATHS = [
    "/Game/Characters/Mannequins/Textures/Manny/T_Manny_01_D",
    "/Game/Characters/Mannequins/Textures/Manny/T_Manny_02_D",
    "/Game/Characters/Mannequins/Textures/Quinn/T_Quinn_01_D",
    "/Game/Characters/Mannequins/Textures/Quinn/T_Quinn_02_D",
    "/Game/Characters/Mannequins/Textures/Manny/T_Manny_01_MRA",
    "/Game/Characters/Mannequins/Textures/Quinn/T_Quinn_01_MRA",
]


def main():
    os.makedirs(EXPORT_ROOT, exist_ok=True)
    exporter = unreal.TextureExporterPNG()
    for asset_path in ASSET_PATHS:
        texture = unreal.load_asset(asset_path)
        if not texture:
            unreal.log_warning(f"Could not load {asset_path}")
            continue

        filename = os.path.join(EXPORT_ROOT, f"{texture.get_name()}.png")
        task = unreal.AssetExportTask()
        task.object = texture
        task.filename = filename
        task.automated = True
        task.prompt = False
        task.replace_identical = True
        task.exporter = exporter
        task.write_empty_files = False
        unreal.Exporter.run_asset_export_task(task)
        unreal.log(f"Exported {asset_path} -> {filename}")


if __name__ == "__main__":
    main()
