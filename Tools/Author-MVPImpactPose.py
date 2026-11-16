"""Author the compact H01 impact-dive pose from the existing Manny hover clip."""

import math

import unreal as ue


SOURCE = '/Game/IB_Flight/A_FlightHover'
DESTINATION = '/Game/IB_Flight/A_ImpactDive'
FRAMES = 14
LIBRARY = ue.AnimationLibrary


def quaternion(rotation):
    return rotation.x, rotation.y, rotation.z, rotation.w


def multiply(a, b):
    x, y, z, w = a
    i, j, k, r = b
    return (w*i + x*r + y*k - z*j, w*j - x*k + y*r + z*i,
            w*k + x*j - y*i + z*r, w*r - x*i - y*j - z*k)


def inverse(q):
    return -q[0], -q[1], -q[2], q[3]


def unit(v):
    length = math.sqrt(sum(component * component for component in v))
    return tuple(component / length for component in v)


def blend(a, b, weight):
    if sum(left * right for left, right in zip(a, b)) < 0.0:
        b = tuple(-component for component in b)
    return unit(tuple(left + (right - left) * weight for left, right in zip(a, b)))


def direction_delta(before, after):
    a, b = unit(before), unit(after)
    alignment = sum(x * y for x, y in zip(a, b))
    if alignment < -0.9999:
        perpendicular = (0, -a[2], a[1]) if abs(a[0]) > 0.5 else (-a[2], 0, a[0])
        return (*unit(perpendicular), 0.0)
    return unit((a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                 a[0] * b[1] - a[1] * b[0], 1.0 + alignment))


source = ue.load_asset(SOURCE)
assert source is not None, SOURCE
assets = ue.EditorAssetLibrary
clip = assets.load_asset(DESTINATION) if assets.does_asset_exist(DESTINATION) else assets.duplicate_asset(SOURCE, DESTINATION)
assert clip is not None, DESTINATION

# Directions are in Manny component space (+Y forward). Lead the dive with the
# right fist and pull the left forearm back, leaving a deliberate asymmetric knee.
directions = {
    'upperarm_l': ('lowerarm_l', (-0.24, 0.20, -0.92)),
    'lowerarm_l': ('hand_l', (0.24, 0.66, -0.46)),
    'upperarm_r': ('lowerarm_r', (0.17, 0.51, -0.87)),
    'lowerarm_r': ('hand_r', (-0.06, 0.28, -0.96)),
    'thigh_l': ('calf_l', (-0.11, -0.17, -0.98)),
    'calf_l': ('foot_l', (-0.08, -0.24, -0.97)),
    'thigh_r': ('calf_r', (0.12, 0.28, -0.91)),
    'calf_r': ('foot_r', (0.05, -0.54, -0.84)),
}
pitch_degrees = {'spine_03': -8.0, 'neck_01': 4.0, 'head': 2.0}
animated = [*pitch_degrees, *directions]
parents = {
    bone: [str(parent) for parent in LIBRARY.find_bone_path_to_root(source, bone)][1:][::-1]
    for bone in animated
}
bones = {str(bone) for bone in LIBRARY.get_animation_track_names(source)}
needed = bones | {child for child, _ in directions.values()}
needed.update(parent for path in parents.values() for parent in path)
base = {bone: LIBRARY.get_bone_pose_for_frame(source, bone, 0, False) for bone in needed}
rotations = {bone: [] for bone in animated}
for frame in range(FRAMES + 1):
    # Reach the braced pose in roughly 0.17 seconds and hold it until physical contact.
    engage = min(frame / 5.0, 1.0)
    weight = engage * engage * (3.0 - 2.0 * engage)
    adjusted = {}
    for bone in animated:
        parent = (0.0, 0.0, 0.0, 1.0)
        for ancestor in parents[bone]:
            parent = multiply(parent, adjusted.get(ancestor) or quaternion(base[ancestor].rotation))
        local = quaternion(base[bone].rotation)
        current = multiply(parent, local)
        if bone in pitch_degrees:
            half_angle = math.radians(pitch_degrees[bone]) / 2.0
            delta = (math.sin(half_angle), 0.0, 0.0, math.cos(half_angle))
        else:
            child, target = directions[bone]
            offset = base[child].translation
            aim = multiply(multiply(current, (offset.x, offset.y, offset.z, 0.0)), inverse(current))[:3]
            delta = direction_delta(aim, target)
        target_local = unit(multiply(inverse(parent), multiply(delta, current)))
        adjusted[bone] = blend(local, target_local, weight)
        rotations[bone].append(ue.Quat(*adjusted[bone]))

controller = clip.controller
controller.open_bracket('Author compact impact dive', False)
try:
    controller.set_frame_rate(ue.FrameRate(30, 1), False)
    controller.set_number_of_frames(ue.FrameNumber(FRAMES), False)
    for bone in sorted(bones):
        pose = base[bone]
        keys = rotations.get(bone, [pose.rotation])
        assert controller.set_bone_track_keys(bone, [pose.translation] * len(keys), keys,
                                              [pose.scale3d] * len(keys), False), bone
finally:
    controller.close_bracket(False)
for notify in LIBRARY.get_animation_notify_event_names(clip):
    LIBRARY.remove_animation_notify_events_by_name(clip, notify)
clip.set_editor_property('enable_root_motion', False)
assert LIBRARY.get_num_frames(clip) == FRAMES
assert assets.save_loaded_asset(clip)
ue.log(f'IB IMPACT DIVE SAVED: {clip.get_path_name()} ({FRAMES / 30.0:.3f} s, no root motion)')
