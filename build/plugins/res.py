from _common import iterpair
from _common import listid, pathid


def split(lst, limit):
    # paths are specified with replaceable prefix
    # real length is unknown at the moment, that why we use root_lenght
    # as a rough estimation
    root_lenght = 200
    filepath = None
    lenght = 0
    bucket = []

    for item in lst:
        if filepath:
            lenght += root_lenght + len(filepath) + len(item)
            if lenght > limit and bucket:
                yield bucket
                bucket = []
                lenght = 0

            bucket.append(filepath)
            bucket.append(item)
            filepath = None
        else:
            filepath = item

    if bucket:
        yield bucket


def onresource(unit, *args):
    unit.onpeerdir(['library/resource'])

    # Since the maximum length of lpCommandLine string for CreateProcess is 8kb (windows) characters,
    # we make several calls of rescompiler
    # https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms682425.aspx
    for part_args in split(args, 8000):
        output = listid(part_args) + '.cpp'
        inputs = [x for x, y in iterpair(part_args) if x != '-']
        if inputs:
            inputs = ['IN'] + inputs

        unit.onrun_program(['tools/rescompiler', output] + part_args + inputs + ['OUT_NOAUTO', output])
        unit.onsrcs(['GLOBAL', output])


def onro_resource(unit, *args):
    unit.onpeerdir(['library/resource'])

    for part_args in split(args, 8000):
        srcs_gen = []
        for p, n in iterpair(part_args):
            if p == '-':
                continue
            lid = '_' + pathid(p)
            compressed_out = lid + '.rodata'
            unit.onrun_program(['tools/rescompressor', p, compressed_out, 'IN', p, 'OUT', compressed_out])
            srcs_gen.append('{}={}'.format(n, lid))
        output = listid(part_args) + '.cpp'
        unit.onrun_program(['tools/rorescompiler', output] + srcs_gen + ['OUT_NOAUTO', output])
        unit.onsrcs(['GLOBAL', output])


def onfrom_sandbox(unit, *args):
    unit.onsetup_from_sandbox(list(args))
    res_id = args[0]
    if res_id == "FILE":
        res_id = args[1]
    unit.onadd_check(["check.resource", res_id])
