# Notas de análise e interpretação

Este arquivo reúne a interpretação dos resultados, para a escrita do artigo. Os números vêm de
`RESULTADOS.md` (gerado por `experiment/analyze.py`) e o método de `experiment/PROTOCOLO.md`; aqui ficam o
porquê de cada resultado, as ressalvas e o posicionamento do estudo. Cada afirmação indica de onde vem.
Quando um número daqui e de `RESULTADOS.md` divergirem, vale o de `RESULTADOS.md`.

Conferido em 2026-11-04 contra os dados brutos (`experiment/results/raw/`), o código-fonte e os patches; o
segundo lote de defeitos, em 2026-11-05.

## 0. Ângulo do artigo e conclusão central (ler primeiro)

Definidos com o autor em 2026-11-05 e 2026-11-06, depois do segundo lote de defeitos. A introdução, a
discussão e as considerações finais giram em torno desta seção; as seções seguintes dão os detalhes. O roteiro
do artigo, seção por seção, está em `artigo/ROTEIRO.md`.

### Fio condutor

**Testar software de áudio é difícil, e o estudo mostra concretamente por quê.** Para medir isso sem viés,
foram mineradas 34 correções reais de bugs em 12 projetos de áudio de código aberto (o JUCE e outros 11),
reintroduzidas no tapSynth por agentes que nunca viram os testes, além de 225 mutantes automáticos. Os testes
foram congelados antes de qualquer defeito existir. O resultado aponta duas dificuldades concretas, e não uma
terceira que se poderia esperar:

- **Situações:** um *plugin* roda em muitas situações (taxas de amostragem, tamanhos de bloco, mono ou
  estéreo, valores de borda dos parâmetros, parâmetros mudando durante a nota, vozes reaproveitadas, estado
  salvo, restrições de tempo real). Um defeito só aparece na situação certa. Dos 21 defeitos que nenhuma
  abordagem detectou, 16 não mudaram **nenhum** valor medido nas situações que os testes criaram (seção 3).
- **Oráculo** (saber qual é a saída certa): em 9 casos, o teste criou a situação e o som mudou, mas o teste de
  propriedade só verificava se o áudio era plausível (finito, sem *clipping*, audível) e aceitou. Quatro deles
  ninguém detectou (F04, F12, F14, F16); cinco só o *golden master* detectou, porque compara com uma gravação
  (F20, F24, F26, F32, F34).
- **Não foi o rigor:** a tolerância decidiu um único defeito em 34 (F17). As tolerâncias padrão deram zero
  alarmes falsos, e a curva de tolerância é plana até um "penhasco" (seção 6).

O fio condutor é **"situações e valores esperados, não rigor"**. Escrever "**quais** situações", não
"**mais** situações": os defeitos perdidos apontam situações específicas, e é delas que saem as
recomendações.

### Resposta à pergunta geral do plano

Pergunta: "como diferentes estratégias de teste automatizado de áudio se comparam, em detecção de defeitos e
em alarmes falsos, na verificação de um *plugin* VST3?" Resposta, no tapSynth:

1. **São complementares, cada uma com um ponto cego diferente.**
   - O pluginval verifica o *plugin* como programa, não o som. Detectou 2 dos 34 defeitos, os dois que quebram
     o *plugin* (F05, NaN na saída; F23, trava na primeira nota), e deixou passar seis de som claramente
     errado que os testes de propriedade detectaram (F02, F03, F18, F19, F21, F29). Nos mutantes, cada
     estratégia de propriedade, sozinha, detecta significativamente mais que ele após Holm (sanidade 37 contra
     5, espectral 24 contra 7, temporal 47 contra 7; RESULTADOS, seção 7).
   - Em contrapartida, o pluginval detectou 5 mutantes que nenhum teste percebeu (M0002, M0011, M0043, M0044,
     M0064), todos erros de memória (seção 4).
   - O *golden master* foi a rede mais ampla contra regressões: detectou os 13 defeitos detectados, 5 com
     exclusividade, e é o único par significativo contra o pluginval nos defeitos (11 contra 0, p = 0,010 após
     Holm). Vale só para regressões (seção 8, ressalva 1).
   - Testes do conteúdo sonoro (as quatro estratégias) contra pluginval: defeitos 11 contra 0, mutantes 91
     contra 5, ambos p < 0,001 (RESULTADOS, seção 7, comparação complementar).
2. **Combinar é necessário, mas não basta.** Todas juntas detectaram 13 de 34 defeitos (38,2%, IC 23,9–55,0%).
   Os 21 restantes formam a lista de pontos cegos do ferramental usual (seção 3).
3. **O que limita a detecção são situações e oráculo, não o rigor** (fio condutor acima). Alarmes falsos: zero
   para todas as abordagens nas tolerâncias padrão, em 8 condições inofensivas, incluindo Windows e macOS.
4. **Isso vira recomendação prática** (`experiment/GUIA_RECOMENDACAO.md`): usar as três abordagens na
   integração contínua; planejar explicitamente as situações (taxas, layouts de canais, bordas, automação
   durante a nota, estado, vozes reaproveitadas); dar valor esperado aos testes; e complementar com
   verificações de tempo real e de saídas que não são áudio.

### "Então os testes eram ruins?" (resposta honesta, para a discussão)

Em parte, a suíte tinha lacunas, e algumas eram evitáveis: todos os testes eram estéreo; o teste de FM só
rodava em 48 kHz; os 200 *patches* aleatórios só verificavam se o áudio era válido; o *helper* `render()`
limpava o *buffer*; nenhum teste salvava e restaurava estado nem reaproveitava uma voz ainda soando. O artigo
deve dizer isso abertamente.

Mas "malfeitos" não descreve o que os dados mostram:

- os testes são **rigorosos, não frouxos**: apertar as tolerâncias mudaria o resultado de 1 defeito em 34;
  faltou **amplitude**, não rigor;
- funcionam no que cobrem: zero alarmes falsos entre plataformas, compiladores e refatorações; 82% dos
  mutantes não equivalentes com as três estratégias de propriedade, 96% com o *golden*;
- parte dos defeitos está fora do alcance de qualquer teste de áudio (CPU, alocação, medidor, host);
- o autor dos testes nunca viu os defeitos. Toda suíte real é escrita antes dos bugs que depois deixa passar;
  34 de 34 com testes escritos conhecendo os bugs seria suspeito.

O estudo **não consegue separar** "testar áudio é difícil assim" de "esta suíte tinha estas lacunas"; outro
autor poderia escrever uma suíte melhor (seção 8, ressalva 2). Isso vai nas limitações.

Para a discussão, uma tabela que separa os 21 perdidos pelo que os resolveria (classificação
interpretativa, não regra do protocolo; a tabela da seção 3 agrupa os mesmos 21 pelo efeito medido, por isso
F07 e F15 aparecem em grupos diferentes):

| Tipo de lacuna | n | Defeitos | O que resolveria (inferência, não medido) |
| --- | --- | --- | --- |
| Situação previsível que a suíte não criou | 9 | F01 (FM em uma só taxa), F07 (`render()` limpa o *buffer*), F08, F28, F35 (mono), F09 (automação durante a nota), F13 (ataque e decay 0), F25 (voz reaproveitada), F36 (estado salvo e restaurado) | planejar as situações por recurso do *plugin* |
| Situação criada, mas sem valor esperado (oráculo) | 4 | F04, F12, F14, F16 | valores esperados ou relações entre saídas nos testes com *patches* aleatórios e extremos |
| Abaixo da tolerância | 1 | F17 | *golden* com 10⁻⁵ (seção 6) |
| Borda difícil de prever | 1 | F15 (só difere quando o envelope chega exatamente a 1) | — |
| Fora do alcance de teste de áudio | 6 | F06, F10 (tempo real), F33 (gesto de automação no host), F11, F30, F31 (medidor) | ferramentas de tempo real e CPU; testes de unidade do medidor; verificação do protocolo com o host |

Leitura: 14 dos 21 (as três primeiras linhas) são lacunas que um planejamento de situações e valores esperados
poderia cobrir; 7 exigiriam outro tipo de verificação ou são muito difíceis de prever. Isso é inferência: os
testes estão congelados e nenhuma suíte ampliada foi medida.

### Como ler o "13 de 34"

- Não é que "os defeitos não serviam para os testes". Os defeitos foram escolhidos por autores que nunca viram
  os testes, do jeito que bugs reais chegam; escolher defeitos que os testes pegam seria justamente o viés que
  o desenho evitou.
- O padrão de mercado (pluginval) detectou 2 de 34; com os testes do conteúdo sonoro, 13 de 34 (RESULTADOS,
  seção 2). Só os testes detectam 11, só o pluginval nenhum (p < 0,001; inclui o *golden*). Com o lote 1
  sozinho, a diferença não era significativa (2 contra 0, p = 0,500).
- Os dois lotes não devem ser comparados como "melhora": 3 de 11 (27,3%) e 10 de 23 (43,5%) têm intervalos que
  se sobrepõem quase inteiros (9,7–56,6% e 25,6–63,2%), e os lotes têm fontes e categorias diferentes.

### O *golden master* no texto

É uma rede de segurança barata para regressões (73 linhas, 0,3 s no Linux; RESULTADOS, seção 9). Foi o mais
amplo nos defeitos e nos mutantes, mas o desenho o favorece: todo defeito entrou depois da gravação das
referências. Entra na resposta à pergunta (conclusão 1) e nas recomendações, não como manchete. Não omitir: a
RESULTADOS mostra o número, e omiti-lo pareceria seletivo.

### Não afirmar

- que o teste automatizado é melhor que ouvir (não foi medido);
- que áudio é mais difícil de testar que outros tipos de software (não houve comparação; dizer que o estudo
  mostra **como** é difícil no tapSynth);
- que o estudo demonstrou a importância do teste automatizado em geral (isso vem da literatura, na introdução:
  Wang et al., 2022; Andrei, Verano Merino e Malavolta, 2025);
- que as situações que faltaram detectariam os defeitos perdidos (inferência; os testes estão congelados);
- que o segundo lote "melhorou" a detecção;
- que cada estratégia de propriedade supera o pluginval nos defeitos reais com significância (3 a 4 contra 0,
  não significativo; a significância vem dos mutantes, ou do conjunto com o *golden*);
- por que o pluginval não detectou o F36 (não investigado);
- generalização além do tapSynth.

### Rascunho das considerações finais

