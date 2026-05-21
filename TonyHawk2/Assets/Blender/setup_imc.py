"""Configure IMC_Skating key mappings."""
import unreal

def make_key(name):
    k = unreal.Key()
    k.import_text(name)
    return k

imc = unreal.load_asset("/Game/Phase1/Input/IMC_Skating")
ia_move = unreal.load_asset("/Game/Phase1/Input/IA_Move")
ia_ollie = unreal.load_asset("/Game/Phase1/Input/IA_Ollie")
ia_brake = unreal.load_asset("/Game/Phase1/Input/IA_Brake")
ia_switch = unreal.load_asset("/Game/Phase1/Input/IA_SwitchStance")

if not all([imc, ia_move, ia_ollie, ia_brake, ia_switch]):
    print("ERROR: Could not load all input assets")
else:
    mappings = []

    # WASD for IA_Move (Axis2D)
    for key_name in ["W", "S", "A", "D"]:
        m = unreal.EnhancedActionKeyMapping()
        m.set_editor_property("Action", ia_move)
        m.set_editor_property("Key", make_key(key_name))
        mappings.append(m)

    # Gamepad left stick
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("Action", ia_move)
    m.set_editor_property("Key", make_key("Gamepad_LeftX"))
    mappings.append(m)

    # Space for Ollie
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("Action", ia_ollie)
    m.set_editor_property("Key", make_key("SpaceBar"))
    mappings.append(m)

    # Gamepad face button bottom (A/Cross) for Ollie
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("Action", ia_ollie)
    m.set_editor_property("Key", make_key("Gamepad_FaceButton_Bottom"))
    mappings.append(m)

    # Left Shift for Brake
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("Action", ia_brake)
    m.set_editor_property("Key", make_key("LeftShift"))
    mappings.append(m)

    # Tab for Switch Stance
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("Action", ia_switch)
    m.set_editor_property("Key", make_key("Tab"))
    mappings.append(m)

    imc.set_editor_property("Mappings", mappings)
    unreal.EditorAssetLibrary.save_asset("/Game/Phase1/Input/IMC_Skating")
    print(f"IMC_Skating configured with {len(mappings)} mappings")
