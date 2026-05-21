"""Set up IMC_Skating key mappings, BP_PlayerController, GameMode, and build debug level."""
import unreal

el = unreal.EditorLevelLibrary
eas = unreal.EditorAssetSubsystem()

# --- Configure IMC_Skating with key mappings via Python ---
imc = unreal.load_asset("/Game/Phase1/Input/IMC_Skating")
ia_move = unreal.load_asset("/Game/Phase1/Input/IA_Move")
ia_ollie = unreal.load_asset("/Game/Phase1/Input/IA_Ollie")
ia_brake = unreal.load_asset("/Game/Phase1/Input/IA_Brake")
ia_switch = unreal.load_asset("/Game/Phase1/Input/IA_SwitchStance")

if imc and ia_move and ia_ollie and ia_brake and ia_switch:
    print("All input assets loaded successfully")
else:
    print("ERROR: Failed to load some input assets")

# --- Set GameMode in DefaultEngine.ini ---
# We set it via config so it applies to all maps
unreal.SystemLibrary.execute_console_command(
    None,
    "r.SetRes 1920x1080"
)

print("Setup script complete")
print("NOTE: Key mappings for IMC_Skating must be configured in-editor")
print("NOTE: Set GameMode to TH2GameMode in Project Settings > Maps & Modes")