> Este trabalho comparou estratégias de teste automatizado de áudio no tapSynth, um sintetizador VST3 em JUCE,
> reintroduzindo no *plugin* 34 correções reais de *bugs*, mineradas de 12 projetos de áudio de código aberto,
> além de 225 mutantes. Os resultados mostram por que testar *software* de áudio é difícil: o que limitou a
> detecção não foi o rigor das comparações, mas as situações que os testes criam e a capacidade de saber qual
> é o som correto. Dos 21 defeitos que nenhuma abordagem detectou, 16 não alteraram nenhum valor medido nas
> situações testadas: só se manifestavam em processamento mono, em outras taxas de amostragem, em valores de
> borda do envelope, no reaproveitamento de vozes ou na restauração do estado, ou não afetavam o som, como os
> de segurança de tempo real. Em outros casos, o teste criou a situação e o som mudou, mas verificava apenas
> se o áudio era válido, não se estava correto. Apertar a tolerância teria mudado o resultado de um único
> defeito, e as tolerâncias adotadas não produziram nenhum alarme falso.
>
> As ferramentas mostraram-se complementares. O *pluginval* detectou erros de memória que nenhum teste de
> áudio percebeu, mas não detecta som errado; os testes de propriedade detectaram defeitos sonoros que o
> *pluginval* aceitou; e a comparação com gravações de referência funcionou como rede de segurança contra
> regressões, inclusive quando o som mudou de forma plausível. Mesmo combinadas, porém, deixaram passar a
> maioria dos defeitos reais. Para desenvolvedores de *plugins*, isso sugere combinar as três abordagens na
> integração contínua e, sobretudo, planejar explicitamente as situações de teste (taxas de amostragem,
> layouts de canais, valores de borda, automação durante a nota, estado salvo) e os valores esperados de cada
> uma, complementando-as com verificações específicas de tempo real. Por se tratar de um único *plugin*, esses
> resultados não se generalizam diretamente; trabalhos futuros podem medir se suítes construídas a partir
> dessas situações detectam os defeitos que esta não detectou.

## 1. O estudo em poucas linhas

- **Objeto:** tapSynth, sintetizador JUCE 8.0.9 com 5 vozes (2 osciladores com FM, ganho, ADSR de amplitude,
  filtro de variáveis de estado com envelope e LFO, reverb e medidor de nível).
- **Abordagens comparadas:** quatro estratégias de teste de áudio escritas para o estudo (GoogleTest, 80 testes
  no total) e o pluginval 1.0.4 (rigor 10, semente 42).

  | Estratégia | Testes | O que verifica |
  | --- | --- | --- |
  | sanidade | 24 | saída finita, sem clipping, audível, sem offset DC; também com todos os parâmetros no mínimo, no máximo e 200 patches aleatórios via JSON |
  | espectral | 21 | fundamental de cada nota (44,1/48/96 kHz), transposição dos osciladores, bandas laterais de FM, filtros passa-baixa/passa-alta/passa-banda, aliasing |
  | temporal | 15 | tempos de ataque, decay e release, nível de sustain, LFO no cutoff, independência entre vozes |
  | golden master | 13 | comparação amostra a amostra com 13 gravações de referência (`tests/references/`) |
  | (autoteste) | 7 | verifica as próprias medições; não é estratégia |

- **Condições:** 34 defeitos em dois lotes (F01–F11 e F12–F36, sem F22 e F27; seção 2), 225 mutantes do Mull
  (M0001–M0225), 8 condições inofensivas (H01–H04, build Debug, clang, Windows, macOS) e a referência (código
  limpo, 5 execuções).
- **Desenho contra viés:** testes congelados na tag `tests-frozen` antes de qualquer defeito existir; defeitos
  escritos por autores sem acesso aos testes; protocolo fixado antes da execução, com desvios registrados.

## 2. Origem dos defeitos (importante para o texto)

Os defeitos **não são inventados**: o guia (`experiment/GUIA_DEFEITOS.pdf`) exige que cada um reintroduza no
tapSynth um bug real, corrigido em software de áudio open source, com o link da correção aberto e conferido
pelo autor. No lote 1 as fontes eram plugins JUCE e o próprio JUCE; no lote 2, qualquer software de áudio
open source em C/C++. As fontes estão em `experiment/faults/INDEX.md`. "Origem" (projeto/JUCE) indica **onde
o patch é aplicado**, não de onde veio o bug.

**Lote 1 (F01–F11):**

| Defeito | Categoria | Origem | Fonte da correção real |
| --- | --- | --- | --- |
| F01 | altura | projeto | Odin 2 |
| F02 | nível | projeto | JUCE |
| F03 | envelope | JUCE | JUCE (ADSR) |
| F04 | envelope | JUCE | JUCE (ADSR) |
| F05 | filtro | projeto | Organic-Chemistry |
| F06 | numérico | projeto | Dexed |
| F07 | estado | projeto | JUCE |
| F08 | nível | JUCE | JUCE (Reverb) |
| F09 | envelope | JUCE | JUCE (Reverb) |
| F10 | estado | projeto | JUCE |
| F11 | numérico | projeto | LufsNormalizer |

**Lote 2 (F12–F36, sem F22 e F27):**

| Fonte da correção real | Defeitos |
| --- | --- |
| JUCE | F12, F13, F14, F19 |
| Odin 2 | F15, F32 |
| Helm | F16, F18, F26 |
| sfizz | F17, F34 |
| Tracktion Engine | F20, F21, F24, F31 |
| Monique | F23 |
| amsynth | F25, F33 |
| Dexed | F28 |
| ZynAddSubFX | F29, F30, F36 |
| Surge XT | F35 |

O lote 2 foi escrito por um segundo agente de IA, também sem acesso aos testes, pelo mesmo guia, com as regras
de inclusão e análise fixadas antes de o lote existir (PROTOCOLO, seção 10, emenda do segundo lote). Foram
pedidos 40 e entregues 25; F22 e F27 eram cópias exatas de F01 e F05 e foram excluídos antes da execução,
ficando 23. Todos aplicaram e compilaram (RESULTADOS, seção 1). Sete se afastam do mecanismo do bug original
(F14, F19, F23, F24, F26, F31, F36; `experiment/faults/RELATORIO-F12-F36.md`). Há concentração: 6 dos 8
defeitos com origem JUCE no lote 2 (F12–F17) estão no mesmo arquivo, `juce_ADSR.h`.

