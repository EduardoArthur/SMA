# -*- coding: utf-8 -*-
"""
Neon Stack - setup automático do editor (UE 5.x).

Cria, em /Game/NeonStack:
  Data/      DA_GameConfig, DA_Palettes
  Input/     8 Input Actions + IMC_Gameplay + IMC_UI
  Art/Materials/  MPC_NeonStack, M_Block, M_Ghost, M_SynthSky, M_SynthSun, M_SynthFloor, M_Well
  Maps/      L_Game, L_MainMenu (câmera ortográfica, fundo synthwave, post process, atores do jogo)

Como rodar: Tools > Execute Python Script... > escolha este arquivo
            (ou, no Output Log, mude o combo para "Python" e digite:  py "<caminho>/Scripts/setup_neon_stack.py")

É seguro rodar de novo: o que já existe NÃO é sobrescrito (apague o asset para recriá-lo).
Requer os plugins "Python Editor Script Plugin" e "Editor Scripting Utilities" (já habilitados no .uproject).
"""

import unreal

ROOT = "/Game/NeonStack"
DATA = ROOT + "/Data"
INPUT = ROOT + "/Input"
MATS = ROOT + "/Art/Materials"
MAPS = ROOT + "/Maps"

tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary

created_count = 0
warnings = []


def log(msg):
    unreal.log("[NeonStack] " + msg)


def warn(msg):
    warnings.append(msg)
    unreal.log_warning("[NeonStack] " + msg)


def get_or_create(name, folder, asset_class, factory):
    """Retorna (asset, criado_agora). Nunca sobrescreve um asset existente."""
    global created_count
    path = folder + "/" + name
    if eal.does_asset_exist(path):
        log("já existe: " + path)
        return eal.load_asset(path), False
    asset = tools.create_asset(name, folder, asset_class, factory)
    if asset is None:
        warn("falhou ao criar " + path)
        return None, False
    created_count += 1
    log("criado: " + path)
    return asset, True


def save(asset):
    if asset is not None:
        eal.save_loaded_asset(asset)


# ---------------------------------------------------------------------------
# 1. Data Assets
# ---------------------------------------------------------------------------
def make_data_assets():
    for name, cls in (("DA_GameConfig", unreal.NSGameConfig), ("DA_Palettes", unreal.NSPaletteData)):
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", cls.static_class())
        asset, _ = get_or_create(name, DATA, cls, factory)
        save(asset)


# ---------------------------------------------------------------------------
# 2. Input (GDD §4.3 / m1-setup-guide §6)
# ---------------------------------------------------------------------------
GAMEPLAY_KEYS = {
    "IA_MoveLeft": ["Left"],
    "IA_MoveRight": ["Right"],
    "IA_SoftDrop": ["Down"],
    "IA_HardDrop": ["SpaceBar"],
    "IA_RotateCW": ["Up", "X"],
    "IA_RotateCCW": ["Z"],
    "IA_Hold": ["C", "LeftShift"],
    "IA_Pause": ["Escape", "P"],
}


def make_key(name):
    key = unreal.Key()
    key.set_editor_property("key_name", name)
    return key


