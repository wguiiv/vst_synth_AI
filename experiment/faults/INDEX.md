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

## Alterações inofensivas (`experiment/harmless/`)

| id | tipo | origem | arquivo alvo | fonte |
|----|------|--------|--------------|-------|
| H01 | refatoracao | projeto | Source/Data/OscData.cpp | não se aplica |
| H02 | refatoracao | projeto | Source/Data/MeterData.cpp | não se aplica |
| H03 | refatoracao | projeto | Source/SynthVoice.cpp | não se aplica |
| H04 | refatoracao | projeto | Source/PluginProcessor.cpp | não se aplica |