O autor do lote 1 foi um agente de IA separado, sem acesso aos testes nem ao plano, trabalhando num
repositório sem histórico e sem testes (desvio declarado no PROTOCOLO, seção 10; o colega previsto desistiu).
Para o texto: são **defeitos reais transplantados** para o tapSynth, **produzidos em dois lotes** pelo mesmo
procedimento: "O segundo lote foi acrescentado para ampliar a amostra, com as regras de inclusão e análise
fixadas antes de sua criação." Isso responde à crítica de "defeitos artificiais", mas não totalmente: a
adaptação ao tapSynth é feita à mão, e Andrews et al. (2005) observaram que defeitos semeados à mão podem ser
mais difíceis de detectar que os reais.

## 3. Defeitos: o que cada um faz e por que foi ou não detectado

Detectados nas tolerâncias padrão: **13 de 34** (lote 1: 3 de 11; lote 2: 10 de 23; RESULTADOS, seções 2 e 3).
Os valores citados nas explicações vêm das propriedades gravadas por cada teste (`tests.jsonl` em
`experiment/results/raw/`), comparadas com a referência.

**Lote 1:**

| Defeito | Detectado por | Explicação |
| --- | --- | --- |
| F02 reverb dry/wet trocados | sanidade, espectral, temporal, golden | com o padrão (dry 1, wet 0) a saída vira só reverb; todo teste de conteúdo percebe. O pluginval não, porque o áudio é válido, só está errado |
| F03 taxas de decay/release do ADSR | temporal, golden | muda os tempos do envelope; é exatamente o que o temporal mede |
| F05 cutoff modulado sem limite | as cinco | o cutoff passa do limite, o filtro gera NaN/inf; o pluginval acusa "NaNs found in buffer" |
| F01 FM sem taxa de amostragem | ninguém | sem a taxa, o oscilador fica na taxa padrão do JUCE (48 kHz); o teste de FM só roda em 48 kHz, onde as duas coincidem |
| F04 ataque zero no ADSR | ninguém | ataque 0 só aparece no teste com tudo no mínimo (onde a profundidade do filtro também é 0) e no patch aleatório 68, que muda cerca de 1% (pico 0,0346 → 0,0350), dentro da tolerância |
| F06 sem flush de denormais | ninguém | afeta só o uso de CPU, não o áudio |
| F07 buffer de saída sem limpar | ninguém | a função `render()` dos testes já limpa o buffer a cada bloco |
| F08 reverb mono com sinal seco | ninguém | os testes são sempre estéreo |
| F09 freeze do reverb | ninguém | o freeze só é ligado desde o início, quando segura silêncio dos dois jeitos |
| F10 alocação no callback de áudio | ninguém | má prática de tempo real, sem efeito no som; o pluginval 1.0.4 não tem verificação de alocação no Linux |
| F11 medidor com bloco vazio | ninguém | os testes nunca mandam bloco de 0 amostras, e o medidor não é áudio |

**Lote 2:**

| Defeito | Detectado por | Explicação |
| --- | --- | --- |
| F23 tipo de onda sem atualização inicial | as cinco | o oscilador nunca é inicializado e o plugin trava na primeira nota; o pluginval também falha |
| F18 reset incompleto do filtro | sanidade, golden | parte do estado do filtro sobra de uma nota para a outra; o pico passa do limite da sanidade |
| F19 ganho da voz não aplicado | sanidade, golden | sem o ganho fixo de 0,07, a saída fica muito mais alta e passa do limite de pico |
| F21 frequência sem `force` | espectral, temporal, golden | a suavização de 50 ms atua no início da nota e na FM; as bandas laterais da FM e a abertura do filtro saem do esperado |
| F29 filtro zerado a cada bloco | espectral, temporal, golden | o estado do filtro é apagado a cada bloco; a atenuação do passa-baixa, o aliasing e a abertura do filtro saem do limite |
| F20 oscilador sem limite em Nyquist | só golden | o golden falhou só na configuração com todos os parâmetros no máximo; nos patches aleatórios da sanidade o pico mudou até 56%, mas esse teste só verifica se o áudio é válido |
| F24 voz ignora `startSample` | só golden | a nota perde a posição dentro do bloco; na sanidade, o pico de uma configuração dobrou (0,283 → 0,566), ainda abaixo do limite; o golden falhou só no acorde |
| F26 envelope do filtro sem taxa de amostragem | só golden | o envelope do filtro fica na taxa padrão do JUCE ADSR (44,1 kHz); a abertura do filtro medida no temporal mudou pouco (41,7 → 41,1 dB, contra o mínimo de 18 dB) |
| F32 reverb sem `prepare` | só golden | o reverb fica preparado para 44,1 kHz; na sanidade, o pico da configuração com reverb mudou 14%, dentro do limite |
| F34 largura do reverb com padrão 0 | só golden | muda a imagem estéreo do reverb; na sanidade, o pico do canal direito com reverb caiu 30%, dentro do limite |
| F12 ataque vai sempre para o decay | ninguém | só muda o envelope do filtro com decay 0; mudou a componente DC de um único patch aleatório, num teste que só verifica se o áudio é válido |
| F13 sustain sem valor inicial | ninguém | exige ataque e decay iguais a 0 com sustain diferente de 0; nenhum teste cria essa combinação |
| F14 taxa de release calculada do sustain | ninguém | errado só quando a nota é solta antes de chegar ao sustain; mudou 677 valores nos patches aleatórios e no "tudo no máximo" (pico até 8%), mas esses testes só verificam se o áudio é válido; o teste temporal do release solta a nota já no sustain |
| F15 fim do ataque com `>` em vez de `>=` | ninguém | só difere quando o envelope chega exatamente a 1 |
| F16 ataque zero sem passar ao decay | ninguém | mesmo cenário do F04 (patch aleatório 68): o pico subiu de 0,035 para 0,058 (+68%), mas o teste só verifica se o áudio é válido |
| F17 fim do release sem zerar | ninguém | deixa um resíduo minúsculo depois do release; o golden mudou até 4,6 × 10⁻⁵, abaixo da tolerância de 10⁻⁴; com 10⁻⁵ seria detectado sem alarme falso (RESULTADOS, seção 8) |
| F25 reset da voz em todo note-on | ninguém | só muda quando uma voz é reaproveitada enquanto ainda soa; nenhum teste cria essa situação |
| F28 voz escreve no canal 2 em mono | ninguém | os testes são sempre estéreo (o mesmo ponto cego do F08) |
| F30 medidor lê só o canal esquerdo | ninguém | os testes não leem o medidor |
| F31 medidor divide por bloco vazio | ninguém | como o F11: nenhum teste manda bloco de 0 amostras |
| F33 gesto de automação sem fim | ninguém | afeta só a comunicação com o host (automação), não o som |
| F35 saída mono recusada | ninguém | os testes são estéreo, e o pluginval aceitou o plugin só com estéreo |
| F36 estado restaurado descartado | ninguém | os testes não salvam e restauram o estado. O pluginval 1.0.4 tem a verificação "Plugin state restoration", que rodou (nível 10) e passou; o motivo não foi investigado |

