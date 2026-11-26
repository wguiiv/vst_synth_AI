# tapSynth

Sintetizador subtrativo em JUCE 8.0.9 (VST3; AU no macOS) com um recurso de IA que transforma uma descrição de timbre em parâmetros do plugin. Este repositório também é o objeto de um estudo de TCC sobre testes automatizados de áudio em plugins.


## Compilar

Requer CMake 3.22+ e um compilador C++17. O JUCE é baixado na configuração (use `-DTAPSYNTH_JUCE_DIR=/caminho/para/JUCE` para uma cópia local).

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

O VST3 fica em `build/tapSynth_artefacts/Release/VST3/`. No Linux, as dependências estão em `.github/workflows/build.yml`.

## Recurso de IA

Defina a variável de ambiente `GEMINI_API_KEY` antes de abrir o host. A chave nunca é gravada no repositório.

## Testes

```bash
cmake --build build --target tapSynth_tests
ctest --test-dir build --output-on-failure
```

Os testes ficam em `tests/` (GoogleTest) e renderizam áudio pelo processador real, sem editor.

