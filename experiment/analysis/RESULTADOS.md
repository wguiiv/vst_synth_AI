# Resultados do experimento

Gerado por `experiment/analyze.py` a partir de `experiment/results/`. Não edite à mão: rode o script de novo.
Taxas com intervalo de confiança de 95% de Wilson. Uma abordagem detecta uma condição quando detecta em pelo menos uma execução.

## 1. Validade das execuções

| Tipo | Condições | Situação |
| --- | --- | --- |
| defeito | 11 | ok: 11 |
| inofensiva | 8 | ok: 8 |
| mutante | 225 | ok: 225 |
| referencia | 1 | ok: 1 |

Referência (código limpo no Linux): 5 execuções, 0 falhas nas tolerâncias padrão.

## 2. Detecção por abordagem e por origem (QP1, QP2, QP4)

Detectados (taxa) em cada conjunto. "Possivelmente equivalentes" são os mutantes que nenhuma abordagem detecta nas tolerâncias padrão (66 de 225; seção 7 do protocolo).

| Conjunto | Origem | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| defeitos | todos | 11 | 2 (18,2%) | 2 (18,2%) | 3 (27,3%) | 3 (27,3%) | 1 (9,1%) | 3 (27,3%) | 3 (27,3%) |
| defeitos | projeto | 7 | 2 (28,6%) | 2 (28,6%) | 2 (28,6%) | 2 (28,6%) | 1 (14,3%) | 2 (28,6%) | 2 (28,6%) |
| defeitos | juce | 4 | 0 (0,0%) | 0 (0,0%) | 1 (25,0%) | 1 (25,0%) | 0 (0,0%) | 1 (25,0%) | 1 (25,0%) |
| mutantes | todos | 225 | 100 (44,4%) | 85 (37,8%) | 108 (48,0%) | 153 (68,0%) | 68 (30,2%) | 154 (68,4%) | 159 (70,7%) |
| mutantes | projeto | 94 | 44 (46,8%) | 44 (46,8%) | 44 (46,8%) | 54 (57,4%) | 31 (33,0%) | 54 (57,4%) | 59 (62,8%) |
| mutantes | juce | 131 | 56 (42,7%) | 41 (31,3%) | 64 (48,9%) | 99 (75,6%) | 37 (28,2%) | 100 (76,3%) | 100 (76,3%) |
| mutantes sem os possivelmente equivalentes | todos | 159 | 100 (62,9%) | 85 (53,5%) | 108 (67,9%) | 153 (96,2%) | 68 (42,8%) | 154 (96,9%) | 159 (100,0%) |
| mutantes sem os possivelmente equivalentes | projeto | 59 | 44 (74,6%) | 44 (74,6%) | 44 (74,6%) | 54 (91,5%) | 31 (52,5%) | 54 (91,5%) | 59 (100,0%) |
| mutantes sem os possivelmente equivalentes | juce | 100 | 56 (56,0%) | 41 (41,0%) | 64 (64,0%) | 99 (99,0%) | 37 (37,0%) | 100 (100,0%) | 100 (100,0%) |

Com intervalo de confiança:

| Conjunto | Abordagem | Detectados | Taxa | IC 95% |
| --- | --- | --- | --- | --- |
| defeitos | sanidade | 2/11 | 18,2% | 5,1–47,7% |
| defeitos | espectral | 2/11 | 18,2% | 5,1–47,7% |
| defeitos | temporal | 3/11 | 27,3% | 9,7–56,6% |
| defeitos | golden | 3/11 | 27,3% | 9,7–56,6% |
| defeitos | pluginval | 1/11 | 9,1% | 1,6–37,7% |
| defeitos | qualquer teste | 3/11 | 27,3% | 9,7–56,6% |
| defeitos | qualquer abordagem | 3/11 | 27,3% | 9,7–56,6% |
| mutantes | sanidade | 100/225 | 44,4% | 38,1–51,0% |
| mutantes | espectral | 85/225 | 37,8% | 31,7–44,3% |
| mutantes | temporal | 108/225 | 48,0% | 41,6–54,5% |
| mutantes | golden | 153/225 | 68,0% | 61,6–73,7% |
| mutantes | pluginval | 68/225 | 30,2% | 24,6–36,5% |
| mutantes | qualquer teste | 154/225 | 68,4% | 62,1–74,2% |
| mutantes | qualquer abordagem | 159/225 | 70,7% | 64,4–76,2% |
| mutantes sem os possivelmente equivalentes | sanidade | 100/159 | 62,9% | 55,2–70,0% |
| mutantes sem os possivelmente equivalentes | espectral | 85/159 | 53,5% | 45,7–61,0% |
| mutantes sem os possivelmente equivalentes | temporal | 108/159 | 67,9% | 60,3–74,7% |
| mutantes sem os possivelmente equivalentes | golden | 153/159 | 96,2% | 92,0–98,3% |
| mutantes sem os possivelmente equivalentes | pluginval | 68/159 | 42,8% | 35,3–50,5% |
| mutantes sem os possivelmente equivalentes | qualquer teste | 154/159 | 96,9% | 92,9–98,6% |
| mutantes sem os possivelmente equivalentes | qualquer abordagem | 159/159 | 100,0% | 97,6–100,0% |

## 3. Matriz defeito × abordagem

| Defeito | Origem | Categoria | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F01 | projeto | altura | · | · | · | · | · | · | · |
| F02 | projeto | nivel | ● | ● | ● | ● | · | ● | ● |
| F03 | juce | envelope | · | · | ● | ● | · | ● | ● |
| F04 | juce | envelope | · | · | · | · | · | · | · |
| F05 | projeto | filtro | ● | ● | ● | ● | ● | ● | ● |
| F06 | projeto | numerico | · | · | · | · | · | · | · |
| F07 | projeto | estado | · | · | · | · | · | · | · |
| F08 | juce | nivel | · | · | · | · | · | · | · |
| F09 | juce | envelope | · | · | · | · | · | · | · |
| F10 | projeto | estado | · | · | · | · | · | · | · |
| F11 | projeto | numerico | · | · | · | · | · | · | · |

A matriz dos 225 mutantes está em `matriz_mutantes.csv`.

## 4. Detecção por categoria

Defeitos, por categoria:

| Categoria | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| envelope | 3 | 0/3 | 0/3 | 1/3 | 1/3 | 0/3 | 1/3 | 1/3 |
| estado | 2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 |
| nivel | 2 | 1/2 | 1/2 | 1/2 | 1/2 | 0/2 | 1/2 | 1/2 |
| numerico | 2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 | 0/2 |
| altura | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| filtro | 1 | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 |

Mutantes, por grupo de operador:

| Grupo | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| comparação | 109 | 31/109 | 33/109 | 52/109 | 60/109 | 22/109 | 61/109 | 65/109 |
| aritmético | 85 | 44/85 | 28/85 | 32/85 | 68/85 | 21/85 | 68/85 | 68/85 |
| incremento | 31 | 25/31 | 24/31 | 24/31 | 25/31 | 25/31 | 25/31 | 26/31 |

Mutantes, por arquivo:

| Arquivo | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| modules/juce_audio_basics/utilities/juce_Reverb.h | 78 | 44/78 | 31/78 | 32/78 | 64/78 | 32/78 | 64/78 | 64/78 |
| modules/juce_audio_basics/utilities/juce_ADSR.h | 43 | 5/43 | 4/43 | 27/43 | 28/43 | 2/43 | 29/43 | 29/43 |
| Source/SynthVoice.cpp | 33 | 24/33 | 23/33 | 26/33 | 29/33 | 14/33 | 29/33 | 30/33 |
| Source/PluginProcessor.cpp | 26 | 10/26 | 11/26 | 11/26 | 11/26 | 11/26 | 11/26 | 13/26 |
| Source/Data/MeterData.cpp | 22 | 3/22 | 3/22 | 3/22 | 3/22 | 5/22 | 3/22 | 5/22 |
| Source/Data/OscData.cpp | 13 | 7/13 | 7/13 | 4/13 | 11/13 | 1/13 | 11/13 | 11/13 |
| modules/juce_dsp/widgets/juce_Gain.h | 5 | 2/5 | 1/5 | 1/5 | 2/5 | 1/5 | 2/5 | 2/5 |
| modules/juce_dsp/processors/juce_StateVariableTPTFilter.cpp | 3 | 3/3 | 3/3 | 2/3 | 3/3 | 1/3 | 3/3 | 3/3 |
| modules/juce_dsp/widgets/juce_Oscillator.h | 2 | 2/2 | 2/2 | 2/2 | 2/2 | 1/2 | 2/2 | 2/2 |