def make_input():
    actions = {}
    for name in GAMEPLAY_KEYS:
        ia, created = get_or_create(name, INPUT, unreal.InputAction, unreal.InputAction_Factory())
        if ia is None:
            continue
        if created:
            ia.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
            save(ia)
        actions[name] = ia

    imc, created = get_or_create("IMC_Gameplay", INPUT, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    if imc is not None and created:
        for name, keys in GAMEPLAY_KEYS.items():
            for k in keys:
                imc.map_key(actions[name], make_key(k))
        save(imc)

    imc_ui, created = get_or_create("IMC_UI", INPUT, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    if imc_ui is not None and created:
        for k in GAMEPLAY_KEYS["IA_Pause"]:
            imc_ui.map_key(actions["IA_Pause"], make_key(k))
        save(imc_ui)


# ---------------------------------------------------------------------------
# 3. Material Parameter Collection (m4-m5-setup-guide §2)
# ---------------------------------------------------------------------------
MPC_VECTORS = {
    "SkyTop": unreal.LinearColor(0.006, 0.0, 0.024, 1.0),
    "SkyHorizon": unreal.LinearColor(0.1, 0.006, 0.11, 1.0),
    "SunTop": unreal.LinearColor(1.0, 0.2, 0.05, 1.0),
    "SunBottom": unreal.LinearColor(1.0, 0.03, 0.25, 1.0),
    "GridColor": unreal.LinearColor(1.0, 0.03, 0.25, 1.0),
    "AccentColor": unreal.LinearColor(1.0, 0.05, 0.6, 1.0),
}
MPC_SCALARS = ["FloorOffset", "ComboGlow", "SunPulse", "Flash"]


def make_mpc():
    mpc, created = get_or_create("MPC_NeonStack", MATS, unreal.MaterialParameterCollection,
                                 unreal.MaterialParameterCollectionFactoryNew())
    if mpc is not None and created:
        scalars = []
        for name in MPC_SCALARS:
            p = unreal.CollectionScalarParameter()
            p.set_editor_property("parameter_name", name)
            p.set_editor_property("default_value", 0.0)
            scalars.append(p)
        vectors = []
        for name, color in MPC_VECTORS.items():
            p = unreal.CollectionVectorParameter()
            p.set_editor_property("parameter_name", name)
            p.set_editor_property("default_value", color)
            vectors.append(p)
        mpc.set_editor_property("scalar_parameters", scalars)
        mpc.set_editor_property("vector_parameters", vectors)
        save(mpc)
    return mpc


# ---------------------------------------------------------------------------
# 4. Materiais (Unlit + nós Custom em HLSL)
# ---------------------------------------------------------------------------
def new_material(name, blend=None, ism=False):
    mat, created = get_or_create(name, MATS, unreal.Material, unreal.MaterialFactoryNew())
    if mat is None or not created:
        return None
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    if blend is not None:
        mat.set_editor_property("blend_mode", blend)
    if ism:
        mat.set_editor_property("used_with_instanced_static_meshes", True)
    return mat


def custom_node(mat, code, inputs, out_type, x=-400, y=0):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionCustom, x, y)
    node.set_editor_property("code", code)
    node.set_editor_property("output_type", out_type)
    pins = []
    for n in inputs:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", n)
        pins.append(ci)
    node.set_editor_property("inputs", pins)
    return node


def mpc_node(mat, mpc, name, x, y):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionCollectionParameter, x, y)
    node.set_editor_property("collection", mpc)
    node.set_editor_property("parameter_name", name)
    return node


def custom_data_node(mat, index, x, y):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionPerInstanceCustomData, x, y)
    node.set_editor_property("data_index", index)
    return node


def custom_data_rgb(mat, x, y):
    """RGB dos índices 0-2. Usa o nó 3Vector se existir; senão, junta 3 escalares com Append."""
    if hasattr(unreal, "MaterialExpressionPerInstanceCustomData3Vector"):
        node = mel.create_material_expression(mat, unreal.MaterialExpressionPerInstanceCustomData3Vector, x, y)
        node.set_editor_property("data_index", 0)
        return node
    r = custom_data_node(mat, 0, x - 300, y)
    g = custom_data_node(mat, 1, x - 300, y + 80)
    b = custom_data_node(mat, 2, x - 300, y + 160)
    rg = mel.create_material_expression(mat, unreal.MaterialExpressionAppendVector, x - 150, y)
    mel.connect_material_expressions(r, "", rg, "A")
    mel.connect_material_expressions(g, "", rg, "B")
    rgb = mel.create_material_expression(mat, unreal.MaterialExpressionAppendVector, x, y)
    mel.connect_material_expressions(rg, "", rgb, "A")
    mel.connect_material_expressions(b, "", rgb, "B")
    return rgb


def world_pos(mat, x, y):
    return mel.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, x, y)


def wire(src, dst, pin):
    if not mel.connect_material_expressions(src, "", dst, pin):
        warn("não conectou o pino " + pin)


def finish(mat, name):
    mel.layout_material_expressions(mat)
    mel.recompile_material(mat)
    save(mat)
    log("material pronto: " + name)


