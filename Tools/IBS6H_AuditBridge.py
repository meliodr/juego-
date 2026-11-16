"""Read-only API evidence audit. This is NOT a C++/UHT compile or world test."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--s35', required=True, type=Path)
    parser.add_argument('--s4', required=True, type=Path)
    parser.add_argument('--engine', required=True, type=Path)
    args = parser.parse_args()
    sources = [
        (args.candidate / 'ImpactBeyond/Source/ImpactBeyond/IB_S3/IBS3World.h',
         ['bool CanTraverse(', 'bool IsOperational()', 'void UpdateResidency(']),
        (args.candidate / 'ImpactBeyond/Source/ImpactBeyond/ImpactCircuitComponent.h', ['bool IsCarHeld() const']),
        (args.s35 / 'ImpactBeyond/Source/ImpactBeyond/IB_S35/IBS35GripComponent.h', ['bool TryGrab(', 'void Drop()', 'GetHeld() const']),
        (args.s35 / 'ImpactBeyond/Source/ImpactBeyond/IB_S35/IBS35ReceiverComponent.h', ['bool Receive(FIBS35DamageMessage&']),
        (args.s4 / 'ImpactBeyond/Source/ImpactBeyond/IB_S4/IBS4PowerComponent.h', ['bool IsHolding() const']),
        (args.engine / 'Engine/Source/Runtime/Engine/Classes/Engine/World.h',
         ['OverlapBlockingTestByChannel(', 'SweepSingleByChannel(', 'LineTraceSingleByChannel(']),
        (args.engine / 'Engine/Source/Runtime/Engine/Classes/Components/InputComponent.h', ['IsBoundToObject(', 'FInputKeyBinding& BindKey(']),
        (args.engine / 'Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h', ['IgnoreComponentWhenMoving(']),
        (args.engine / 'Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h', ['SetActorLocation(FVector', 'RemoveInstanceComponent(']),
    ]
    evidence = []
    for path, signatures in sources:
        content = path.read_text(encoding='utf-8-sig')
        missing = [s for s in signatures if s not in content]
        if missing:
            raise RuntimeError(f'Missing signatures in {path}: {missing}')
        evidence.append({'file': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'present': signatures})
    bridge = ROOT / 'ImpactBeyond/Source/ImpactBeyond/IB_S6H'
    header = (bridge / 'IBS6HBridge.h').read_text()
    includes = [line for line in header.splitlines() if line.startswith('#include')]
    if not includes[-1].endswith('IBS6HBridge.generated.h"'):
        raise RuntimeError('UHT generated include must be last')
    result = {'status': 'API_SIGNATURE_AUDIT_PASS_ONLY', 'unreal_compilation': 'NOT_RUN',
              'world_collision': 'NOT_RUN', 'dependencies': evidence,
              'bridge_sha256': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(bridge.glob('*'))}}
    (ROOT / 'docs/IB_S6H_BRIDGE_API_AUDIT.json').write_text(json.dumps(result, indent=2)+'\n')
    print(f'PASS: {len(sources)} real headers inspected; generated include order checked. No Unreal compile/run.')


if __name__ == '__main__':
    main()
