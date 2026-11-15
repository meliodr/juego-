"""Author two looping mannequin clips from the template's airborne animation."""

import math

import unreal as ue


SOURCE = "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop"
DESTINATION = "/Game/IB_Flight/"


def quaternion(rotation):
    return (rotation.x, rotation.y, rotation.z, rotation.w)


def multiply(a, b):
    x, y, z, w = a
    i, j, k, r = b
    return (w*i + x*r + y*k - z*j, w*j - x*k + y*r + z*i,
            w*k + x*j - y*i + z*r, w*r - x*i - y*j - z*k)


def inverse(q):
    return (-q[0], -q[1], -q[2], q[3])


def rotate(q, v):
    return multiply(multiply(q, (*v, 0)), inverse(q))[:3]


def unit(v):
    length = math.sqrt(sum(x*x for x in v))
    return tuple(x / length for x in v)


def aim_delta(before, after):
    a, b = unit(before), unit(after)
    dot = sum(x*y for x, y in zip(a, b))
    if dot < -0.9999:
        perpendicular = (0, -a[2], a[1]) if abs(a[0]) > 0.5 else (-a[2], 0, a[0])
        return (*unit(perpendicular), 0)
    return unit((a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2],
                 a[0]*b[1] - a[1]*b[0], 1.0 + dot))


def author(source, name, travel):
    asset = ue.EditorAssetLibrary.load_asset(DESTINATION + name)
    if asset is None:
        asset = ue.EditorAssetLibrary.duplicate_asset(SOURCE, DESTINATION + name)
    assert asset is not None, name
    library = ue.AnimationLibrary
    frame_count = library.get_num_frames(source)
    # The mannequin mesh faces +Y. In travel the body pitches forward around the capsule.
    directions = {
        'upperarm_l': ('lowerarm_l', (-0.22, -0.25, -1.0) if travel else (-0.28, -0.10, -1.0)),
        'lowerarm_l': ('hand_l', (-0.12, 0.38, -1.0) if travel else (-0.12, 0.46, -1.0)),
        'upperarm_r': ('lowerarm_r', (0.16, 0.36, 1.0) if travel else (0.28, -0.10, -1.0)),
        'lowerarm_r': ('hand_r', (0.06, 0.38, 1.0) if travel else (0.12, 0.46, -1.0)),
        'thigh_l': ('calf_l', (-0.15, 0.08, -1.0)),
        'calf_l': ('foot_l', (-0.03, -0.28, -1.0)),
        'thigh_r': ('calf_r', (0.14, -0.10, -1.0) if travel else (0.18, 0.24, -1.0)),
        'calf_r': ('foot_r', (0.04, -0.18, -1.0) if travel else (0.04, -0.45, -1.0)),
    }
    paths = {key: [str(bone) for bone in list(library.find_bone_path_to_root(source, key))[1:]][::-1]
             for key in directions}
    tracks = {bone: ([], [], []) for bone in directions}
    for frame in range(frame_count + 1):
        # Sampling frame zero at both ends makes the adjusted tracks wrap cleanly.
        sample = frame % frame_count
        needed = set(directions) | {child for child, _ in directions.values()}
        needed.update(parent for path in paths.values() for parent in path)
        local = {bone: library.get_bone_pose_for_frame(source, bone, sample, False) for bone in needed}
        adjusted = {}
        motion = 0.045 * math.sin(2.0 * math.pi * frame / frame_count)
        for bone, (child, direction) in directions.items():
            parent = (0, 0, 0, 1)
            for ancestor in paths[bone]:
                q = adjusted.get(ancestor) or quaternion(local[ancestor].rotation)
                parent = multiply(parent, q)
            current = multiply(parent, quaternion(local[bone].rotation))
            offset = local[child].translation
            aim = rotate(current, (offset.x, offset.y, offset.z))
            target = (direction[0], direction[1] + motion, direction[2])
            rotation = unit(multiply(inverse(parent), multiply(aim_delta(aim, target), current)))
            adjusted[bone] = rotation
            positions, rotations, scales = tracks[bone]
            positions.append(local[bone].translation)
            rotations.append(ue.Quat(*rotation))
            scales.append(local[bone].scale3d)
    controller = asset.controller
    controller.open_bracket("Author airborne pose", False)
    try:
        for bone, (positions, rotations, scales) in tracks.items():
            assert controller.set_bone_track_keys(bone, positions, rotations, scales, False), bone
    finally:
        controller.close_bracket(False)
    assert ue.EditorAssetLibrary.save_loaded_asset(asset), name
    ue.log(f"IMPACT FLIGHT CLIP SAVED {name} ({frame_count + 1} keys)")


source_animation = ue.load_asset(SOURCE)
assert source_animation
author(source_animation, "A_FlightHover", False)
author(source_animation, "A_FlightCruise", True)