BLOCK_EDGE = """
float e = min(min(UV.x, 1.0 - UV.x), min(UV.y, 1.0 - UV.y));
float edge = 1.0 - smoothstep(0.05, 0.11, e);
"""


def make_block_material(mpc):
    mat = new_material("M_Block", ism=True)
    if mat is None:
        return
    code = BLOCK_EDGE + """
return C * (0.12 + edge * 5.0 * B) + C * P * 8.0 + Flash * 1.5;
"""
    node = custom_node(mat, code, ["C", "B", "P", "Flash", "UV"], unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    wire(custom_data_rgb(mat, -900, -200), node, "C")
    wire(custom_data_node(mat, 4, -900, 0), node, "B")
    wire(custom_data_node(mat, 5, -900, 100), node, "P")
    wire(mpc_node(mat, mpc, "Flash", -900, 200), node, "Flash")
    wire(mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 300), node, "UV")
    mel.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(mat, "M_Block")


def make_ghost_material():
    mat = new_material("M_Ghost", blend=unreal.BlendMode.BLEND_TRANSLUCENT, ism=True)
    if mat is None:
        return
    emissive = custom_node(mat, BLOCK_EDGE + "return C * edge * 2.5;", ["C", "UV"],
                           unreal.CustomMaterialOutputType.CMOT_FLOAT3, -400, -100)
    opacity = custom_node(mat, BLOCK_EDGE + "return edge * A;", ["UV", "A"],
                          unreal.CustomMaterialOutputType.CMOT_FLOAT1, -400, 150)
    rgb = custom_data_rgb(mat, -900, -200)
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 100)
    wire(rgb, emissive, "C")
    wire(uv, emissive, "UV")
    wire(uv, opacity, "UV")
    wire(custom_data_node(mat, 6, -900, 250), opacity, "A")
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    finish(mat, "M_Ghost")


