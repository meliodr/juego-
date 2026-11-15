"""Author looping flight and carry clips on the template mannequin rig."""

import math

import unreal as ue


SOURCE = "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop"
NEUTRAL = "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"
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


def pitch(degrees):
    half_angle = math.radians(degrees) / 2.0
    return (math.sin(half_angle), 0.0, 0.0, math.cos(half_angle))


def author(source, neutral, name, travel=False, carry=False):
    path = DESTINATION + name
    if ue.EditorAssetLibrary.does_asset_exist(path):
        asset = ue.EditorAssetLibrary.load_asset(path)
    else:
        asset = ue.EditorAssetLibrary.duplicate_asset(SOURCE, DESTINATION + name)
    assert asset is not None, name
    library = ue.AnimationLibrary
    frame_count = library.get_num_frames(source)
    # The mannequin faces +Y. Cruise adds pitch around the capsule in game, so
    # extend one arm past the head and compensate at the neck to keep a clear sightline.
    directions = {
        'upperarm_l': ('lowerarm_l', (-0.15, -0.15, -1.0) if travel else (-0.23, -0.08, -1.0)),
        'lowerarm_l': ('hand_l', (0.03, 0.48, -1.0) if travel else (-0.02, 0.33, -1.0)),
        'upperarm_r': ('lowerarm_r', (0.18, 0.23, 1.0) if travel else (0.23, -0.08, -1.0)),
        'lowerarm_r': ('hand_r', (0.07, 0.19, 1.0) if travel else (0.02, 0.33, -1.0)),
        'thigh_l': ('calf_l', (-0.10, 0.04, -1.0)),
        'calf_l': ('foot_l', (-0.03, -0.11, -1.0)),
        'thigh_r': ('calf_r', (0.10, -0.07, -1.0) if travel else (0.13, 0.13, -1.0)),
        'calf_r': ('foot_r', (0.02, -0.17, -1.0) if travel else (0.02, -0.27, -1.0)),
    }
    if carry:
        directions = {
            'upperarm_l': ('lowerarm_l', (-0.54, 0.12, 0.85)),
            'lowerarm_l': ('hand_l', (0.31, 0.35, 0.92)),
            'upperarm_r': ('lowerarm_r', (0.54, 0.12, 0.85)),
            'lowerarm_r': ('hand_r', (-0.31, 0.35, 0.92)),
            'thigh_l': ('calf_l', (-0.17, 0.14, -1.0)),
            'calf_l': ('foot_l', (-0.03, -0.22, -1.0)),
            'thigh_r': ('calf_r', (0.17, 0.14, -1.0)),
            'calf_r': ('foot_r', (0.03, -0.22, -1.0)),
        }
    axial_pitch = {'spine_03': 5.0 if travel else 4.0, 'neck_01': 12.0 if travel else -1.0,
                   'neck_02': 12.0 if travel else 0.0, 'head': 10.0 if travel else 1.0}
    animated = list(axial_pitch) + list(directions)
    paths = {bone: [str(parent) for parent in library.find_bone_path_to_root(neutral, bone)][1:][::-1]
             for bone in animated}
    rest_tracks = [str(bone) for bone in library.get_animation_track_names(source)]
    needed = set(rest_tracks) | set(animated) | {child for child, _ in directions.values()}
    needed.update(parent for path in paths.values() for parent in path)
    local = {bone: library.get_bone_pose_for_frame(neutral, bone, 0, False) for bone in needed}
    tracks = {bone: ([], [], []) for bone in animated}
    for frame in range(frame_count + 1):
        # Use one neutral pose as the basis of the whole skeleton. Only authored motion
        # repeats, so no part of the original fall cycle survives on unedited tracks.
        adjusted = {}
        breath = math.sin(2.0 * math.pi * frame / frame_count)
        for bone in animated:
            parent = (0, 0, 0, 1)
            for ancestor in paths[bone]:
                q = adjusted.get(ancestor) or quaternion(local[ancestor].rotation)
                parent = multiply(parent, q)
            current = multiply(parent, quaternion(local[bone].rotation))
            if bone in axial_pitch:
                degrees = axial_pitch[bone] + (1.5 * breath if bone == 'spine_03' else 0.0)
                delta = pitch(degrees)
            else:
                child, direction = directions[bone]
                offset = local[child].translation
                aim = rotate(current, (offset.x, offset.y, offset.z))
                target = (direction[0], direction[1] + (0.026 if travel else 0.040) * breath, direction[2])
                delta = aim_delta(aim, target)
            rotation = unit(multiply(inverse(parent), multiply(delta, current)))
            adjusted[bone] = rotation
            positions, rotations, scales = tracks[bone]
            positions.append(local[bone].translation)
            rotations.append(ue.Quat(*rotation))
            scales.append(local[bone].scale3d)
    controller = asset.controller
    controller.open_bracket("Author airborne pose", False)
    try:
        for bone in rest_tracks:
            if bone not in tracks:
                pose = local[bone]
                assert controller.set_bone_track_keys(bone, [pose.translation], [pose.rotation], [pose.scale3d], False), bone
        for bone, (positions, rotations, scales) in tracks.items():
            assert controller.set_bone_track_keys(bone, positions, rotations, scales, False), bone
    finally:
        controller.close_bracket(False)
    assert ue.EditorAssetLibrary.save_loaded_asset(asset), name
    ue.log(f"IMPACT FLIGHT CLIP SAVED {name} ({frame_count + 1} keys)")


source_animation = ue.load_asset(SOURCE)
neutral_animation = ue.load_asset(NEUTRAL)
assert source_animation and neutral_animation
author(source_animation, neutral_animation, "A_FlightHover", False)
author(source_animation, neutral_animation, "A_FlightCruise", True)
author(source_animation, neutral_animation, "A_FlightCarry", carry=True)
