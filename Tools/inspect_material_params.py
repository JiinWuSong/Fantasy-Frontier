import unreal
lines = []
assets = [
    '/Game/Character/Materials/MI_Female_Body',
    '/Game/Character/Materials/M_Male_Body',
    '/Game/Characters/Mannequins/Materials/Manny/MI_Manny_01_New',
    '/Game/Characters/Mannequins/Materials/Manny/MI_Manny_02_New',
    '/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_01',
    '/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_02'
]
for path in assets:
    asset = unreal.load_asset(path)
    lines.append(f'ASSET {path} {asset}')
    try:
        lines.append('SCALAR ' + ','.join([str(x) for x in unreal.MaterialEditingLibrary.get_scalar_parameter_names(asset)]))
        lines.append('VECTOR ' + ','.join([str(x) for x in unreal.MaterialEditingLibrary.get_vector_parameter_names(asset)]))
        lines.append('TEXTURE ' + ','.join([str(x) for x in unreal.MaterialEditingLibrary.get_texture_parameter_names(asset)]))
    except Exception as exc:
        lines.append('ERR ' + str(exc))
with open(r'C:\Users\david\Desktop\Fantasy Frontier\Saved\material_params.txt', 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines))