def make_sky_material(mpc):
    mat = new_material("M_SynthSky")
    if mat is None:
        return
    code = """
float t = saturate((WP.z - 900.0) / 1500.0);
float3 col = lerp(Hor, Top, t);
float2 g = floor(WP.yz / 40.0);
float h = frac(sin(dot(g, float2(12.9898, 78.233))) * 43758.5453);
float star = step(0.997, h) * t;
return col + star * 0.8 + Flash * 0.3;
"""
    node = custom_node(mat, code, ["WP", "Top", "Hor", "Flash"], unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    wire(world_pos(mat, -900, -150), node, "WP")
    wire(mpc_node(mat, mpc, "SkyTop", -900, 0), node, "Top")
    wire(mpc_node(mat, mpc, "SkyHorizon", -900, 100), node, "Hor")
    wire(mpc_node(mat, mpc, "Flash", -900, 200), node, "Flash")
    mel.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(mat, "M_SynthSky")


SUN_SHAPE = """
float2 p = WP.yz - float2(500.0, 1500.0);
float r = length(p) / (800.0 * (1.0 + Pulse * 0.05));
"""


def make_sun_material(mpc):
    mat = new_material("M_SynthSun", blend=unreal.BlendMode.BLEND_MASKED)
    if mat is None:
        return
    color_code = SUN_SHAPE + """
float v = saturate((p.y + 800.0) / 1600.0);
return lerp(Bot, Top, v) * (1.5 + Pulse * 2.0);
"""
    mask_code = SUN_SHAPE + """
float disk = 1.0 - step(1.0, r);
float below = saturate(-p.y / 800.0);
float stripes = step(below * 0.6, frac(p.y / 70.0));
return disk * (p.y > 0.0 ? 1.0 : stripes);
"""
    color = custom_node(mat, color_code, ["WP", "Top", "Bot", "Pulse"], unreal.CustomMaterialOutputType.CMOT_FLOAT3, -400, -100)
    mask = custom_node(mat, mask_code, ["WP", "Pulse"], unreal.CustomMaterialOutputType.CMOT_FLOAT1, -400, 150)
    wp = world_pos(mat, -900, -150)
    pulse = mpc_node(mat, mpc, "SunPulse", -900, 200)
    wire(wp, color, "WP")
    wire(wp, mask, "WP")
    wire(mpc_node(mat, mpc, "SunTop", -900, 0), color, "Top")
    wire(mpc_node(mat, mpc, "SunBottom", -900, 100), color, "Bot")
    wire(pulse, color, "Pulse")
    wire(pulse, mask, "Pulse")
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(mask, "", unreal.MaterialProperty.MP_OPACITY_MASK)
    finish(mat, "M_SynthSun")


def make_floor_material(mpc):
    mat = new_material("M_SynthFloor")
    if mat is None:
        return
    code = """
float d = saturate((1000.0 - WP.z) / 1300.0);
float u = (WP.y - 500.0) / (d + 0.02) / 300.0;
float v = 1.0 / (d + 0.08) * 0.6 + Offset;
float lu = abs(frac(u) - 0.5);
float lv = abs(frac(v) - 0.5);
float w = 0.04;
float lines = max(step(0.5 - w, lu), step(0.5 - w * (1.0 + d * 2.0), lv));
float fade = saturate(d * 3.0);
float3 baseCol = float3(0.01, 0.0, 0.02);
return baseCol + Grid * lines * fade * (1.5 + Glow * 3.0) + Flash * 0.2;
"""
    node = custom_node(mat, code, ["WP", "Grid", "Offset", "Glow", "Flash"], unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    wire(world_pos(mat, -900, -200), node, "WP")
    wire(mpc_node(mat, mpc, "GridColor", -900, -50), node, "Grid")
    wire(mpc_node(mat, mpc, "FloorOffset", -900, 50), node, "Offset")
    wire(mpc_node(mat, mpc, "ComboGlow", -900, 150), node, "Glow")
    wire(mpc_node(mat, mpc, "Flash", -900, 250), node, "Flash")
    mel.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(mat, "M_SynthFloor")


def make_well_material(mpc):
    mat = new_material("M_Well")
    if mat is None:
        return
    code = """
float2 c = frac(WP.yz / 100.0);
float g = step(0.96, max(c.x, c.y));
return float3(0.02, 0.016, 0.04) + Grid * g * 0.08 + Flash * 0.1;
"""
    node = custom_node(mat, code, ["WP", "Grid", "Flash"], unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    wire(world_pos(mat, -900, -100), node, "WP")
    wire(mpc_node(mat, mpc, "GridColor", -900, 50), node, "Grid")
    wire(mpc_node(mat, mpc, "Flash", -900, 150), node, "Flash")
    mel.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    finish(mat, "M_Well")


# ---------------------------------------------------------------------------
# 5. Mapas
# ---------------------------------------------------------------------------
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)


def spawn(cls, loc=(0.0, 0.0, 0.0), label=None):
    actor = eas.spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(roll=0.0, pitch=0.0, yaw=0.0))
    if actor is not None and label:
        actor.set_actor_label(label)
    return actor


def spawn_quad(label, mat_name, x, y, z, width, height):
    """Placa fina (cubo achatado em X) virada para a câmera, que olha para +X."""
    cube = unreal.load_asset("/Engine/BasicShapes/Cube")
    actor = eas.spawn_actor_from_object(cube, unreal.Vector(x, y, z))
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(0.01, width / 100.0, height / 100.0))
    smc = actor.get_editor_property("static_mesh_component")
    mat = eal.load_asset(MATS + "/" + mat_name)
    if mat is not None:
        smc.set_material(0, mat)
    else:
        warn("material não encontrado: " + mat_name)
    smc.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    smc.set_editor_property("cast_shadow", False)
    return actor


def set_if(obj, prop, value):
    try:
        obj.set_editor_property(prop, value)
        return True
    except Exception as exc:  # propriedade pode não existir nesta versão da engine
        warn("não ajustou %s: %s" % (prop, exc))
        return False


def spawn_camera():
    cam = spawn(unreal.CameraActor, (-1500.0, 500.0, 1000.0), "Camera")
    comp = cam.get_editor_property("camera_component")
    comp.set_editor_property("projection_mode", unreal.CameraProjectionMode.ORTHOGRAPHIC)
    comp.set_editor_property("ortho_width", 4400.0)
    comp.set_editor_property("constrain_aspect_ratio", False)
    if hasattr(comp, "auto_calculate_ortho_planes"):
        set_if(comp, "auto_calculate_ortho_planes", False)
    set_if(comp, "ortho_near_clip_plane", 0.0)
    set_if(comp, "ortho_far_clip_plane", 20000.0)
    cam.set_editor_property("auto_activate_for_player", unreal.AutoReceiveInput.PLAYER0)
    return cam