**Achado principal: por que 21 defeitos passaram.**

| Por que passou | n | Defeitos |
| --- | --- | --- |
| Nenhum valor medido mudou: situação que os testes não criam | 9 | F01, F08, F09, F13, F15, F25, F28, F35, F36 |
| Nenhum valor medido mudou: o defeito não afeta o som (tempo real, medidor, host) | 6 | F06, F10, F11, F30, F31, F33 |
| Nenhum valor medido mudou: escondido pelo `render()` | 1 | F07 |
| O som mudou, mas só em testes que verificam se o áudio é válido, sem valor esperado | 4 | F04, F12, F14, F16 |
| O som mudou abaixo da tolerância do golden | 1 | F17 |

16 dos 21 deixaram **todos os valores medidos idênticos bit a bit** à referência; para eles, nenhuma
tolerância, por mais rígida, bastaria. O limite principal continua sendo **quais situações os testes criam**
(taxas de amostragem, mono, blocos vazios, valores de borda, parâmetros que mudam durante a nota, vozes
reaproveitadas, estado salvo, CPU). O lote 2 mostra um segundo limite, o do **oráculo** (saber qual é a saída
certa): em 4 perdidos e nos 5 que só o golden pegou, o teste criou a situação e o som mudou, mas o teste de
propriedade só verificava se o áudio era plausível (finito, sem clipping, audível) ou se uma medida ficava
acima de um mínimo largo. A tolerância decidiu um único caso (F17).

**Envelope é a categoria mais perdida** (3 de 12; RESULTADOS, seção 4). Cinco dos seis defeitos do ADSR do
JUCE no lote 2 (F12–F16) envolvem tempos iguais a 0, o valor exato 1 ou soltar a nota antes do sustain; o
teste temporal mede ataque, decay e release com valores normais e solta a nota no sustain.

**Estatística:** com n = 34, o Q de Cochran é significativo (p < 0,001), mas, após Holm, só golden × pluginval
difere (11 contra 0, p = 0,010). Lote 1: nada significativo (Q, p = 0,231). Lote 2: Q, p < 0,001; após Holm,
só golden × pluginval (9 contra 0, p = 0,039) (RESULTADOS, seção 7). A taxa geral tem intervalo largo
(38,2%, IC 23,9–55,0%). Os defeitos servem sobretudo para a análise qualitativa; o peso estatístico das
comparações entre estratégias vem dos mutantes.

## 4. Mutantes

Números em RESULTADOS, seções 2, 4, 5 e 7.

- **225 mutantes**, 94 no projeto e 131 no JUCE (sobretudo `juce_Reverb.h`, 78, e `juce_ADSR.h`, 43; limitação
  do Mull com templates, PROTOCOLO 7.1).
- **51 mutantes quebram o plugin por completo** (travamento ou tempo esgotado) e são detectados por todas as
  estratégias. A comparação real está nos outros 174.
- **Golden master é quase um superconjunto das outras estratégias:** dos 154 mutantes que algum teste detecta,
  o golden só perde 1 (M0101: com release exatamente 0, o ADSR divide por zero; release 0 só aparece no teste
  de sanidade com tudo no mínimo, e o golden não tem esse cenário). O golden detecta sozinho 24 mutantes.
- **As três estratégias de propriedade juntas** (sanidade, espectral, temporal) detectam 130 dos 159 mutantes
  não equivalentes (82%); o golden, 153 (96%). Ou seja, quem escreve só testes de propriedade perde cerca de
  1 regressão em 6.
- **Só o pluginval detecta 5 mutantes** (M0002, M0011, M0043, M0044, M0064). Todos são laços que vão um passo
  além do fim de um vetor (`<` → `<=`, ou contagem invertida): erros de memória. Nos testes, a leitura fora do
  limite não derruba o processo; no pluginval, que varia taxas de amostragem (44,1/48/96 kHz) e tamanhos de
  bloco (64 a 1024) e carrega o VST3 compilado, derruba. Os testes detectam 91 mutantes que o pluginval não.

### Os 66 mutantes que ninguém detecta

O protocolo (seção 7) os classifica como "possivelmente equivalentes" e as taxas são dadas com e sem eles.
Nenhum é detectado em nenhum ponto da grade em que o código limpo passa, nem com o golden em tolerância 0
(comparação bit a bit): nos cenários dos testes, eles não mudam uma única amostra. A leitura do código de cada
um (interpretação, não regra do protocolo) dá quatro grupos:

