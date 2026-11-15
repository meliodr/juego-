"""Author in-place Manny flight speed loops and a braking transition in Unreal Editor."""

import math

import unreal as ue


FOLDER = '/Game/IB_Flight/'
SOURCE_PATH = FOLDER + 'A_FlightHover'
FPS = 30

# Manny faces +Y in mesh space. X points toward his left, Z points up.
# Values describe the direction of the child joint relative to its parent.
CHILDREN = {
    'upperarm_l': 'lowerarm_l', 'lowerarm_l': 'hand_l',
    'upperarm_r': 'lowerarm_r', 'lowerarm_r': 'hand_r',
    'thigh_l': 'calf_l', 'calf_l': 'foot_l',
    'thigh_r': 'calf_r', 'calf_r': 'foot_r',
    'foot_l': 'ball_l', 'foot_r': 'ball_r',
}
SPINE = ('spine_01', 'spine_03', 'neck_01', 'head')

# A controlled lead at V2 gives way to a long forward reach at V3. V4 tucks
# both arms for sustained boost. V5 tightens the same silhouette.
SPEED_2 = (
    (0.27, -0.25, -1.00), (0.04, 0.20, -1.00),
    (-0.23, 0.60, 0.82), (0.05, 0.72, 0.70),
    (0.09, -0.04, -1.00), (0.03, -0.28, -1.00),
    (-0.11, 0.20, -1.00), (-0.04, -0.48, -1.00),
    (0.09, 0.83, -0.56), (-0.09, 0.87, -0.49),
)
SPEED_3 = (
    (0.27, -0.40, -0.93), (-0.03, -0.08, -1.00),
    (-0.22, 0.81, 0.84), (0.04, 0.87, 0.88),
    (0.05, -0.15, -1.00), (0.01, -0.23, -1.00),
    (-0.08, 0.10, -1.00), (-0.02, -0.40, -1.00),
    (0.07, 0.68, -0.73), (-0.07, 0.74, -0.67),
)
SPEED_4 = (
    (0.25, -0.46, -0.94), (-0.08, -0.09, -1.00),
    (-0.26, -0.50, -0.93), (0.08, -0.17, -1.00),
    (0.05, -0.13, -1.00), (0.03, -0.25, -1.00),
    (-0.04, -0.07, -1.00), (-0.02, -0.25, -1.00),
    (0.06, 0.64, -0.76), (-0.05, 0.64, -0.76),
)
SPEED_5 = (
    (0.21, -0.50, -0.91), (-0.07, -0.20, -1.00),
    (-0.22, -0.52, -0.90), (0.07, -0.24, -1.00),
    (0.035, -0.16, -1.00), (0.01, -0.22, -1.00),
    (-0.03, -0.13, -1.00), (-0.01, -0.22, -1.00),
    (0.04, 0.59, -0.81), (-0.04, 0.59, -0.81),
)
BRAKE = (
    (0.72, 0.57, -0.51), (0.18, 0.66, 0.73),
    (-0.69, 0.57, -0.49), (-0.19, 0.71, 0.70),
    (0.14, 0.31, -0.97), (0.06, -0.45, -0.84),
    (-0.20, 0.46, -0.89), (-0.04, -0.58, -0.78),
    (0.10, 0.96, -0.18), (-0.10, 0.95, -0.19),
)


def components(rotation):
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


def alignment_rotation(before, after):
    a, b = unit(before), unit(after)
    dot = sum(x * y for x, y in zip(a, b))
    if dot < -0.9999:
        perpendicular = (0, -a[2], a[1]) if abs(a[0]) > 0.5 else (-a[2], 0, a[0])
        return (*unit(perpendicular), 0.0)
    return unit((a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2],
                 a[0]*b[1] - a[1]*b[0], 1.0 + dot))


def smoothstep(t):
    t = max(0.0, min(t, 1.0))
    return t * t * (3.0 - 2.0 * t)


