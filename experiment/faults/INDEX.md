# Índice dos patches

Cada patch aplica sozinho sobre o código limpo. Os patches de origem `projeto` aplicam na raiz deste repositório (`git apply`), e os de origem `juce` aplicam na raiz de um clone do JUCE 8.0.9 (`git -C /caminho/para/JUCE apply`).

## Defeitos

| id | categoria | origem | arquivo alvo | fonte |
|----|-----------|--------|--------------|-------|
| F01 | altura | projeto | Source/Data/OscData.cpp | https://github.com/TheWaveWarden/odin2/commit/8cd9fa4002548b9095531ba49757b9f9d33eee28 |
| F02 | nivel | projeto | Source/PluginProcessor.cpp | https://github.com/juce-framework/JUCE/commit/3a2be25b9b404cb6941ab06c2b035e7001951bf1 |
| F03 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/juce-framework/JUCE/commit/088dd14037cb571ba4f08dcf0fd05a1c754ade68 |
| F04 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/juce-framework/JUCE/commit/dd9802e265fc9e90f8bd7b2bc3a7160b69111508 |
| F05 | filtro | projeto | Source/SynthVoice.cpp | https://github.com/sweetorange1/Organic-Chemistry/commit/ddbe96c95f11ea63a27a3655efa1f765db7ab80a |
| F06 | numerico | projeto | Source/PluginProcessor.cpp | https://github.com/asb2m10/dexed/commit/a428fe8a51edc23cca33ac42b410a9c470031576 |
| F07 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/juce-framework/JUCE/commit/a933d7c4d0a9979de941475b02892b1c756e25e1 |
| F08 | nivel | juce | modules/juce_audio_basics/utilities/juce_Reverb.h | https://github.com/juce-framework/JUCE/commit/7273838d8db2ff60b6112fda9f15bc19368629b9 |
| F09 | envelope | juce | modules/juce_audio_basics/utilities/juce_Reverb.h | https://github.com/juce-framework/JUCE/commit/3262a3c5480dbefdc09776c805e18e23e9abe806 |
| F10 | estado | projeto | Source/SynthVoice.cpp | https://github.com/juce-framework/JUCE/commit/9a130f2810284cfee42a2f81c926303b152e4af5 |
| F11 | numerico | projeto | Source/Data/MeterData.cpp | https://github.com/diamondjirapat/LufsNormalizer/commit/8d38625b518ebf3c642c07904a68deadeebdbac9 |
| F12 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/juce-framework/JUCE/commit/5d3854732b22efa83bfce5f6de7d8fe9f4410a05 |
| F13 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/juce-framework/JUCE/commit/792f992dfc5636543a976092c4f101acac5a92db |
| F14 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/juce-framework/JUCE/commit/efe197de398231dfec194bac5748bdadf8f7d49b |
| F15 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/TheWaveWarden/odin2/commit/2e2f2c1527965372bb225f1eb990a882ae453f92 |
| F16 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/mtytel/helm/commit/39f3c2728283bbfc45cf1ca00ce9d5dda9787d80 |
| F17 | envelope | juce | modules/juce_audio_basics/utilities/juce_ADSR.h | https://github.com/sfztools/sfizz/commit/844aa1a6c29e23bdc49d60793b261e340b7e20b4 |
| F18 | filtro | juce | modules/juce_dsp/processors/juce_StateVariableTPTFilter.cpp | https://github.com/mtytel/helm/commit/8475fd5cdb191c3b541d72fb99d0c3a01f6c9f2e |
| F19 | nivel | juce | modules/juce_dsp/widgets/juce_Gain.h | https://github.com/juce-framework/JUCE/commit/dc7217fbbb4fd5541f62a049b838b137f1fc66ed |
| F20 | altura | projeto | Source/Data/OscData.cpp | https://github.com/Tracktion/tracktion_engine/commit/8868f7dce11e9bed0c9b2f997831a6b3f75efc15 |
| F21 | altura | projeto | Source/Data/OscData.cpp | https://github.com/Tracktion/tracktion_engine/commit/18de4ff3ebd952b5dc54e7976521b350b366d13f |
| F23 | estado | projeto | Source/Data/OscData.h | https://github.com/surge-synthesizer/monique-monosynth/commit/bc296c0d6bd78bc8721e458e5085bd4533bd9f5b |
| F24 | envelope | projeto | Source/SynthVoice.cpp | https://github.com/Tracktion/tracktion_engine/commit/404b546f053025c65b5464b676e8541dd54889df |
| F25 | envelope | projeto | Source/SynthVoice.cpp | https://github.com/amsynth/amsynth/commit/6ed11bec0016be8565ecd12fee452e907b4130f9 |
| F26 | envelope | projeto | Source/SynthVoice.cpp | https://github.com/mtytel/helm/commit/39280059621c32d5dfb130cf5ce30c6b3fbc1e47 |
| F28 | estado | projeto | Source/SynthVoice.cpp | https://github.com/asb2m10/dexed/commit/2c2732816f15b304ae3e7e8478ab74578d70e2c6 |
| F29 | filtro | projeto | Source/Data/FilterData.cpp | https://github.com/zynaddsubfx/zynaddsubfx/commit/d4bcd004a6ff49bb195ee9152e82dcb97a875b28 |
| F30 | nivel | projeto | Source/Data/MeterData.cpp | https://github.com/zynaddsubfx/zynaddsubfx/commit/c141f777ba02ab7f6580f194aa86b99d0be366c9 |
| F31 | numerico | projeto | Source/Data/MeterData.cpp | https://github.com/Tracktion/tracktion_engine/commit/8e89fe5d6d4bddf1e9dfbdb9b16a68e660e3798c |
| F32 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/TheWaveWarden/odin2/commit/2921637b0c546f3582f93bf6f8245dbea369f529 |
| F33 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/amsynth/amsynth/commit/81cd6e9f24bdbfd481b5e083ec296150503d60e7 |
| F34 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/sfztools/sfizz/commit/1faa7f0739d0f0c4c020cd0d6c96efe2fb655fba |
| F35 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/surge-synthesizer/surge/commit/f9549f8afb416e42474dcdc4a8e5cf956d9cddbf |
| F36 | estado | projeto | Source/PluginProcessor.cpp | https://github.com/zynaddsubfx/zynaddsubfx/commit/6e838ba43db848a0645e96d8609310b23db7ea7b |

## Alterações inofensivas (`experiment/harmless/`)

| id | tipo | origem | arquivo alvo | fonte |
|----|------|--------|--------------|-------|
| H01 | refatoracao | projeto | Source/Data/OscData.cpp | não se aplica |
| H02 | refatoracao | projeto | Source/Data/MeterData.cpp | não se aplica |
| H03 | refatoracao | projeto | Source/SynthVoice.cpp | não se aplica |
| H04 | refatoracao | projeto | Source/PluginProcessor.cpp | não se aplica |