| Grupo | n | Mutantes | Por que ninguém detecta |
| --- | --- | --- | --- |
| Código que só serve à interface | 23 | MeterData.cpp (17); `applyParameterSnapshot`, o "Desfazer" do editor (M0056–M0061) | os testes não leem o medidor nem usam o editor |
| Caminho que os cenários não exercitam | 14 | `processMono` do reverb do JUCE (M0183–M0195); M0042, laço que limpa canais extras | os testes são sempre estéreo; o helper `render()` limpa o buffer (o mesmo ponto cego do F07) |
| Efeito existe, mas ninguém verifica | 2 | M0054, M0055, verificações de nulo em `applyParametersFromJson` | o patch JSON (a entrada da função de IA) passa a ser ignorado; o único teste com JSON só verifica se o áudio é válido, não se os parâmetros mudaram. **Lacuna real dos testes** |
| Prováveis equivalentes ou de borda | 27 | comparações com zero exato no ADSR (14), `isFrozen` do reverb (M0196), voz 6 (M0037), `getVoice(5)` nulo (M0040, M0046, M0052), limite de cutoff `fs × 0,49` (M0076, M0094), canais em estéreo (M0090), OscData (M0027, M0028), Gain (M0219–M0221) | a mudança só altera o resultado em valores exatos que não ocorrem ou em situações que os testes não criam (por exemplo, 6 notas ao mesmo tempo; o limite de 20 kHz já vale antes de `fs × 0,49` nas taxas testadas) |

Os grupos 1 a 3 (39 mutantes) repetem o padrão dos defeitos: cobertura de situações, não rigor.

## 5. Alarmes falsos e plataformas

- **Zero alarmes falsos** em todas as abordagens, nas 8 condições inofensivas, nas tolerâncias padrão
  (RESULTADOS, seção 6). A referência passou nas 5 execuções no Linux, nas 5 no Windows e nas 5 no macOS.
- clang, Debug e as quatro refatorações produziram saída **idêntica bit a bit** à referência (diferença máxima
  do golden = 0).
- Windows e macOS diferem da referência Linux em no máximo 2,6 × 10⁻⁷ (Windows) e 1,5 × 10⁻⁶ (macOS) por
  amostra. Por isso o golden com tolerância 0 dá alarme nos dois, e com 10⁻⁶ só no macOS (RESULTADOS, seção 8);
  o padrão de 10⁻⁴ tem folga de quase 100 vezes.
- "Outro sistema" muda sistema operacional, compilador e processador ao mesmo tempo; não dá para atribuir a
  diferença a um deles.

## 6. Tolerâncias: o "penhasco" (QP3)

RESULTADOS, seção 8, e `curva_tolerancia.svg`.

- Em quase toda a grade, a detecção é **plana**: apertar ou afrouxar a tolerância não muda quem é detectado.
  Ganhos pequenos ao apertar: aliasing 1 dB (+2 mutantes), passband 0,5 dB (+1), release 10 ms (+1); todos já
  detectados pelo golden.
- Nos pontos mais rígidos, os testes **falham no código limpo** e "detectam" tudo, com 8 alarmes falsos:
  passband 0,25 dB, tempo do envelope 2 e 5 ms, release 5 ms.
- No golden, o limite fica nas outras plataformas: 0 e 10⁻⁶ falham no macOS (e 0 também no Windows), sem
  mudar a detecção dos mutantes.
- Nos defeitos, o único ponto que muda a detecção é o golden em 10⁻⁵ (e abaixo): detecta também o F17 (14 de
  34 em vez de 13), e 10⁻⁵ ainda não dá alarme falso. Ou seja, um ponto mais rígido que o padrão pegaria um
  defeito a mais sem custo nesta amostra. O padrão de 10⁻⁴ foi fixado antes de existir qualquer defeito;
  escolher 10⁻⁵ agora, depois de ver o F17, seria ajustar a tolerância aos dados. Na discussão, vale como
  observação: a folga de 10⁻⁴ sobre a maior diferença entre plataformas (1,5 × 10⁻⁶) era maior que o necessário.
- Conclusão: não há um compromisso gradual entre detecção e alarmes falsos; há uma zona larga e segura e um
  penhasco. As tolerâncias padrão ficaram na zona segura.

## 7. Respostas às perguntas de pesquisa

| Pergunta | Hipótese do plano | Resultado |
| --- | --- | --- |
| QP1: que estratégia detecta que categoria, com quantos alarmes falsos? | cada estratégia se especializa | Parcial. O temporal detecta muito mais mutantes do ADSR (27/43) que o espectral (4/43) e a sanidade (5/43). Mas o golden detecta 28/43 e quase tudo que os outros detectam; em detecção, o generalista vence os especialistas. Nos defeitos, envelope é a categoria mais perdida (3/12): o temporal só pega o F03, porque os defeitos do ADSR do lote 2 dependem de valores de borda que ele não cria. Zero alarmes falsos para todas |
| QP2: como as estratégias se comparam ao pluginval? | o pluginval pega estado, host e travamento, poucos defeitos de som | Confirmada em parte. Detecta 2 de 34 defeitos, os dois que quebram o plugin (F05, F23), e perde seis de som errado que os testes de propriedade pegam (F02, F03, F18, F19, F21, F29); seus 5 mutantes exclusivos são erros de memória. Não pegou os defeitos de host e estado do lote 2 (F33, F35, F36), incluindo o F36, apesar de ter verificação de restauração de estado. Complementar aos testes, não substituto (McNemar: mutantes 91 contra 5, defeitos 11 contra 0, ambos p < 0,001, ambos incluindo o golden) |
| QP3: como a tolerância afeta o compromisso? | o golden detecta mais, com mais alarmes falsos | Metade. Detecta mais (nos mutantes, significativo contra todas), mas sem alarmes falsos no padrão. A tolerância quase não importa até o penhasco; nos defeitos, decidiu um caso (F17, golden em 10⁻⁵; seção 6) |
| QP4: projeto vs JUCE? | sem diferença relevante | Mutantes, bruto: 57% (projeto) contra 76% (JUCE) nos testes. Sem os possivelmente equivalentes: 91,5% contra 100%. A diferença vem do código do projeto que não toca o áudio (medidor, editor), não de testes piores para o projeto. Defeitos: o inverso, 45,5% (10/22) no projeto contra 25,0% (3/12) no JUCE, com intervalos largos; 6 dos 12 defeitos do JUCE estão no mesmo arquivo (ADSR), quase todos com valores de borda e transições |

