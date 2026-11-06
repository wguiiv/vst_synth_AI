# Resultados do experimento

Gerado por `experiment/analyze.py` a partir de `experiment/results/`. Não edite à mão: rode o script de novo.
Taxas com intervalo de confiança de 95% de Wilson. Uma abordagem detecta uma condição quando detecta em pelo menos uma execução.

## 1. Validade das execuções

| Tipo | Condições | Situação |
| --- | --- | --- |
| defeito | 34 | ok: 34 |
| inofensiva | 8 | ok: 8 |
| mutante | 225 | ok: 225 |
| referencia | 1 | ok: 1 |

Referência (código limpo no Linux): 5 execuções, 0 falhas nas tolerâncias padrão.

## 2. Detecção por abordagem e por origem (QP1, QP2, QP4)

"defeitos" reúne os dois lotes (11 + 23; PROTOCOLO, seção 10), que também aparecem separados.

Detectados (taxa) em cada conjunto. "Possivelmente equivalentes" são os mutantes que nenhuma abordagem detecta nas tolerâncias padrão (66 de 225; seção 7 do protocolo).

| Conjunto | Origem | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| defeitos | todos | 34 | 5 (14,7%) | 5 (14,7%) | 6 (17,6%) | 13 (38,2%) | 2 (5,9%) | 13 (38,2%) | 13 (38,2%) |
| defeitos | projeto | 22 | 3 (13,6%) | 5 (22,7%) | 5 (22,7%) | 10 (45,5%) | 2 (9,1%) | 10 (45,5%) | 10 (45,5%) |
| defeitos | juce | 12 | 2 (16,7%) | 0 (0,0%) | 1 (8,3%) | 3 (25,0%) | 0 (0,0%) | 3 (25,0%) | 3 (25,0%) |
| defeitos, lote 1 | todos | 11 | 2 (18,2%) | 2 (18,2%) | 3 (27,3%) | 3 (27,3%) | 1 (9,1%) | 3 (27,3%) | 3 (27,3%) |
| defeitos, lote 1 | projeto | 7 | 2 (28,6%) | 2 (28,6%) | 2 (28,6%) | 2 (28,6%) | 1 (14,3%) | 2 (28,6%) | 2 (28,6%) |
| defeitos, lote 1 | juce | 4 | 0 (0,0%) | 0 (0,0%) | 1 (25,0%) | 1 (25,0%) | 0 (0,0%) | 1 (25,0%) | 1 (25,0%) |
| defeitos, lote 2 | todos | 23 | 3 (13,0%) | 3 (13,0%) | 3 (13,0%) | 10 (43,5%) | 1 (4,3%) | 10 (43,5%) | 10 (43,5%) |
| defeitos, lote 2 | projeto | 15 | 1 (6,7%) | 3 (20,0%) | 3 (20,0%) | 8 (53,3%) | 1 (6,7%) | 8 (53,3%) | 8 (53,3%) |
| defeitos, lote 2 | juce | 8 | 2 (25,0%) | 0 (0,0%) | 0 (0,0%) | 2 (25,0%) | 0 (0,0%) | 2 (25,0%) | 2 (25,0%) |
| mutantes | todos | 225 | 100 (44,4%) | 85 (37,8%) | 108 (48,0%) | 153 (68,0%) | 68 (30,2%) | 154 (68,4%) | 159 (70,7%) |
| mutantes | projeto | 94 | 44 (46,8%) | 44 (46,8%) | 44 (46,8%) | 54 (57,4%) | 31 (33,0%) | 54 (57,4%) | 59 (62,8%) |
| mutantes | juce | 131 | 56 (42,7%) | 41 (31,3%) | 64 (48,9%) | 99 (75,6%) | 37 (28,2%) | 100 (76,3%) | 100 (76,3%) |
| mutantes sem os possivelmente equivalentes | todos | 159 | 100 (62,9%) | 85 (53,5%) | 108 (67,9%) | 153 (96,2%) | 68 (42,8%) | 154 (96,9%) | 159 (100,0%) |
| mutantes sem os possivelmente equivalentes | projeto | 59 | 44 (74,6%) | 44 (74,6%) | 44 (74,6%) | 54 (91,5%) | 31 (52,5%) | 54 (91,5%) | 59 (100,0%) |
| mutantes sem os possivelmente equivalentes | juce | 100 | 56 (56,0%) | 41 (41,0%) | 64 (64,0%) | 99 (99,0%) | 37 (37,0%) | 100 (100,0%) | 100 (100,0%) |

Com intervalo de confiança:

