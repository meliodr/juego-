# Herramientas locales de IMPACT: BEYOND

## IB-W00 — comprobar Windows y Unreal sin modificar el equipo

`Check-WindowsUnreal.ps1` utiliza comandos **incluidos con Windows y PowerShell** para comprobar:

- edición y versión de Windows, CPU y memoria;
- nombre y versión del controlador gráfico (no infiere VRAM desde un campo de 32 bits);
- ubicaciones comunes y registradas de UnrealEditor.exe;
- Visual Studio detectado con vswhere, compilador MSVC x64 y versiones de Windows SDK;
- espacio disponible de la unidad del checkout y número de cambios Git, sin enumerar rutas de archivos modificados.

**No descarga, instala, edita registro, cambia drivers ni abre puertos.** No abre Unreal; una instalación detectada no es una prueba de que el editor funcione.

Para ejecutarlo **en la máquina Windows con el checkout de este repositorio**, desde PowerShell:

```powershell
& ".\Tools\Check-WindowsUnreal.ps1"
```

Si Windows bloquea scripts por su política de ejecución, no cambies la política global automáticamente. Un agente autorizado puede inspeccionar el archivo y ejecutar cada comprobación equivalente por separado. No utilices un comando de red no revisado para instalar nada.

La salida JSON se imprime en pantalla y no se guarda sola. **Revísala antes de compartirla:** puede incluir la ruta de instalación del motor u otros datos del equipo. No copies información privada al GitHub público.

### Interpretación

- `UnrealEditorInstallationsFound`: instalación encontrada, **no** significa editor abierto.
- `DirectX12DllDetected`: archivo del sistema detectado, **no** significa que el GPU haya superado la prueba DX12.
- `X64CompilerDetected`: `cl.exe` encontrado, **no** garantiza que UnrealBuildTool compile.
- `WindowsSdkVersions`: directorios del SDK, **no** confirma que todas las cabeceras/libs estén completas.

**Después**, cuando se confirme el editor y compilador: abrir Unreal, crear **Third Person C++** con la plantilla oficial, hacer Play y empaquetar Windows. Solo entonces pasar IB-W00/IB-W01 a COMPILADO/EJECUTADO. No crear otro motor de movimiento antes de reutilizar la plantilla.

Fuente de verdad: [Plan de Windows](../docs/WINDOWS_INICIO_V12.md), [Plan activo](../docs/PLAN_ACTIVO.md), [Cola de tareas](../docs/TAREAS_ACTIVAS.md).

## Próximo paso técnico

**IB-W01** reutilizará directamente Third Person de Epic. Después, el sprint de vuelo adaptará únicamente la lógica necesaria del C++ MIT de ManOfSteel, comprobando por separado la procedencia y disponibilidad real de animaciones. **No se ha probado aún en este equipo**.
