# Moji Robot - ESP32

Guía rápida para compilar, subir firmware y abrir el monitor serial con PlatformIO.

## Compilar

### VS Code

- Guarda los cambios.
- En PlatformIO, haz clic en **Build**.

### Terminal

- `pio run`

## Subir firmware

### VS Code

- Conecta la ESP32 por USB.
- Verifica el puerto en [platformio.ini](platformio.ini).
- En PlatformIO, haz clic en **Upload**.
- Si hace falta, presiona **BOOT** durante la carga.

### Terminal

- `pio run -t upload`

## Abrir monitor serial

### VS Code

- En PlatformIO, haz clic en **Monitor**.

### Terminal

- `pio device monitor`

## Si cambia el puerto COM

Solo cambia estas líneas en [platformio.ini](platformio.ini):

- `upload_port = COM3`
- `monitor_port = COM3`

Reemplaza `COM3` por el puerto correcto, por ejemplo `COM4` o `COM5`.