**Situação das hipóteses (para o texto):** QP1 confirmada só em parte; QP2 confirmada só em parte (com o lote
2, o pluginval não pegou os defeitos de estado e host que a hipótese previa); QP3 metade confirmada (detecta
mais, sem mais alarmes falsos); QP4 sem diferença atribuível a viés de autoria (nos mutantes, a diferença vem
de código que não toca o áudio; nos defeitos, intervalos largos e concentração no ADSR).

Estatística (RESULTADOS, seção 7): nos mutantes, Q de Cochran p < 0,001; todos os pares diferem após Holm,
exceto sanidade × temporal (20 contra 28, p = 0,312), que detectam quantidades parecidas de mutantes
diferentes. Nos defeitos (n = 34), Q de Cochran p < 0,001, mas após Holm só golden × pluginval difere
(p = 0,010); no lote 1 sozinho (n = 11), nada é significativo.

## 8. Ressalvas de interpretação (escrever na discussão)

1. **O desenho favorece o golden master.** As referências foram gravadas do código correto e todo defeito ou
   mutante entrou depois: todos são **regressões**, o ponto forte do golden. Um bug que já existe quando a
   referência é gravada fica protegido pelo golden; só os testes de propriedade (82% juntos) o pegariam. Logo,
   "o golden detecta 96%" vale para regressões. O mesmo vale para os defeitos: o golden detectou os 13
   detectados, 5 com exclusividade, porque todos foram inseridos depois da gravação.
2. **Os resultados são destes testes, não das estratégias em geral.** Um teste espectral escrito de outro jeito
   poderia detectar mais.
3. **Parte dos resultados é esperada** (comparação amostra a amostra detecta quase toda mudança; o pluginval
   não sabe como o sintetizador deve soar; teste não pega bug em código que não executa). As contribuições
   menos óbvias são: zero alarmes falsos entre plataformas e compiladores com 10⁻⁴; o penhasco; cobertura de
   situações acima de rigor; o papel do pluginval como verificação de memória; a medida de quanto os testes de
   propriedade perdem (1 em 6 regressões); e o limite do oráculo nos defeitos reais (testes que criam a
   situação, veem o som mudar e aceitam, porque só verificam se o áudio é plausível; seção 3).
4. **Duas hipóteses do plano falharam** (golden com mais alarmes falsos; especialização forte), o que mostra que
   o estudo podia dar errado.

## 9. Comparação de frameworks (análise secundária)

Detalhes em `experiment/frameworks/COMPARACAO.md`. O teste de altura foi portado para GoogleTest, Catch2 e
doctest; os três medem os mesmos 81 valores do teste original. Pontos para o texto:

- Só o GoogleTest grava valores medidos no relatório (`RecordProperty`); a varredura de tolerâncias do estudo
  dependia disso.
- O `Approx` do doctest tem margem que cresce com o valor; `Approx(0).epsilon(5)` nunca falha (demonstrado).
- Altura em cents é uma tolerância relativa por natureza (5 cents = 0,289% da frequência); tempos e
  diferenças de amostra são absolutas.
- O relatório do GoogleTest mostra cada combinação parametrizada como um caso de teste; Catch2 e doctest, um só.
- Custo de compilação e execução quase igual; o JUCE domina.

## 10. Posicionamento na literatura

- **Trabalho mais próximo:** VALENTAN, S.; WOTAWA, F. On the Automation of Audio Plugin Testing. QRS 2021,
  p. 270–278 (IEEE Xplore 9724773). Ferramenta de regressão para plugins que compara a saída renderizada de
  duas versões (na prática, um golden master) e compara geração combinatória e aleatória de entradas pelo escore
  de mutação. **Diferença deste estudo:** compara *tipos de verificação* (propriedade, golden, validador),
  mede alarmes falsos entre plataformas e compiladores, varre as tolerâncias e compara projeto e JUCE.
  Conferir autores e iniciais na página do IEEE antes de citar: o plano lista "VALENTAN, A.; WOTAWA, F.", e a
  busca indica Franz Wotawa e Stephan Valentan.
- **Automação de testes e qualidade (motivação):** WANG, Y.; MÄNTYLÄ, M. V.; LIU, Z.; MARKKULA, J. Test
  automation maturity improves product quality: quantitative study of open source projects using continuous
  integration. Journal of Systems and Software, v. 188, 2022. Conferido (existe; o resumo bate). São 37
  projetos Java: serve de evidência de outro domínio, não de áudio.
- **Lacuna de prática:** ANDREI, B.; VERANO MERINO, M.; MALAVOLTA, I. (o sobrenome é Andrei: o PDF traz
  "Bogdan Andrei", e-mail b.m.andrei@uva.nl; o plano escreveu "BOGDAN, A." por engano). The Ecosystem of Open-Source Music
  Production Software: A Mining Study on the Development Practices of VST Plugins on GitHub. MSR 2025,
  p. 513–525. Minerou 299 repositórios de plugins VST, incluindo práticas de teste; ler para os números exatos.