## 5. Detecção exclusiva

Condições que só uma abordagem detecta:

| Conjunto | Abordagem | n | Ids |
| --- | --- | --- | --- |
| defeitos | sanidade | 0 | – |
| defeitos | espectral | 0 | – |
| defeitos | temporal | 0 | – |
| defeitos | golden | 0 | – |
| defeitos | pluginval | 0 | – |
| mutantes | sanidade | 1 | M0101 |
| mutantes | espectral | 0 | – |
| mutantes | temporal | 0 | – |
| mutantes | golden | 24 | M0023, M0026, M0085, M0113, M0138, M0141, M0142, M0145, M0152, M0160, M0162, M0165, M0167, M0170, M0174, M0176, M0179, M0181, M0197, M0200, M0204, M0206, M0211, M0214 |
| mutantes | pluginval | 5 | M0002, M0011, M0043, M0044, M0064 |

Mutantes que travam ou estouram o tempo em alguma das quatro estratégias e são detectados por todas elas: 51 de 225.

## 6. Alarmes falsos

Execuções que falharam em cada condição inofensiva, nas tolerâncias padrão:

| Condição | Execuções | sanidade | espectral | temporal | golden | pluginval |
| --- | --- | --- | --- | --- | --- | --- |
| H01 | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| H02 | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| H03 | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| H04 | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| clang | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| debug | 1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| macos | 5 | 0/5 | 0/5 | 0/5 | 0/5 | 0/5 |
| windows | 5 | 0/5 | 0/5 | 0/5 | 0/5 | 0/5 |

|  | sanidade | espectral | temporal | golden | pluginval |
| --- | --- | --- | --- | --- | --- |
| Condições com alarme falso | 0/8 | 0/8 | 0/8 | 0/8 | 0/8 |

## 7. Comparação pareada

Teste Q de Cochran entre as cinco abordagens, e McNemar exato par a par com correção de Holm (10 pares por conjunto).

**defeitos** (n = 11): Q de Cochran = 5,60, gl = 4, p = 0,231.

| Par | Só o 1º detecta | Só o 2º detecta | p | p (Holm) |
| --- | --- | --- | --- | --- |
| sanidade × espectral | 0 | 0 | 1,000 | 1,000 |
| sanidade × temporal | 0 | 1 | 1,000 | 1,000 |
| sanidade × golden | 0 | 1 | 1,000 | 1,000 |
| sanidade × pluginval | 1 | 0 | 1,000 | 1,000 |
| espectral × temporal | 0 | 1 | 1,000 | 1,000 |
| espectral × golden | 0 | 1 | 1,000 | 1,000 |
| espectral × pluginval | 1 | 0 | 1,000 | 1,000 |
| temporal × golden | 0 | 0 | 1,000 | 1,000 |
| temporal × pluginval | 2 | 0 | 0,500 | 1,000 |
| golden × pluginval | 2 | 0 | 0,500 | 1,000 |

Complementar (QP2, fora da correção de Holm): qualquer teste × pluginval: só os testes detectam 2, só o pluginval detecta 0, p = 0,500.

**mutantes** (n = 225): Q de Cochran = 164,63, gl = 4, p = < 0,001.