| Conjunto | Abordagem | Detectados | Taxa | IC 95% |
| --- | --- | --- | --- | --- |
| defeitos | sanidade | 5/34 | 14,7% | 6,4–30,1% |
| defeitos | espectral | 5/34 | 14,7% | 6,4–30,1% |
| defeitos | temporal | 6/34 | 17,6% | 8,3–33,5% |
| defeitos | golden | 13/34 | 38,2% | 23,9–55,0% |
| defeitos | pluginval | 2/34 | 5,9% | 1,6–19,1% |
| defeitos | qualquer teste | 13/34 | 38,2% | 23,9–55,0% |
| defeitos | qualquer abordagem | 13/34 | 38,2% | 23,9–55,0% |
| defeitos, lote 1 | sanidade | 2/11 | 18,2% | 5,1–47,7% |
| defeitos, lote 1 | espectral | 2/11 | 18,2% | 5,1–47,7% |
| defeitos, lote 1 | temporal | 3/11 | 27,3% | 9,7–56,6% |
| defeitos, lote 1 | golden | 3/11 | 27,3% | 9,7–56,6% |
| defeitos, lote 1 | pluginval | 1/11 | 9,1% | 1,6–37,7% |
| defeitos, lote 1 | qualquer teste | 3/11 | 27,3% | 9,7–56,6% |
| defeitos, lote 1 | qualquer abordagem | 3/11 | 27,3% | 9,7–56,6% |
| defeitos, lote 2 | sanidade | 3/23 | 13,0% | 4,5–32,1% |
| defeitos, lote 2 | espectral | 3/23 | 13,0% | 4,5–32,1% |
| defeitos, lote 2 | temporal | 3/23 | 13,0% | 4,5–32,1% |
| defeitos, lote 2 | golden | 10/23 | 43,5% | 25,6–63,2% |
| defeitos, lote 2 | pluginval | 1/23 | 4,3% | 0,8–21,0% |
| defeitos, lote 2 | qualquer teste | 10/23 | 43,5% | 25,6–63,2% |
| defeitos, lote 2 | qualquer abordagem | 10/23 | 43,5% | 25,6–63,2% |
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

| Defeito | Lote | Origem | Categoria | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F01 | 1 | projeto | altura | · | · | · | · | · | · | · |
| F02 | 1 | projeto | nivel | ● | ● | ● | ● | · | ● | ● |
| F03 | 1 | juce | envelope | · | · | ● | ● | · | ● | ● |
| F04 | 1 | juce | envelope | · | · | · | · | · | · | · |
| F05 | 1 | projeto | filtro | ● | ● | ● | ● | ● | ● | ● |
| F06 | 1 | projeto | numerico | · | · | · | · | · | · | · |
| F07 | 1 | projeto | estado | · | · | · | · | · | · | · |
| F08 | 1 | juce | nivel | · | · | · | · | · | · | · |
| F09 | 1 | juce | envelope | · | · | · | · | · | · | · |
| F10 | 1 | projeto | estado | · | · | · | · | · | · | · |
| F11 | 1 | projeto | numerico | · | · | · | · | · | · | · |
| F12 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F13 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F14 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F15 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F16 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F17 | 2 | juce | envelope | · | · | · | · | · | · | · |
| F18 | 2 | juce | filtro | ● | · | · | ● | · | ● | ● |
| F19 | 2 | juce | nivel | ● | · | · | ● | · | ● | ● |
| F20 | 2 | projeto | altura | · | · | · | ● | · | ● | ● |
| F21 | 2 | projeto | altura | · | ● | ● | ● | · | ● | ● |
| F23 | 2 | projeto | estado | ● | ● | ● | ● | ● | ● | ● |
| F24 | 2 | projeto | envelope | · | · | · | ● | · | ● | ● |
| F25 | 2 | projeto | envelope | · | · | · | · | · | · | · |
| F26 | 2 | projeto | envelope | · | · | · | ● | · | ● | ● |
| F28 | 2 | projeto | estado | · | · | · | · | · | · | · |
| F29 | 2 | projeto | filtro | · | ● | ● | ● | · | ● | ● |
| F30 | 2 | projeto | nivel | · | · | · | · | · | · | · |
| F31 | 2 | projeto | numerico | · | · | · | · | · | · | · |
| F32 | 2 | projeto | estado | · | · | · | ● | · | ● | ● |
| F33 | 2 | projeto | estado | · | · | · | · | · | · | · |
| F34 | 2 | projeto | estado | · | · | · | ● | · | ● | ● |
| F35 | 2 | projeto | estado | · | · | · | · | · | · | · |
| F36 | 2 | projeto | estado | · | · | · | · | · | · | · |

A matriz dos 225 mutantes está em `matriz_mutantes.csv`.

## 4. Detecção por categoria

Defeitos, por categoria:

| Categoria | n | sanidade | espectral | temporal | golden | pluginval | qualquer teste | qualquer abordagem |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| envelope | 12 | 0/12 | 0/12 | 1/12 | 3/12 | 0/12 | 3/12 | 3/12 |
| estado | 9 | 1/9 | 1/9 | 1/9 | 3/9 | 1/9 | 3/9 | 3/9 |
| nivel | 4 | 2/4 | 1/4 | 1/4 | 2/4 | 0/4 | 2/4 | 2/4 |
| altura | 3 | 0/3 | 1/3 | 1/3 | 2/3 | 0/3 | 2/3 | 2/3 |
| filtro | 3 | 2/3 | 2/3 | 2/3 | 3/3 | 1/3 | 3/3 | 3/3 |
| numerico | 3 | 0/3 | 0/3 | 0/3 | 0/3 | 0/3 | 0/3 | 0/3 |

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
| defeitos | golden | 5 | F20, F24, F26, F32, F34 |
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

