# Guía Completa: Cómo Pasar y Compilar LAUTA FX en tu Mac

Esta guía te explica paso a paso cómo llevar este proyecto a tu **Mac**, compilar los plugins en formatos **Audio Unit (AU - `.component`)** y **VST3 (`.vst3`)**, e instalarlos en **Ableton Live**.

---

## 📦 Paso 1: Qué archivos pasar a tu Mac

En tu computadora actual (Windows), comprime o copia la carpeta `plugin` en un pendrive o envíala por AirDrop / Drive / red.

> **💡 Consejo importante:**  
> No hace falta copiar la carpeta `build/` (esa contiene los archivos compilados de Windows).  
> Solo necesitas copiar:
> - `CMakeLists.txt`
> - La carpeta `Source/`
> - La carpeta `external/` (contiene JUCE)
> - La carpeta `Tests/`
> - `README.md`

---

## 🛠️ Paso 2: Requisitos previos en tu Mac (se hace una sola vez)

Abre la **Terminal** en tu Mac (puedes buscar "Terminal" con `Cmd + Espacio` en Spotlight) y asegúrate de tener las herramientas de desarrollo:

1. **Herramientas de compilación de Apple (Xcode Command Line Tools):**
   ```bash
   xcode-select --install
   ```
   *(Si ya las tienes instaladas, te dirá que ya están al día).*

2. **CMake y Ninja (con Homebrew):**
   Si usas Homebrew en tu Mac, instala CMake y Ninja ejecutando:
   ```bash
   brew install cmake ninja
   ```
   *(Si no tienes Homebrew, puedes descargar el instalador oficial de CMake desde [cmake.org](https://cmake.org/download/)).*

---

## 🚀 Paso 3: Compilar los plugins en tu Mac

1. En la **Terminal**, entra en la carpeta del proyecto que copiaste. Por ejemplo:
   ```bash
   cd ~/Downloads/plugin
   # o donde hayas colocado la carpeta
   ```

2. Ejecuta la configuración con **CMake**:
   ```bash
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```
   *(Esto detectará automáticamente macOS y configurará tanto el VST3 como el Audio Unit de Apple).*

3. Compila los plugins:
   ```bash
   cmake --build build --config Release
   ```

¡Listo! Cuando termine la compilación (tarda 1 o 2 minutos), se habrán creado los siguientes archivos dentro de tu Mac:

- **Audio Unit (AU):** `build/LautaFX_artefacts/Release/AU/LAUTA FX.component`
- **VST3:** `build/LautaFX_artefacts/Release/VST3/LAUTA FX.vst3`
- **Standalone:** `build/LautaFX_artefacts/Release/Standalone/LAUTA FX.app`

---

## 📂 Paso 4: Instalar los plugins en macOS

Para que Ableton Live los reconozca automáticamente, copia los archivos generados a las carpetas estándar de plugins de macOS ejecutando estos dos comandos en la Terminal:

```bash
# Copiar el VST3
cp -R "build/LautaFX_artefacts/Release/VST3/LAUTA FX.vst3" ~/Library/Audio/Plug-Ins/VST3/

# Copiar el Audio Unit (.component)
cp -R "build/LautaFX_artefacts/Release/AU/LAUTA FX.component" ~/Library/Audio/Plug-Ins/Components/
```

> **Nota de seguridad de macOS (Gatekeeper):**  
> Como es un plugin recién compilado por ti mismo en tu máquina local, si macOS o Ableton te muestran un aviso de seguridad al escanearlo, ejecuta este comando en la Terminal para darle permiso total:
> ```bash
> xattr -cr ~/Library/Audio/Plug-Ins/VST3/"LAUTA FX.vst3"
> xattr -cr ~/Library/Audio/Plug-Ins/Components/"LAUTA FX.component"
> ```

---

## 🎛️ Paso 5: Abrir y Usar en Ableton Live (Mac)

1. Abre **Ableton Live**.
2. Ve a **Live** > **Settings / Preferences** > pestaña **Plug-Ins**:
   - Asegúrate de tener activadas las opciones:
     - *Use Audio Units (AU)*
     - *Use VST3 Plug-In System Folders*
   - Si no aparece de inmediato, mantén presionada la tecla `Option` (Alt) y haz clic en **Rescan**.
3. En el navegador de la izquierda de Ableton, ve a:
   - **Plug-Ins** > **Audio Units** (o **VST3**) > **Lauta Audio** > **LAUTA FX**.
4. Arrastra **LAUTA FX** a la pista **Master** (o al canal que desees procesar).

---

## 🎹 Mapeo MIDI para Live Performance

Para controlar los 4 macros desde tu controlador físico (Push, APC40, MIDI Fighter Twister, Novation Launch Control, etc.):

1. En Ableton Live presiona `Cmd + M` para entrar en el modo **MIDI Map** (la pantalla se pondrá azul/morada).
2. Haz clic en el control **ENERGY** de LAUTA FX y gira la perilla 1 de tu controlador físico.
3. Haz clic en **SPACE** y gira la perilla 2.
4. Haz clic en **MOVEMENT** y gira la perilla 3.
5. Haz clic en **WILD** y gira la perilla 4.
6. Presiona `Cmd + M` nuevamente para salir del modo de mapeo.

---

## 🎚️ Resumen de los 4 Macros en vivo

- **`ENERGY`**: Crea tensión y barridos de filtro para anticipar subidas y drops (corta subgraves progresivamente y añade saturación cálida).
- **`SPACE`**: Genera una atmósfera espacial profunda combinando delay estéreo y reverb de difusión. Al soltarlo a cero, las colas de delay y reverb continúan sonando con decaimiento natural (no se cortan abruptamente).
- **`MOVEMENT`**: Incrementa el feedback del delay y a partir del 35% activa el looper de repetición rítmica en sincronía con el tempo de Ableton.
- **`WILD`**: Introduce variaciones de repetición rítmica más rápidas (1/16 y 1/32) con degradado y saturación extra para transiciones agresivas y stutters controlados.
- **`RESET MACROS`**: Botón en la interfaz para regresar instantáneamente los 4 macros a 0 sin cortes bruscos de audio.
