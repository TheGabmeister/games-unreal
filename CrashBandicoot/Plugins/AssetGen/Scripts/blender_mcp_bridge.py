"""
MCP stdio server that bridges Claude Code to the Blender MCP addon's TCP socket.

The Blender addon (mcp-0.3.0) runs a TCP server on localhost:9876 that accepts
null-byte-delimited JSON requests containing Python code to execute inside Blender.
This script exposes that as a proper MCP server over stdio so Claude Code can use it.

Usage in .mcp.json:
  {"mcpServers": {"blender": {"command": "python", "args": ["path/to/blender_mcp_bridge.py"]}}}
"""

import json
import socket
from mcp.server.fastmcp import FastMCP

BLENDER_HOST = "localhost"
BLENDER_PORT = 9876
RECV_BUFFER = 65536
TIMEOUT = 30.0


def _send_to_blender(code: str, strict_json: bool = False) -> dict:
    request = json.dumps({"type": "execute", "code": code, "strict_json": strict_json}) + "\0"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(TIMEOUT)
    try:
        sock.connect((BLENDER_HOST, BLENDER_PORT))
        sock.sendall(request.encode("utf-8"))
        buf = bytearray()
        while b"\0" not in buf:
            chunk = sock.recv(RECV_BUFFER)
            if not chunk:
                break
            buf.extend(chunk)
        response_str = buf[:buf.index(b"\0")].decode("utf-8") if b"\0" in buf else buf.decode("utf-8")
        return json.loads(response_str)
    except ConnectionRefusedError:
        return {"status": "error", "message": "Cannot connect to Blender. Is it running with the MCP addon enabled?"}
    except socket.timeout:
        return {"status": "error", "message": "Blender did not respond within the timeout period."}
    finally:
        sock.close()


mcp = FastMCP("Blender")


@mcp.tool()
def execute_python(code: str) -> str:
    """Execute Python code inside the running Blender instance.

    The code runs in Blender's main thread with full access to bpy and all Blender APIs.
    Store your return value in a variable called `result` as a dict, e.g.:
        result = {"objects": [obj.name for obj in bpy.data.objects]}

    Returns the JSON response from Blender with status and result.
    """
    response = _send_to_blender(code, strict_json=False)
    return json.dumps(response, indent=2)


@mcp.tool()
def get_scene_info() -> str:
    """Get basic information about the current Blender scene."""
    code = """
import bpy
scene = bpy.context.scene
result = {
    "blend_file": bpy.data.filepath or "(unsaved)",
    "scene_name": scene.name,
    "objects": [{"name": obj.name, "type": obj.type} for obj in scene.objects],
    "collections": [c.name for c in bpy.data.collections],
    "frame_range": [scene.frame_start, scene.frame_end],
    "frame_current": scene.frame_current,
}
"""
    response = _send_to_blender(code, strict_json=True)
    return json.dumps(response, indent=2)


@mcp.tool()
def get_selected_objects() -> str:
    """Get details about the currently selected objects in Blender."""
    code = """
import bpy
result = {"selected": []}
for obj in bpy.context.selected_objects:
    info = {"name": obj.name, "type": obj.type, "location": list(obj.location)}
    if obj.type == "MESH":
        info["vertices"] = len(obj.data.vertices)
        info["faces"] = len(obj.data.polygons)
    result["selected"].append(info)
if bpy.context.active_object:
    result["active"] = bpy.context.active_object.name
"""
    response = _send_to_blender(code, strict_json=True)
    return json.dumps(response, indent=2)


if __name__ == "__main__":
    mcp.run(transport="stdio")
