import unreal
assets = [
    '/Game/Character/Mesh/SK_Mannequin',
    '/Game/Character/Mesh/SK_Mannequin_Female',
    '/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple',
    '/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple',
]
lines = []
for path in assets:
    asset = unreal.load_asset(path)
    lines.append(f'ASSET {path}: {asset}')
    if not asset:
        continue
    try:
        mats = asset.get_editor_property('materials')
        lines.append('  MATERIALS ' + ', '.join([str(m.material_slot_name) + '=' + str(m.material_interface) for m in mats]))
    except Exception as exc:
        lines.append('  MATERIALS_ERR ' + str(exc))
    try:
        skel = asset.get_editor_property('skeleton')
        lines.append('  SKELETON ' + str(skel))
    except Exception as exc:
        lines.append('  SKELETON_ERR ' + str(exc))
    try:
        imported = asset.get_editor_property('imported_bounds')
        lines.append('  BOUNDS ' + str(imported.box_extent) + ' ' + str(imported.origin))
    except Exception as exc:
        lines.append('  BOUNDS_ERR ' + str(exc))
out = r'C:\Users\david\Desktop\Fantasy Frontier\Saved\mesh_info.txt'
with open(out, 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines))
print(out)