def spawn_post_process():
    vol = spawn(unreal.PostProcessVolume, (0.0, 0.0, 0.0), "PostProcess")
    vol.set_editor_property("unbound", True)
    s = vol.get_editor_property("settings")

    def override(name, value):
        if set_if(s, "override_" + name, True):
            set_if(s, name, value)

    override("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
    override("auto_exposure_apply_physical_camera_exposure", False)
    override("auto_exposure_bias", 0.0)
    override("bloom_intensity", 1.2)
    override("bloom_threshold", 1.0)
    override("motion_blur_amount", 0.0)
    override("vignette_intensity", 0.4)
    override("film_grain_intensity", 0.05)
    override("scene_fringe_intensity", 0.1)
    vol.set_editor_property("settings", s)
    return vol


def spawn_backdrop():
    # Vista ortográfica: Y de -1700 a 2700, Z de -240 a 2240 (câmera em Y=500, Z=1000, largura 4400)
    spawn_quad("Sky", "M_SynthSky", 4000.0, 500.0, 1000.0, 6000.0, 4000.0)
    spawn_quad("Sun", "M_SynthSun", 3500.0, 500.0, 1500.0, 1700.0, 1700.0)
    spawn_quad("Floor", "M_SynthFloor", 3000.0, 500.0, 350.0, 6000.0, 1300.0)


def set_game_mode(cls):
    world = ues.get_editor_world()
    ws = world.get_world_settings()
    ws.set_editor_property("default_game_mode", cls.static_class())


def make_level(path, build):
    global created_count
    if eal.does_asset_exist(path):
        log("já existe: " + path + " (apague para recriar)")
        return
    if not les.new_level(path):
        warn("não criou o mapa " + path)
        return
    build()
    les.save_current_level()
    created_count += 1
    log("mapa criado: " + path)


def build_game_level():
    spawn_camera()
    pp = spawn_post_process()
    spawn_backdrop()
    spawn_quad("WellBackground", "M_Well", 60.0, 500.0, 1000.0, 1000.0, 2000.0)
    spawn(unreal.NSBoardActor, (0.0, 0.0, 0.0), "Board")
    spawn(unreal.NSBackgroundActor, (0.0, 0.0, 0.0), "Background")
    fx = spawn(unreal.NSFXDirector, (0.0, 0.0, 0.0), "FXDirector")
    fx.set_editor_property("post_process", pp)
    spawn(unreal.NSAudioDirector, (0.0, 0.0, 0.0), "AudioDirector")
    set_game_mode(unreal.NSGameMode)


def build_menu_level():
    spawn_camera()
    spawn_post_process()
    spawn_backdrop()
    spawn(unreal.NSBackgroundActor, (0.0, 0.0, 0.0), "Background")
    spawn(unreal.NSAudioDirector, (0.0, 0.0, 0.0), "AudioDirector")
    set_game_mode(unreal.NSMenuGameMode)


# ---------------------------------------------------------------------------
# Execução
# ---------------------------------------------------------------------------
def main():
    for folder in (DATA, INPUT, MATS, MAPS):
        eal.make_directory(folder)

    make_data_assets()
    make_input()
    mpc = make_mpc()
    if mpc is not None:
        make_block_material(mpc)
        make_ghost_material()
        make_sky_material(mpc)
        make_sun_material(mpc)
        make_floor_material(mpc)
        make_well_material(mpc)
    else:
        warn("sem MPC: materiais não criados")

    make_level(MAPS + "/L_MainMenu", build_menu_level)
    make_level(MAPS + "/L_Game", build_game_level)

    log("=" * 60)
    log("Setup concluído: %d itens criados, %d avisos." % (created_count, len(warnings)))
    for w in warnings:
        log("  aviso: " + w)
    log("Abra Maps/L_MainMenu e aperte Play.")


main()
