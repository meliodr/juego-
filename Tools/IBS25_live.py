"""Temporary local editor job queue for the coordinated S25 validation session.
No server/port. Only IBS25_*.py in this Tools directory; sequence IDs run once.
The queue dies with the editor. It never changes project/plugin configuration.
"""
import json
from pathlib import Path
import sys
import traceback
import unreal as ue
sys.dont_write_bytecode = True
# ExecutePythonScript otherwise queues QUIT_EDITOR after this file returns.
# Native UE API (ScriptName="EditorPythonScripting"); process-local only.
ue.EditorPythonScripting.set_keep_python_script_alive(True)
ROOT = Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
REQUEST = ROOT/'IBS25_jobs'/'request.json'
seen = set()
elapsed = 0

def tick(delta):
    global elapsed
    elapsed += delta
    if elapsed < 1 or not REQUEST.exists():
        return
    elapsed = 0
    request = json.loads(REQUEST.read_text())
    seq,name = request['seq'],request['script']
    if seq in seen:
        return
    seen.add(seq)
    if not name.startswith('IBS25_') or Path(name).name != name or not name.endswith('.py'):
        raise ValueError('Invalid scoped job')
    result = {'seq':seq,'script':name,'status':'STARTED'}
    try:
        path = ROOT/name
        exec(compile(path.read_text(encoding='utf-8'),str(path),'exec'),{'__name__':'__main__','__file__':str(path)})
        result['status']='EXECUTED'
    except Exception:
        result['status']='FAILED'
        result['error']=traceback.format_exc()
        ue.log_error(result['error'])
    (ROOT/'IBS25_jobs'/f'result_{seq}.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    ue.log('IBS25_JOB '+json.dumps(result))

HANDLE=ue.register_slate_post_tick_callback(tick)
ue.log('IBS25 scoped local queue ready')