| Par | Só o 1º detecta | Só o 2º detecta | p | p (Holm) |
| --- | --- | --- | --- | --- |
| sanidade × espectral | 21 | 6 | 0,006 | 0,012 |
| sanidade × temporal | 20 | 28 | 0,312 | 0,312 |
| sanidade × golden | 1 | 54 | < 0,001 | < 0,001 |
| sanidade × pluginval | 37 | 5 | < 0,001 | < 0,001 |
| espectral × temporal | 4 | 27 | < 0,001 | < 0,001 |
| espectral × golden | 0 | 68 | < 0,001 | < 0,001 |
| espectral × pluginval | 24 | 7 | 0,003 | 0,010 |
| temporal × golden | 0 | 45 | < 0,001 | < 0,001 |
| temporal × pluginval | 47 | 7 | < 0,001 | < 0,001 |
| golden × pluginval | 90 | 5 | < 0,001 | < 0,001 |

Complementar (QP2, fora da correção de Holm): qualquer teste × pluginval: só os testes detectam 91, só o pluginval detecta 5, p = < 0,001.

**mutantes sem os possivelmente equivalentes**: idênticos aos de todos os mutantes, porque condições que nenhuma abordagem detecta não entram no Q de Cochran nem no McNemar.


## 8. Curva de tolerância (QP3)

Cada tolerância percorre sua grade com as demais no padrão (seção 6 do protocolo). Os conjuntos têm 11 defeitos, 225 mutantes e 159 mutantes não equivalentes; alarmes falsos contam as 8 condições inofensivas. "Falhas na referência" conta as execuções do código limpo no Linux que falhariam naquele ponto. Gráfico: `curva_tolerancia.svg`.

**spectral.pitch** (espectral, unidade: cents, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 1 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 2 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 5 (padrão) | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 10 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 25 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 50 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.stopband** (espectral, unidade: dB, limite mínimo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 24 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 18 (padrão) | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 12 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 6 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.passband** (espectral, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.25 | 11 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.5 | 2 (18,2%) | 86 (38,2%) | 86 (54,1%) | 0 | 0 |
| 1 (padrão) | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 3 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.aliasing** (espectral, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 1 | 2 (18,2%) | 87 (38,7%) | 87 (54,7%) | 0 | 0 |
| 3 (padrão) | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 6 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 10 | 2 (18,2%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**temporal.time** (temporal, unidade: s, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.002 | 11 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.005 | 11 (100,0%) | 224 (99,6%) | 158 (99,4%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.01 (padrão) | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.02 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.05 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**temporal.release** (temporal, unidade: s, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.005 | 11 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.01 | 3 (27,3%) | 109 (48,4%) | 109 (68,6%) | 0 | 0 |
| 0.02 (padrão) | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.05 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.1 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**temporal.sustain** (temporal, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.1 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.25 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.5 (padrão) | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 1 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 3 | 3 (27,3%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**golden.maxAbsDiff** (golden, unidade: linear, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0 | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 2 (macos, windows) | 0 |
| 1e-06 | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 1 (macos) | 0 |
| 1e-05 | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.0001 (padrão) | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.001 | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.01 | 3 (27,3%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |

Dos 66 mutantes possivelmente equivalentes, 0 são detectados em algum ponto da grade em que o código limpo passa.

## 9. Custo

Tempo de execução de cada abordagem no código limpo (média ± desvio padrão das 5 execuções, em segundos) e linhas de código dos testes (sem linhas em branco e comentários).

| Abordagem | Testes | Linhas de código | Linux (s) | Windows (s) | macOS (s) |
| --- | --- | --- | --- | --- | --- |
| sanidade | 24 | 123 | 6,5 ± 0,2 | 13,4 ± 0,7 | 12,5 ± 0,4 |
| espectral | 21 | 176 | 3,1 ± 0,4 | 4,8 ± 0,3 | 14,6 ± 2,2 |
| temporal | 15 | 171 | 1,0 ± 0,1 | 1,8 ± 0,1 | 8,3 ± 1,2 |
| golden | 13 | 73 | 0,3 ± 0,0 | 0,6 ± 0,1 | 0,5 ± 0,1 |
| pluginval | – | 0 (ferramenta externa) | 0,7 ± 0,1 | 1,1 ± 0,4 | 1,5 ± 0,4 |

Código compartilhado por todas as estratégias (`tests/common/`): 372 linhas. O golden master também guarda 13 arquivos de referência (7,5 MB).
