# IB-F01 — DualSense en el paquete Win64

Base revisada: b65f9a6. Entorno objetivo: UE 5.8.4, Win64 Development, DX12, Lvl_ThirdPerson. El procedimiento reutiliza Steam Input y las asignaciones de gamepad existentes. Coste adicional previsto: RD$0.

## Evidencia de solo lectura

- `Get-PnpDevice -PresentOnly` enumeró interfaces de juego HID Sony USB (054C:0CE6) y Bluetooth en estado OK. La enumeración confirma presencia, pero queda por comprobar la entrega de eventos de juego.
- Un proceso PowerShell independiente llamó a `xinput1_4.dll!XInputGetState` para los índices 0–3. Los cuatro devolvieron 1167 (dispositivo no conectado). Steam estaba en ejecución. El resultado mide la visibilidad XInput desde el proceso de sondeo. Valve documenta que la emulación puede operar dentro del proceso de juego mediante el overlay.
- La lógica de vuelo escucha teclas estándar de gamepad: pulsaciones R3/R1/L1, estado de Cuadrado y valores analógicos de R2/L2. L3 se consulta al procesar movimiento en suelo. Las entradas HID genéricas de RawInput por sí solas no alimentan esas consultas. Se aplaza activar el plugin experimental de Epic hasta medir la ruta de Steam.

## Prueba física pendiente

1. Usar el paquete Win64 Development existente y conectar el DualSense por USB de datos. Para el primer ensayo usar un solo mando y un solo transporte.
2. Abrir el paquete directamente una vez y probar ambos sticks y Cruz. Si ya responde, ejecutar la tabla completa en esa ruta antes de configurar Steam.
3. Si el lanzamiento directo falla, cerrar el juego. En el Steam ya instalado, añadir el paquete como juego externo. Si el ejecutable de la raíz abre un segundo proceso y Steam pierde el seguimiento, apuntar el acceso directo al binario de juego en `ImpactBeyond/Binaries/Win64` del mismo paquete, con su carpeta como directorio de trabajo.
4. En la configuración de mando de ese acceso directo, activar Steam Input y seleccionar la plantilla **Gamepad**. Verificar en la prueba de entradas de Steam que ambos sticks, los dos gatillos y los botones responden. Lanzar desde la biblioteca y confirmar que el overlay abre dentro del juego si no hay respuesta. En el perfil deben emitirse controles de gamepad y conservarse los gatillos analógicos.
5. En Lvl_ThirdPerson, verificar la tabla. Anotar lanzamiento directo/Steam, USB/Bluetooth, resultado por fila y evidencia de cualquier pulso perdido o duplicado. Para Bluetooth repetir con el cable desconectado y el mando ya emparejado.

| Entrada física | Respuesta esperada |
| --- | --- |
| Stick izquierdo y derecho | Desplazamiento y cámara, con reposo al soltar |
| Cruz y L3 | Salto en suelo y esprint mientras se mueve con L3 sostenido |
| R3 | Un cambio suelo/aire por pulsación |
| R1 y L1 | Un nivel arriba/abajo por pulsación dentro de los límites |
| R2 y L2 | Ascenso y descenso proporcionales, con reposo vertical al soltar |
| Cuadrado | Freno mientras se mantiene pulsado en vuelo |

Tras la prueba, cerrar el paquete. Para revertir la configuración, eliminar el acceso directo externo de la biblioteca. La prueba no requiere recompilar, instalar software ni editar el personaje.

**Decisión IB-F01** — CONTINUAR con la validación física. Se verificó la presencia HID y el resultado XInput del proceso de sondeo. Quedan por observar la emulación dentro del paquete y la respuesta completa de los controles. No se abrió el editor ni el juego durante esta auditoría.

Fuentes: [Emulación de gamepad en Steam Input](https://partner.steamgames.com/doc/features/steam_controller/steam_input_gamepad_emulation_bestpractices), [acceso directo a un juego externo](https://help.steampowered.com/en/faqs/view/4B8B-9697-2338-40EC), [plugin Windows RawInput de Epic](https://dev.epicgames.com/documentation/unreal-engine/API/PluginIndex/RawInput).
