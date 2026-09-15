"""Select a real Cycles GPU backend in every Blender process.
Device preferences are process/user preferences, not stored in the .blend.
"""
import bpy

def configure_gpu():
    prefs = bpy.context.preferences.addons['cycles'].preferences
    for backend in ('OPTIX', 'CUDA', 'HIP', 'ONEAPI'):
        try:
            prefs.compute_device_type = backend
            prefs.get_devices()
        except (TypeError, RuntimeError):
            continue
        devices = [d for d in prefs.devices if d.type == backend]
        if not devices:
            continue
        for device in prefs.devices:
            device.use = device.type == backend
        bpy.context.scene.cycles.device = 'GPU'
        print('CYCLES_GPU_ACTIVE', backend, [d.name for d in devices], flush=True)
        return backend
    raise RuntimeError('No supported Cycles GPU found; refusing silent CPU fallback.')

configure_gpu()
