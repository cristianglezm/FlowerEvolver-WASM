bl_info = {
    "name":        "Import glTF Flower with lights (FlowerEvolver)",
    "author":      "Cristian Gonzalez",
    "version":     (1, 2, 1),
    "blender":     (2, 80, 0),
    "location":    "File > Import > glTF 2.0 (FlowerEvolver Lights)",
    "description": "Import Emissive Flowers from FlowerEvolver",
    "warning":     "",
    "support":     "COMMUNITY",
    "wiki_url":    "https://github.com/cristianglezm/FlowerEvolver-WASM#readme",
    "tracker_url": "https://github.com/cristianglezm/FlowerEvolver-WASM/issues",
    "category":    "Import-Export",
}

import bpy
from bpy.props import BoolProperty
from bpy_extras.io_utils import ImportHelper
from bpy.props import StringProperty
from bpy.types import AddonPreferences, Operator
from bpy.props import StringProperty

class FEL_Importer_Prefs(AddonPreferences):
    bl_idname = __name__
    repo_url: StringProperty(
        name="Repo URL",
        default="https://github.com/cristianglezm/FlowerEvolver-WASM/issues"
    )

    def draw(self, context):
        layout = self.layout
        row = layout.row()
        row.operator("wm.url_open", text="Open GitHub Repo", icon='URL').url = self.repo_url

class ImportGLTFFlowerWithLights(bpy.types.Operator, ImportHelper):
    bl_idname      = "import_scene.gltf_flower_lights"
    bl_label       = "glTF 2.0 (FlowerEvolver Lights)"
    filename_ext   = ".gltf"
    filter_glob: StringProperty(default="*.gltf;", options={'HIDDEN'})
    import_extras: BoolProperty(
        name="Import Extras",
        default=True,
        description="Bring in glTF extras as custom properties"
    )
    def execute(self, context):
        # Prepare light counter and collection
        self.light_counter = 0

        # Import and detect new objects
        before = set(context.scene.objects)
        bpy.ops.import_scene.gltf('EXEC_DEFAULT', filepath=self.filepath, import_scene_extras = self.import_extras)
        after  = set(context.scene.objects)
        new_objs = after - before

        # Create lights for matching nodes
        spawned = 0
        for obj in new_objs:
            if not obj.name.startswith("Petal_Layer"):
                continue

            lights = obj.get("lights")
            if not isinstance(lights, list):
                print(f"[Skip] No lights array on {obj.name}")
                continue

            ld = lights[0]
            print(f"[Found] Light for {obj.name}: {ld}")
            self._create_light(obj, ld)
            spawned += 1

        self.report({'INFO'}, f"Spawned {spawned} lights")
        return {'FINISHED'}

    def _create_light(self, parent, ld):
        light_name = parent.name.replace("Node", "Light")
        self.light_counter += 1

        # Type resolution
        lt = ld.get("type", "PointLight").upper()
        bl_type = "POINT" if "POINT" in lt else lt

        # Create data & object
        L = bpy.data.lights.new(light_name, type=bl_type)
        ci = int(ld.get("color", 0))
        L.color = (
            ((ci >> 16) & 0xFF) / 255,
            ((ci >>  8) & 0xFF) / 255,
            ( ci        & 0xFF) / 255,
        )
        L.energy           = ld.get("intensity", 1.0)
        L.shadow_soft_size = ld.get("radius", 0.1)
        decay = ld.get("decay", 2.0)

        obj_L = bpy.data.objects.new(light_name, L)

        # Position & parent
        x, y, z = ld.get("position", [0,0,0])
        obj_L.location = (x, -z, y)
        # parent under the petal node
        obj_L.parent = parent
        obj_L.matrix_parent_inverse = parent.matrix_world.inverted()

        # link into the same collection(s) as the parent
        for coll in parent.users_collection:
            coll.objects.link(obj_L)

def menu_func_import(self, context):
    self.layout.operator(
        ImportGLTFFlowerWithLights.bl_idname,
        text="glTF 2.0 Flower With lights (FlowerEvolver)"
    )

def register():
    bpy.utils.register_class(FEL_Importer_Prefs)
    bpy.utils.register_class(ImportGLTFFlowerWithLights)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)

def unregister():
    bpy.utils.unregister_class(ImportGLTFFlowerWithLights)
    bpy.utils.unregister_class(FEL_Importer_Prefs)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)

if __name__ == "__main__":
    register()
