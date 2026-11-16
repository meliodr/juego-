"""Offline import/soft-reference reader for this lab's uncooked UE packages.

Not an AssetRegistry, object deserializer or cook validator. Version/offset checks
fail closed. Layout checked against the locally installed UE 5.8 sources:
PackageFileSummary.cpp, ObjectResource.cpp and ObjectVersion.h. No engine execution.
"""
import struct


class Reader:
    def __init__(self, data, offset=0):
        self.data, self.offset = data, offset

    def take(self, size):
        if size < 0 or self.offset < 0 or self.offset + size > len(self.data):
            raise ValueError('package range outside file')
        value = self.data[self.offset:self.offset + size]
        self.offset += size
        return value

    def integer(self):
        return struct.unpack('<i', self.take(4))[0]

    def string(self):
        n = self.integer()
        if abs(n) > 1048576:
            raise ValueError('oversized FString')
        if not n:
            return ''
        raw = self.take(abs(n) * (2 if n < 0 else 1))
        end = b'\0\0' if n < 0 else b'\0'
        if not raw.endswith(end):
            raise ValueError('unterminated FString')
        return raw[:-len(end)].decode('utf-16-le' if n < 0 else 'utf-8')


def references(data):
    r = Reader(data)
    if r.take(4) != b'\xc1\x83\x2a\x9e':
        raise ValueError('not an Unreal package (or LFS pointer)')
    legacy = r.integer()
    if legacy not in (-7, -8, -9):
        raise ValueError('unsupported legacy package version')
    r.integer()  # legacy UE3
    ue4 = r.integer()
    ue5 = r.integer() if legacy <= -8 else 0
    licensee = r.integer()
    if ue4 != 522 or ue5 > 1018 or licensee:
        raise ValueError('unsupported package version')
    if ue5 >= 1016:
        r.take(20)
        header = r.integer()
    custom_count = r.integer()
    if not 0 <= custom_count <= 10000:
        raise ValueError('invalid custom version count')
    r.take(custom_count * 20)
    if ue5 < 1016:
        header = r.integer()
    saved_name = r.string()
    flags = r.integer() & 0xffffffff
    if flags & (0x80000000 | 0x200):
        raise ValueError('cooked/editor-filtered packages unsupported')
    nc, no = r.integer(), r.integer()
    sc, so = (r.integer(), r.integer()) if ue5 >= 1008 else (0, 0)
    r.string()  # localization id (UE4 522, editor data)
    r.take(8)  # gatherable text
    ec, eo, ic, io = (r.integer() for _ in range(4))
    if ue5 >= 1015:
        r.take(16)  # Verse cell export/import tables
    if ue5 >= 1014:
        r.take(4)  # metadata offset
    r.take(4)  # depends offset
    pc, po = r.integer(), r.integer()
    stride = 40 if ue5 >= 1003 else 36
    if not 0 < header <= len(data):
        raise ValueError('invalid header size')
    for count, offset, minimum in ((nc,no,8), (ic,io,stride), (sc,so,20), (pc,po,8)):
        if count < 0 or count > 1000000 or (count and (offset < r.offset or offset + count * minimum > header)):
            raise ValueError('invalid table bounds')
    n = Reader(data, no)
    names = []
    for _ in range(nc):
        names.append(n.string())
        n.take(4)  # serialized name hashes
    if n.offset > header:
        raise ValueError('names exceed header')

    def fname(reader):
        index, number = reader.integer(), reader.integer()
        if not 0 <= index < len(names) or number < 0:
            raise ValueError('invalid FName')
        return names[index] + ('_' + str(number - 1) if number else '')

    imports, objects = set(), []
    ir = Reader(data, io)
    for _ in range(ic):
        cp, cn, outer, obj = fname(ir), fname(ir), ir.integer(), fname(ir)
        package = fname(ir)
        optional = ir.integer() if ue5 >= 1003 else 0
        if optional not in (0, 1) or not -ic <= outer <= ec:
            raise ValueError('invalid import record')
        if cn == 'Package' and obj.startswith('/'):
            imports.add(obj)
        if package.startswith('/'):
            imports.add(package)
        objects.append({'class_package': cp, 'class': cn, 'outer': outer,
                        'name': obj, 'package': package, 'optional': bool(optional)})
    soft = set()
    sr = Reader(data, so)
    for _ in range(sc):
        pkg, obj, sub = fname(sr), fname(sr), sr.string()
        if pkg.startswith('/'):
            soft.add(pkg)
    pr = Reader(data, po)
    for _ in range(pc):
        pkg = fname(pr)
        if pkg.startswith('/'):
            soft.add(pkg)
    return {'ue4': ue4, 'ue5': ue5, 'saved_name': saved_name,
            'imports': sorted(imports), 'soft_packages': sorted(soft),
            'import_objects': objects}