- **Mutantes como substitutos de defeitos reais:** ANDREWS et al., ICSE 2005; JUST et al., FSE 2014
  (acoplamento com 73% dos defeitos reais).
- **Mutantes equivalentes:** MADEYSKI et al., IEEE TSE 2014 (problema indecidível; relatar com e sem).
- **Oráculo e tolerância em software numérico:** KANEWALA; BIEMAN, revisão sistemática sobre teste de software
  científico (arXiv 1804.01954); BARR et al., The Oracle Problem in Software Testing, IEEE TSE 2015 (a conferir).
- **Ferramenta de mutação:** DENISOV; PANKEVICH, Mull it over, ICSTW 2018.
- **Uso de IA:** BALTES et al., Guidelines for Empirical Studies in Software Engineering involving Large
  Language Models (arXiv), já verificado no plano.
- **Método:** RUNESON; HÖST, guidelines para estudo de caso, Empirical Software Engineering 2009 (a conferir).
- Literatura cinza útil para a motivação: pluginval (repositório e anúncio no fórum do JUCE), bibliotecas de
  snapshot de áudio. Nenhum estudo empírico comparando estratégias de teste de áudio foi encontrado na busca
  feita; confirmar no Google Scholar, AES E-Library, DAFx, SBCM e SAST.

## 11. Limitações

- Um único plugin, pequeno, em JUCE 8.0.9; estudo de caso exploratório, não regra geral.
- 34 defeitos, 13 detectados, com intervalo largo (IC 23,9–55,0%); nos defeitos, só golden × pluginval é
  significativo após Holm.
- Os defeitos vieram em dois lotes. A decisão de fazer o segundo foi tomada depois de ver os resultados do
  primeiro; as regras foram fixadas antes de o lote existir, os testes continuaram congelados e o autor não viu
  os testes (PROTOCOLO, seção 10). O lote 2 entregou 25 dos 40 pedidos, e 6 de seus defeitos estão no mesmo
  arquivo (ADSR do JUCE).
- Defeitos reais transplantados por dois agentes de IA (não pelo colega previsto); adaptação manual ao
  tapSynth; no lote 2, 7 dos 23 se afastam do mecanismo do bug original.
- Detecção medida só no Linux; Windows e macOS só para alarmes falsos, e "outro sistema" muda SO, compilador
  e processador juntos.
- Mutantes do JUCE concentrados em Reverb e ADSR (limitação do Mull 0.36 com templates); mutantes rodaram num
  build clang com `-ffp-contract=off`, não no build gcc de referência.
- Viés de autoria: os testes foram escritos com assistência de IA, pelo mesmo lado que desenhou o estudo;
  mitigado pelo congelamento e pelo autor independente dos defeitos, não eliminado.
- O desenho só contém regressões (seção 8, item 1).
- Um framework no experimento; a comparação com Catch2 e doctest é qualitativa, com um teste.
- pluginval 1.0.4 sem verificação de tempo real (por isso F10 é indetectável por ele).

## 12. Uso de IA (fatos para declarar na metodologia)

- **Testes (80), executor do experimento, protocolo, script de análise, gráfico e comparação de frameworks:**
  escritos com o Claude Code (Anthropic), com revisão e aprovação do autor antes de cada commit (regra 6 do
  plano).
- **Defeitos e alterações inofensivas:** escritos por um agente de IA separado, sem acesso aos testes, seguindo
  `GUIA_DEFEITOS.pdf`; revisados pelo autor. O lote 2 de defeitos foi escrito por um segundo agente, nas mesmas
  condições, com um adendo ao guia (PROTOCOLO, seção 10); revisão só de formato.
- **Decisões de desenho, revisão e interpretação:** do autor, com apoio do Claude Code na análise.
- **Redação do artigo:** rascunhos gerados com o Claude (Anthropic), seção por seção, a partir deste
  repositório; o autor revisa, confere as afirmações e edita cada seção. Declarar isso também.
- Conferir as regras da universidade e seguir as diretrizes de Baltes et al. ao descrever o uso.

## 13. Pendências

- **Demonstração da apresentação:** a planejada (defeito de taxa de amostragem com o teste espectral falhando)
  não funciona, porque o F01 não foi detectado. Sugestão: F02 (dry/wet trocados), que se ouve na hora e mostra
  a diferença entre testes e pluginval (os quatro testes falham, o pluginval passa); ou F05 (NaN, todos falham).
- Guia de recomendação: feito, em `experiment/GUIA_RECOMENDACAO.md`.
- Escrita: resultados, discussão, limitações.
- Guia de recomendação atualizado com o lote 2 em 2026-11-06 (cenários novos, seção 3.1 sobre valores
  esperados, ressalva do pluginval sobre estado).
- LICENSE, nomes no README, conferir as referências "a conferir", merge para `master`.

## 14. Arquivos para a escrita

| Arquivo | Conteúdo |
| --- | --- |
| `artigo/ROTEIRO.md` | estrutura do artigo, orçamento de páginas, tabelas e figuras por seção |
| `artigo/universidade/REGRAS.md` | regras do modelo e das aulas da universidade (originais na mesma pasta e nos arquivos do projeto do Claude) |
| `experiment/analysis/RESULTADOS.md` | todas as tabelas (fonte dos números) |
| `experiment/analysis/NOTAS.md` | este arquivo |
| `experiment/analysis/curva_tolerancia.svg` | gráfico da QP3 |
| `experiment/PROTOCOLO.md` | método e desvios |
| `experiment/faults/INDEX.md` | defeitos e fontes |
| `experiment/frameworks/COMPARACAO.md` | comparação de frameworks |
| `experiment/GUIA_RECOMENDACAO.md` | guia prático para desenvolvedores (entrega do plano) |
| Plano do estudo (PDF) | perguntas, hipóteses, desenho |