def sample_keys(base, parents, pitches, directions, amplitude, frames, braking):
    keys = {bone: [] for bone in parents}
    base_rotations = {bone: components(pose.rotation) for bone, pose in base.items()}
    for frame in range(frames + 1):
        progress = frame / frames
        # Brake opens quickly, then releases a little tension and holds that
        # stance while the player continues braking. Speed loops close exactly.
        motion = smoothstep(progress / 0.36) - 0.42 * smoothstep((progress - 0.36) / 0.64) if braking else (
            0.0 if frame == frames else math.sin(2.0 * math.pi * progress))
        rotations = {}
        for bone, chain in parents.items():
            parent = (0.0, 0.0, 0.0, 1.0)
            for ancestor in chain:
                parent = multiply(parent, rotations.get(ancestor, base_rotations[ancestor]))
            current = multiply(parent, base_rotations[bone])
            if bone in pitches:
                extra_pitch = amplitude * motion if bone == 'spine_03' else 0.0
                half = math.radians(pitches[bone] + extra_pitch) / 2.0
                delta = (math.sin(half), 0.0, 0.0, math.cos(half))
            else:
                offset = base[CHILDREN[bone]].translation
                aim = multiply(multiply(current, (offset.x, offset.y, offset.z, 0.0)), inverse(current))[:3]
                x, y, z = directions[bone]
                delta = alignment_rotation(aim, (x, y + amplitude * 0.018 * motion, z))
            rotation = unit(multiply(inverse(parent), multiply(delta, current)))
            rotations[bone] = rotation
            keys[bone].append(ue.Quat(*rotation))
    return keys


def main():
    source = ue.load_asset(SOURCE_PATH)
    assert source is not None, SOURCE_PATH
    library = ue.AnimationLibrary
    assets = ue.EditorAssetLibrary
    bones = [str(bone) for bone in library.get_animation_track_names(source)]
    parents = {bone: [str(parent) for parent in library.find_bone_path_to_root(source, bone)][1:][::-1]
               for bone in (*SPINE, *CHILDREN)}
    assert set(parents) <= set(bones), set(parents) - set(bones)
    needed = set(bones) | set(CHILDREN.values()) | {bone for chain in parents.values() for bone in chain}
    base = {bone: library.get_bone_pose_for_frame(source, bone, 0, False) for bone in needed}

    profiles = (
        ('A_FlightSpeed2', (-2.0, -2.0, 6.0, 3.0), SPEED_2, 1.3, 24),
        ('A_FlightSpeed3', (-3.0, -4.0, 8.0, 5.0), SPEED_3, 1.0, 24),
        ('A_FlightSpeed4', (-4.0, -5.0, 10.0, 6.0), SPEED_4, 0.8, 24),
        ('A_FlightSpeed5', (-5.0, -6.0, 11.0, 7.0), SPEED_5, 0.6, 24),
        ('A_FlightBrake', (6.0, 8.0, -4.0, -5.0), BRAKE, 7.0, 12),
    )
    for name, pitch, directions, amplitude, frames in profiles:
        assert len(directions) == len(CHILDREN), name
        braking = name == 'A_FlightBrake'
        keys = sample_keys(base, parents, dict(zip(SPINE, pitch)), dict(zip(CHILDREN, directions)),
                           amplitude, frames, braking)
        path = FOLDER + name
        clip = assets.load_asset(path) if assets.does_asset_exist(path) else assets.duplicate_asset(SOURCE_PATH, path)
        assert clip is not None, path
        controller = clip.controller
        controller.open_bracket(f'Author {name}', False)
        try:
            controller.set_frame_rate(ue.FrameRate(FPS, 1), False)
            controller.set_number_of_frames(ue.FrameNumber(frames), False)
            for bone in bones:
                pose = base[bone]
                rotations = keys.get(bone, [pose.rotation])
                assert controller.set_bone_track_keys(bone, [pose.translation] * len(rotations), rotations,
                                                      [pose.scale3d] * len(rotations), False), (name, bone)
        finally:
            controller.close_bracket(False)

        for notify in library.get_animation_notify_event_names(clip):
            library.remove_animation_notify_events_by_name(clip, notify)
        clip.set_editor_property('enable_root_motion', False)
        assert library.get_num_frames(clip) == frames, name
        assert assets.save_loaded_asset(clip), name
        ue.log(f'IB FLIGHT SPEED CLIP SAVED: {clip.get_path_name()} ({frames / FPS:.2f} s, in place)')


main()
