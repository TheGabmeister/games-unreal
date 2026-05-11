# Plugins/McpAutomationBridge

Native C++ Automation Bridge for Unreal Engine 5.0-5.7. Provides live editor automation via HTTP.

## DIRECT REST API (PRIMARY USAGE)

**Endpoint:** `POST http://localhost:3000/api/call`

No MCP session, no JSON-RPC envelope. One curl per operation. Requires the editor to be running with `bEnableNativeMCP=True` in `DefaultGame.ini`.

### Request Format
```json
{"tool": "<tool_name>", "arguments": {<action-specific params>}}
```

### Response Format
```json
{"success": true, "message": "...", "data": {...}}
```

### Common Operations

**Set inherited component property (e.g., assign mesh to Blueprint):**
```bash
curl -s http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Path/BP_Name","propertyName":"Mesh.SkeletalMeshAsset","value":"/Game/Path/SK_Name.SK_Name"}}'
```

**Get SCS component hierarchy:**
```bash
curl -s http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"get_scs","blueprintPath":"/Game/Path/BP_Name.BP_Name"}}'
```

**Set SCS component property (for Blueprint-added components only):**
```bash
curl -s http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_scs_property","blueprintPath":"/Game/Path/BP_Name.BP_Name","componentName":"MyComp","propertyName":"PropName","value":"PropValue"}}'
```

**Search assets:**
```bash
curl -s http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_asset","arguments":{"action":"search_assets","search_path":"/Game","filter":"SearchTerm"}}'
```

### Key Concepts

- **Inherited C++ components** (e.g., `Mesh` from `ACharacter`): Use `set_default` with dot-path (`Mesh.SkeletalMeshAsset`). This sets the CDO property.
- **SCS components** (added in Blueprint editor): Use `get_scs` / `set_scs_property`.
- **Asset paths** use the format `/Game/Folder/AssetName.AssetName` (with the object name after the dot) for load operations.
- **Blueprint paths** for `requestedPath` can omit the dot suffix: `/Game/Folder/BP_Name`.
- The editor **must be open** for any operation to work. All calls are dispatched on the GameThread.

### Configuration

In `Config/DefaultGame.ini`:
```ini
[/Script/McpAutomationBridge.McpAutomationBridgeSettings]
bEnableNativeMCP=True
NativeMCPPort=3000
bLoadAllToolsOnStart=True
```

Requires editor restart after changing these values.

---

## OVERVIEW
Editor-only UE subsystem executing automation requests received via WebSocket or HTTP. 56 handler files, 70 C++ source files total.

## STRUCTURE
```
Source/McpAutomationBridge/
├── Public/
│   ├── McpAutomationBridgeSubsystem.h  # Main subsystem, handler declarations
│   └── McpAutomationBridgeSettings.h   # Host/Port/Token config
├── Private/
│   ├── McpAutomationBridgeSubsystem.cpp    # Initialize, tick, dispatch
│   ├── McpAutomationBridge_ProcessRequest.cpp  # Request routing
│   ├── *Handlers.cpp                       # Action implementations (56 files)
│   ├── McpAutomationBridgeHelpers.h        # Critical UE 5.7 safety helpers
│   └── MCP/McpNativeTransport.cpp          # HTTP server + /api/call REST endpoint
└── McpAutomationBridge.Build.cs
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add handler | `*Handlers.cpp` | Declare in `Subsystem.h`, register in `InitializeHandlers()` |
| REST endpoint | `MCP/McpNativeTransport.cpp` | `HandleDirectCall()` for /api/call |
| Save Asset | `McpSafeAssetSave(Asset)` | Use helper in `McpAutomationBridgeHelpers.h` |
| Component creation | `SCS->CreateNode()` | Use proper SCS ownership for UE 5.7 |
| JSON Parsing | `FJsonObjectConverter` | UE standard for Struct ↔ JSON |
| Path Security | `SanitizeProjectRelativePath()` | Block traversal attacks |

## CONVENTIONS
- **Game Thread Safety**: Handlers dispatched to game thread automatically by subsystem.
- **UE 5.7+ SCS**: Component templates owned by `SCS_Node`, not Blueprint.
- **Safe Saving**: NEVER use `UPackage::SavePackage()`. Use `McpSafeAssetSave`.
- **ANY_PACKAGE**: Deprecated. Use `nullptr` for path-based object lookups.

## ANTI-PATTERNS
- **Modal Dialogs**: Avoid `UEditorAssetLibrary::SaveAsset()` on new assets (crashes D3D12).
- **Hardcoded Paths**: Do not use absolute Windows paths in handlers.
- **Blocking Thread**: WebSocket frame processing must not block game thread.