**defeitos** (n = 34): Q de Cochran = 24,74, gl = 4, p = < 0,001.

| Par | Só o 1º detecta | Só o 2º detecta | p | p (Holm) |
| --- | --- | --- | --- | --- |
| sanidade × espectral | 2 | 2 | 1,000 | 1,000 |
| sanidade × temporal | 2 | 3 | 1,000 | 1,000 |
| sanidade × golden | 0 | 8 | 0,008 | 0,070 |
| sanidade × pluginval | 3 | 0 | 0,250 | 1,000 |
| espectral × temporal | 0 | 1 | 1,000 | 1,000 |
| espectral × golden | 0 | 8 | 0,008 | 0,070 |
| espectral × pluginval | 3 | 0 | 0,250 | 1,000 |
| temporal × golden | 0 | 7 | 0,016 | 0,109 |
| temporal × pluginval | 4 | 0 | 0,125 | 0,750 |
| golden × pluginval | 11 | 0 | < 0,001 | 0,010 |

Complementar (QP2, fora da correção de Holm): qualquer teste × pluginval: só os testes detectam 11, só o pluginval detecta 0, p = < 0,001.

**defeitos, lote 1** (n = 11): Q de Cochran = 5,60, gl = 4, p = 0,231.

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

**defeitos, lote 2** (n = 23): Q de Cochran = 21,82, gl = 4, p = < 0,001.

| Par | Só o 1º detecta | Só o 2º detecta | p | p (Holm) |
| --- | --- | --- | --- | --- |
| sanidade × espectral | 2 | 2 | 1,000 | 1,000 |
| sanidade × temporal | 2 | 2 | 1,000 | 1,000 |
| sanidade × golden | 0 | 7 | 0,016 | 0,141 |
| sanidade × pluginval | 2 | 0 | 0,500 | 1,000 |
| espectral × temporal | 0 | 0 | 1,000 | 1,000 |
| espectral × golden | 0 | 7 | 0,016 | 0,141 |
| espectral × pluginval | 2 | 0 | 0,500 | 1,000 |
| temporal × golden | 0 | 7 | 0,016 | 0,141 |
| temporal × pluginval | 2 | 0 | 0,500 | 1,000 |
| golden × pluginval | 9 | 0 | 0,004 | 0,039 |

Complementar (QP2, fora da correção de Holm): qualquer teste × pluginval: só os testes detectam 9, só o pluginval detecta 0, p = 0,004.

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

Cada tolerância percorre sua grade com as demais no padrão (seção 6 do protocolo). Os conjuntos têm 34 defeitos, 225 mutantes e 159 mutantes não equivalentes; alarmes falsos contam as 8 condições inofensivas. "Falhas na referência" conta as execuções do código limpo no Linux que falhariam naquele ponto. Gráfico: `curva_tolerancia.svg`.

**spectral.pitch** (espectral, unidade: cents, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 1 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 2 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 5 (padrão) | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 10 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 25 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 50 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.stopband** (espectral, unidade: dB, limite mínimo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 24 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 18 (padrão) | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 12 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 6 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.passband** (espectral, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.25 | 34 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.5 | 5 (14,7%) | 86 (38,2%) | 86 (54,1%) | 0 | 0 |
| 1 (padrão) | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 3 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**spectral.aliasing** (espectral, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 1 | 5 (14,7%) | 87 (38,7%) | 87 (54,7%) | 0 | 0 |
| 3 (padrão) | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 6 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |
| 10 | 5 (14,7%) | 85 (37,8%) | 85 (53,5%) | 0 | 0 |

**temporal.time** (temporal, unidade: s, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.002 | 34 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.005 | 34 (100,0%) | 224 (99,6%) | 158 (99,4%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.01 (padrão) | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.02 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.05 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**temporal.release** (temporal, unidade: s, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.005 | 34 (100,0%) | 225 (100,0%) | 159 (100,0%) | 8 (H01, H02, H03, H04, clang, debug, macos, windows) | 5 |
| 0.01 | 6 (17,6%) | 109 (48,4%) | 109 (68,6%) | 0 | 0 |
| 0.02 (padrão) | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.05 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.1 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**temporal.sustain** (temporal, unidade: dB, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0.1 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.25 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 0.5 (padrão) | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 1 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |
| 3 | 6 (17,6%) | 108 (48,0%) | 108 (67,9%) | 0 | 0 |

**golden.maxAbsDiff** (golden, unidade: linear, limite máximo):

| Valor | Defeitos | Mutantes | Mutantes não equivalentes | Alarmes falsos | Falhas na referência |
| --- | --- | --- | --- | --- | --- |
| 0 | 14 (41,2%) | 153 (68,0%) | 153 (96,2%) | 2 (macos, windows) | 0 |
| 1e-06 | 14 (41,2%) | 153 (68,0%) | 153 (96,2%) | 1 (macos) | 0 |
| 1e-05 | 14 (41,2%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.0001 (padrão) | 13 (38,2%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.001 | 13 (38,2%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |
| 0.01 | 13 (38,2%) | 153 (68,0%) | 153 (96,2%) | 0 | 0 |

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
