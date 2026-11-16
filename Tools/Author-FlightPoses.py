"""Author looping flight and carry clips on the template mannequin rig."""

import math

import unreal as ue


SOURCE = "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop"
NEUTRAL = "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"
JUMP = "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump"
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


def destination_asset(name):
    path = DESTINATION + name
    if ue.EditorAssetLibrary.does_asset_exist(path):
        asset = ue.EditorAssetLibrary.load_asset(path)
    else:
        asset = ue.EditorAssetLibrary.duplicate_asset(SOURCE, path)
    assert asset is not None, name
    return asset


def author(source, neutral, name, travel=False, carry=False):
    asset = destination_asset(name)
    library = ue.AnimationLibrary
    frame_count = library.get_num_frames(source)
    # In Manny's component space, +Y is forward and +X is the mannequin's left.
    # Cruise adds pitch around the capsule, so the raised arm leads the motion.
    directions = {
        'upperarm_l': ('lowerarm_l', (0.18, -0.24, -1.0) if travel else (0.27, -0.16, -1.0)),
        'lowerarm_l': ('hand_l', (-0.09, 0.32, -1.0) if travel else (0.06, 0.12, -1.0)),
        'upperarm_r': ('lowerarm_r', (-0.18, 0.23, 1.0) if travel else (-0.31, -0.12, -1.0)),
        'lowerarm_r': ('hand_r', (0.02, 0.14, 1.0) if travel else (-0.03, 0.23, -1.0)),
        'thigh_l': ('calf_l', (0.04, 0.02, -1.0) if travel else (0.13, -0.06, -1.0)),
        'calf_l': ('foot_l', (0.02, -0.10, -1.0)),
        'thigh_r': ('calf_r', (-0.05, -0.09, -1.0) if travel else (-0.15, 0.24, -1.0)),
        'calf_r': ('foot_r', (-0.02, -0.16, -1.0) if travel else (-0.02, -0.48, -1.0)),
        'foot_l': ('ball_l', (0.09, 0.88, -0.44) if travel else (0.12, 0.89, -0.29)),
        'foot_r': ('ball_r', (-0.09, 0.88, -0.44) if travel else (-0.10, 0.91, -0.33)),
    }
    if carry:
        directions = {
            'upperarm_l': ('lowerarm_l', (0.54, 0.12, 0.85)),
            'lowerarm_l': ('hand_l', (-0.31, 0.35, 0.92)),
            'upperarm_r': ('lowerarm_r', (-0.54, 0.12, 0.85)),
            'lowerarm_r': ('hand_r', (0.31, 0.35, 0.92)),
            'thigh_l': ('calf_l', (0.17, 0.14, -1.0)),
            'calf_l': ('foot_l', (0.03, -0.22, -1.0)),
            'thigh_r': ('calf_r', (-0.17, 0.14, -1.0)),
            'calf_r': ('foot_r', (-0.03, -0.22, -1.0)),
            'foot_l': ('ball_l', (0.12, 0.90, -0.32)),
            'foot_r': ('ball_r', (-0.12, 0.90, -0.32)),
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
    return asset


def smoothstep(value):
    value = max(0.0, min(1.0, value))
    return value * value * (3.0 - 2.0 * value)


def blend_pose(before, after, amount):
    start = quaternion(before.rotation)
    end = quaternion(after.rotation)
    if sum(a * b for a, b in zip(start, end)) < 0.0:
        end = tuple(-component for component in end)
    rotation = ue.Quat(*unit(tuple(a + (b - a) * amount for a, b in zip(start, end))))
    return (before.translation + (after.translation - before.translation) * amount, rotation,
            before.scale3d + (after.scale3d - before.scale3d) * amount)


def author_transition(name, frames, bones, sample):
    asset = destination_asset(name)
    tracks = {bone: ([], [], []) for bone in bones}
    for i in range(frames + 1):
        pose = sample(i / 30.0, i / frames)
        for bone, transform in pose.items():
            for keys, value in zip(tracks[bone], transform):
                keys.append(value)
    controller = asset.controller
    controller.open_bracket("Author flight transition", False)
    try:
        controller.set_frame_rate(ue.FrameRate(30, 1), False)
        controller.set_number_of_frames(ue.FrameNumber(frames), False)
        for bone, (positions, rotations, scales) in tracks.items():
            assert controller.set_bone_track_keys(bone, positions, rotations, scales, False), bone
    finally:
        controller.close_bracket(False)
    for notify in ue.AnimationLibrary.get_animation_notify_event_names(asset):
        ue.AnimationLibrary.remove_animation_notify_events_by_name(asset, notify)
    asset.set_editor_property('enable_root_motion', False)
    assert ue.AnimationLibrary.get_num_frames(asset) == frames, name
    assert ue.EditorAssetLibrary.save_loaded_asset(asset), name
    ue.log(f"IMPACT FLIGHT CLIP SAVED {name} ({frames / 30:.3f} s)")


source_animation = ue.load_asset(SOURCE)
neutral_animation = ue.load_asset(NEUTRAL)
jump_animation = ue.load_asset(JUMP)
assert source_animation and neutral_animation and jump_animation
hover = author(source_animation, neutral_animation, "A_FlightHover")
author(source_animation, neutral_animation, "A_FlightCruise", travel=True)
carry = author(source_animation, neutral_animation, "A_FlightCarry", carry=True)

library = ue.AnimationLibrary
bones = [str(bone) for bone in library.get_animation_track_names(source_animation)]
hover_pose = dict(zip(bones, library.get_bone_poses_for_frame(hover, bones, 0, False)))
carry_pose = dict(zip(bones, library.get_bone_poses_for_frame(carry, bones, 0, False)))
jump_duration = library.get_sequence_length(jump_animation)
lower_body = {'pelvis', 'thigh_l', 'thigh_r', 'calf_l', 'calf_r', 'foot_l', 'foot_r', 'ball_l', 'ball_r'}


def takeoff(time, progress, carrying=False):
    # Use the beginning and extension of Manny's jump, then join frame zero of
    # the actual flight loop before returning control to its normal presentation.
    jump_time = min(time, jump_duration * 0.55)
    jump = dict(zip(bones, library.get_bone_poses_for_time(jump_animation, bones, jump_time, False)))
    final = carry_pose if carrying else hover_pose
    settle = smoothstep((progress - 0.37) / 0.63)
    result = {}
    for bone in bones:
        if carrying:
            # Keep the arms and shoulders braced throughout, with a smaller leg
            # response than unloaded takeoff so that the IK has stable support.
            spring = 0.42 * smoothstep(progress / 0.26) * (1.0 - settle)
            weight = spring if bone in lower_body else 0.0
            result[bone] = blend_pose(final[bone], jump[bone], weight)
        else:
            origin = final[bone] if bone == 'root' else jump[bone]
            result[bone] = blend_pose(origin, final[bone], settle)
    return result


throw_bones = {'spine_03': 0.35, 'spine_05': 0.50, 'upperarm_l': 0.63, 'upperarm_r': 0.63,
               'lowerarm_l': 0.32, 'lowerarm_r': 0.32}
throw_parents = {bone: [str(parent) for parent in library.find_bone_path_to_root(carry, bone)][1:][::-1]
                 for bone in throw_bones}


def throw(time, progress):
    recovery = smoothstep((time - 0.19) / 0.175)
    result = {bone: blend_pose(carry_pose[bone], hover_pose[bone], recovery) for bone in bones}
    # Shift back at first, then push the shoulder and elbow chains through the
    # release moment around 0.17 seconds. No root movement enters gameplay.
    brace = 13.0 * smoothstep(time / 0.08) * (1.0 - smoothstep((time - 0.08) / 0.09))
    drive = -34.0 * smoothstep((time - 0.075) / 0.10) * (1.0 - recovery)
    for bone, weight in throw_bones.items():
        parent = (0.0, 0.0, 0.0, 1.0)
        for ancestor in throw_parents[bone]:
            parent = multiply(parent, quaternion(result[ancestor][1]))
        position, rotation, scale = result[bone]
        delta = multiply(inverse(parent), multiply(pitch((brace + drive) * weight), parent))
        result[bone] = (position, ue.Quat(*unit(multiply(delta, quaternion(rotation)))), scale)
    return result


author_transition("A_FlightTakeoff", 13, bones, takeoff)
author_transition("A_FlightCarryTakeoff", 13, bones, lambda time, progress: takeoff(time, progress, carrying=True))
author_transition("A_FlightThrow", 11, bones, throw)
