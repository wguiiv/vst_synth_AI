# Guia de recomendação: testes automatizados de áudio em plugins JUCE

Para quem desenvolve plugins de áudio (JUCE, VST3/AU) e quer saber **que testes escrever, com que tolerância,
e o que o pluginval cobre ou não**. É a entrega prática do estudo "testes automatizados de áudio no tapSynth".

**Como ler a evidência.** Cada recomendação traz uma marca:

- **[medido]**: resultado deste estudo (números em `experiment/analysis/RESULTADOS.md`, interpretação em
  `experiment/analysis/NOTAS.md`);
- **[inferido]**: conclusão tirada dos resultados, mas não testada diretamente.

O estudo é um caso: um sintetizador subtrativo pequeno em JUCE 8.0.9, avaliado com 34 defeitos reais
transplantados de 12 projetos de áudio de código aberto (em dois lotes) e 225 mutantes automáticos. Use o guia como ponto de partida, não como regra
geral; plugins de outro tipo (compressores, reverbs complexos) podem se comportar diferente.

## Resumo em oito regras

1. **Rode o pluginval na CI desde o primeiro dia.** É barato e pega o que os testes de áudio não pegam:
   travamentos, NaN e erros de memória. Mas ele não sabe como o plugin deve soar.
2. **Tenha um golden master com um cenário por módulo.** É o teste mais barato de escrever e o que mais pega
   regressões.
3. **Escreva testes de propriedade para o que o plugin promete** (altura, filtro, envelope, saída válida). Eles
   pegam bugs que já existiam quando o golden foi gravado e dizem *o que* quebrou.
4. **Invista em cenários, não em rigor.** A maior parte dos defeitos perdidos estava em situações que os testes
   nunca criaram. A lista da seção 3 diz quais.
5. **Dê valor esperado aos testes.** Um teste que só verifica se o áudio é válido aceita som errado: no estudo,
   o som mudou em vários defeitos e esses testes passaram (seção 3.1).
6. **Escolha tolerâncias em unidades naturais e longe do penhasco.** Apertar além de um ponto não pega mais
   nada e passa a falhar no código correto.
7. **Grave os valores medidos, não só passou/falhou.** Assim dá para rever tolerâncias sem rodar tudo de novo.
8. **Teste também o que não é áudio** (medidores, presets, entrada por JSON): verifique o efeito, não só que
   nada quebrou.

## 1. O que cada tipo de teste faz por você

| Tipo | Pergunta que responde | Pega bem | Não pega | Custo no estudo |
| --- | --- | --- | --- | --- |
| pluginval | "É um plugin bem comportado?" | travamento, NaN/inf, threads, erros de memória sob taxas e blocos variados | som errado mas válido; no estudo, também estado descartado (F36) e mono recusado (F35) | 0 linhas; 0,7 s |
| Sanidade | "É áudio válido?" | NaN, clipping, silêncio, offset DC; extremos de parâmetros | erros de altura, tempo ou timbre | 123 linhas; 6,5 s |
| Espectral | "As frequências estão certas?" | altura, transposição, FM, filtros, aliasing | erros de tempo | 176 linhas; 3,1 s |
| Temporal | "O comportamento no tempo está certo?" | envelope, LFO, independência entre vozes | erros que só mudam o timbre | 171 linhas; 1,0 s |
| Golden master | "O som é igual ao aprovado?" | quase qualquer regressão | bugs que já existiam na gravação; não explica a falha | 73 linhas + 7,5 MB de referências; 0,3 s |

Tempos: média de 5 execuções no Linux, código limpo (RESULTADOS, seção 9). **[medido]**

Detecção nos 159 mutantes detectáveis (RESULTADOS, seção 2): golden 96%, temporal 68%, sanidade 63%,
espectral 54%, pluginval 43%; as três estratégias de propriedade juntas, 82%; tudo junto, 100%. **[medido]**

Detecção nos 34 defeitos reais: golden 13, temporal 6, sanidade 5, espectral 5, pluginval 2; as três
estratégias de propriedade juntas, 8; tudo junto, 13 (38%). Os outros 21 estão na seção 3. **[medido]**

## 2. Que testes escrever, em ordem

### 2.1 pluginval na CI

- Rode no binário compilado (VST3/AU), em todo push, com rigor alto e semente fixa, para que toda execução faça
  o mesmo. O estudo usou `--strictness-level 10 --random-seed 42 --skip-gui-tests`
  (`experiment/run_pluginval.sh`); levou menos de 2 s neste plugin. **[medido]**
- Ele detectou 30% dos mutantes, e 5 que nenhum teste de áudio pegou: laços que liam ou escreviam um item além
  do fim de um vetor. Ele carrega o plugin como um host e varia taxas (44,1/48/96 kHz) e blocos (64 a 1024),
  condições em que esses erros derrubam o processo. **[medido]**
- Dos 34 defeitos reais, detectou só os 2 que quebravam o plugin (F05, NaN; F23, trava na primeira nota). Deixou
  passar seis de som errado que os testes pegaram, como o F02 (reverb com dry e wet trocados): o áudio era
  válido, só estava errado. **[medido]**
- Tem uma verificação de restauração de estado, e mesmo assim não detectou o F36, que descarta o estado
  restaurado; o motivo não foi investigado. Não conte com ele para estado. **[medido]**
- Detalhes do que ele cobre e não cobre na seção 5.

### 2.2 Golden master

- Grave a saída do plugin aprovado em alguns cenários e compare amostra a amostra. No estudo, 13 cenários
  (padrão, acorde, cada forma de onda, FM, cada tipo de filtro, LFO, envelope, reverb, parâmetros no mínimo e
  no máximo) pegaram 96% das regressões detectáveis com 73 linhas de código. Nos defeitos reais, pegaram os 13
  detectados, 5 deles sozinhos: o som mudou de forma plausível e os testes de propriedade aceitaram. **[medido]**
- Regra prática: **um cenário por módulo do sinal**, mais os extremos. **[inferido]**
- Grave as referências numa plataforma de referência e versione os arquivos com o código. Use tolerância
  absoluta por amostra; 10⁻⁴ deu zero alarmes falsos em Windows, macOS, clang e Debug, e 10⁻⁵ também, pegando
  um defeito a mais (F17; seção 4.3). **[medido]**
- Quando o som mudar **de propósito**, ouça antes de regravar. Regravar sem ouvir transforma um bug em
  referência. **[inferido]**
- Limite: o golden protege o som *aprovado*. Um bug presente no dia da gravação passa a ser protegido. É por
  isso que ele não basta sozinho (2.3). **[inferido, a partir do desenho do estudo]**

### 2.3 Testes de propriedade

Escreva um teste para cada promessa do plugin, com valores esperados vindos da especificação, não da saída
atual:

- **Sanidade**: saída finita, pico abaixo do limite, audível, sem DC; com parâmetros no padrão, no mínimo, no
  máximo e em patches aleatórios (o estudo usou 200 com semente fixa). É a mais simples de escrever.
- **Espectral**: a fundamental de cada nota em cents, em todas as taxas suportadas; atenuação dos filtros
  acima/abaixo do corte em dB; aliasing comparado ao nível caracterizado.
- **Temporal**: tempos de ataque, decay e release; nível de sustain; velocidade do LFO; uma voz não afeta outra.

Por que vale o esforço, se o golden pega mais: **[medido]** juntos, eles pegaram 82% das regressões e 8 dos 34
defeitos reais (seis deles de som errado que o pluginval aceitou), e são a única defesa contra bugs que já
existiam quando o golden foi gravado. Quando falham, dizem o que quebrou ("a
altura está 30 cents acima em 96 kHz"); o golden só diz que algo mudou. **[inferido]**

Sanidade e temporal pegaram quantidades parecidas de mutantes, mas mutantes diferentes (McNemar p = 0,31): os
dois se complementam. **[medido]**

## 3. Cenários: onde os testes do estudo falharam

Nos 21 defeitos que nada pegou, 16 não mudaram **nenhum** valor medido, nem uma amostra. Nenhuma tolerância os
pegaria: faltou o cenário. **[medido]** Cada item abaixo vem de um defeito ou mutante perdido:

| Cobrir | Por quê (o que escapou no estudo) |
| --- | --- |
| **Toda taxa de amostragem suportada, em toda função** | F01: o FM ficava na taxa padrão do JUCE (48 kHz) e o teste de FM só rodava em 48 kHz |
| **Mono e outros layouts de canais** | F08, F28, F35 e 13 mutantes do reverb mono: os testes eram sempre estéreo |
| **Buffer de saída com lixo** (não limpe o buffer na infraestrutura de teste) | F07 e M0042: o próprio helper de teste limpava o buffer, escondendo o bug |
| **Blocos de 0 amostras e tamanhos de bloco variados** | F11 e F31: o medidor falhava com bloco vazio, que nenhum teste mandava |
| **Parâmetros mudando durante a nota** (automação) | F09: o freeze do reverb só era testado ligado desde o início |
| **Valores exatos de borda** (0, mínimo, máximo) em cada parâmetro, sozinhos e combinados | F04, F12, F13, F16 e M0101: tempos de envelope iguais a 0 só apareciam misturados com outros extremos ou em patches aleatórios |
| **Transições do envelope** (soltar a nota no ataque e no decay, não só no sustain) | F14: o release saía errado quando a nota era solta antes do sustain; o teste de release soltava sempre no sustain |
| **Vozes reaproveitadas** (mais notas que vozes, nota repetida durante o release) | F25: a voz era zerada mesmo quando ainda soava |
| **Salvar e restaurar o estado**, verificando parâmetros e som depois | F36: o estado restaurado era descartado; nenhum teste fazia a volta completa, e o pluginval passou |
| **Comunicação com o host** (gestos de automação) | F33: o gesto de automação ficava aberto; nem testes nem pluginval verificam |
| **Saídas que não são áudio** (medidores, estado exibido) | F11, F30, F31 e 17 mutantes do medidor |
| **Entradas que não são áudio** (presets, JSON, estado salvo): verifique que os parâmetros mudaram | M0054 e M0055: o patch JSON passava a ser ignorado, e o único teste com JSON só verificava se o áudio era válido |
| **Segurança de tempo real** (alocação no callback, denormais e CPU) | F06 e F10: não mudam o som; precisam de ferramenta própria (medição de CPU, detector de alocação) |

Regra geral: **[inferido]** para cada recurso do plugin, pergunte "em que taxa, layout de canais, tamanho de
bloco e valor de parâmetro isso ainda não foi exercitado?" antes de perguntar "a tolerância está apertada o
bastante?".

### 3.1 Valores esperados (o problema do oráculo)

Em 9 defeitos, o teste criou a situação e o som mudou, mas o teste de propriedade só verificava se o áudio era
plausível (finito, sem clipping, audível) e aceitou. Quatro ninguém pegou (F04, F12, F14, F16: por exemplo, no
F16 o pico de um patch aleatório subiu 68%); cinco só o golden pegou (F20, F24, F26, F32, F34). **[medido]**

O que fazer: **[inferido]**

- nos testes com patches aleatórios e extremos, além de "é áudio válido", compare com uma gravação (um golden
  dos patches aleatórios, com semente fixa) ou verifique relações que devem valer para qualquer patch (por
  exemplo, o release dura o tempo configurado, de qualquer ponto do envelope);
- prefira verificações com valor esperado ("o pico é 0,28 ± 1 dB") a limites largos ("o pico é menor que 1")
  sempre que a especificação permitir.

## 4. Com que tolerância

### 4.1 Unidades naturais

Meça cada grandeza na unidade em que o erro faz sentido: **[inferido]**

| Grandeza | Unidade | Por quê |
| --- | --- | --- |
| Altura | cents | é uma razão: 5 cents valem o mesmo em qualquer nota (0,289% da frequência) |
| Níveis, filtros, aliasing | dB | escala da percepção e das especificações de filtro |
| Tempos de envelope | ms | o erro tolerável não depende do tempo configurado |
| Golden master | amplitude linear por amostra | compara forma de onda, não percepção |

### 4.2 Valores que funcionaram

As tolerâncias padrão do estudo deram **zero alarmes falsos** em Windows, macOS, clang, Debug e quatro
refatorações, e são um ponto de partida razoável para um sintetizador. **[medido]**

| Verificação | Tolerância |
| --- | --- |
| Altura | ≤ 5 cents |
| Atenuação na banda de corte | ≥ 18 dB |
| Variação na banda de passagem | ≤ 1 dB |
| Aliasing | ≤ 3 dB acima do nível caracterizado no código correto |
| Tempos de ataque e decay | ≤ 10 ms |
| Tempo de release | ≤ 20 ms |
| Nível de sustain | ≤ 0,5 dB |
| Golden master | ≤ 10⁻⁴ por amostra |
| Sanidade | pico ≤ 1,0 (≤ 4,0 em patches aleatórios); audível ≥ 10⁻³; DC ≤ 0,01 (≤ 0,05 aleatório) |

### 4.3 O penhasco

Em quase toda a faixa testada, mudar a tolerância **não mudou** o que era detectado. A exceção foi o golden em
10⁻⁵: pegou um defeito a mais (F17, um resíduo minúsculo depois do release), ainda sem alarme falso. Nos valores mais rígidos,
os testes passaram a falhar no código correto (8 alarmes falsos em 8 condições): banda de passagem 0,25 dB,
tempos de envelope de 2 e 5 ms, release de 5 ms. No golden, 10⁻⁶ e 0 falharam em outras plataformas
(o macOS difere da referência Linux em até 1,5 × 10⁻⁶ por amostra). **[medido]**
Gráfico: `experiment/analysis/curva_tolerancia.svg`.

Consequência: não gaste tempo afinando tolerâncias em busca de detecção; escolha um valor com folga e invista
em cenários (seção 3). **[inferido]**

### 4.4 Como escolher uma tolerância nova

1. Meça a variação do próprio código correto: em todas as plataformas, compiladores e taxas suportadas.
2. Ponha a tolerância acima dessa variação, com folga. No estudo, o golden ficou com folga de quase 100 vezes
   (10⁻⁴ contra 1,5 × 10⁻⁶); nos mutantes, isso não custou nenhuma detecção, mas nos defeitos custou uma
   (F17), que uma folga de cerca de 7 vezes (10⁻⁵) pegaria sem alarme falso. Uma folga de 5 a 10 vezes parece
   suficiente. **[medido; a faixa de folga é inferida]**
3. Confira que ela ainda pega um defeito conhecido (por exemplo, desafinar uma nota em 20 cents). **[inferido]**
4. Grave o valor medido no relatório, para rever a decisão depois (seção 6). **[medido: foi o que permitiu a
   varredura do estudo]**

## 5. O que o pluginval cobre e o que não cobre

| Cobre | Não cobre |
| --- | --- |
| Abrir, fechar e reabrir o plugin | Se o som está certo (F02 passou) |
| Processar áudio em várias taxas e tamanhos de bloco | Altura, timbre e tempos de envelope corretos |
| NaN e infinito na saída (pegou o F05) | Medidores e outras saídas que não são áudio |
| Salvar e restaurar o estado (tem a verificação, mas o F36, que descarta o estado restaurado, passou) | CPU e denormais (F06) |
| Parâmetros: faixa, automação, mudanças de várias threads, fuzzing | Alocação no callback de áudio (F10); a versão 1.0.4 não verifica no Linux |
| Erros de memória que essas condições expõem (5 mutantes exclusivos) | Bugs que precisam de um cenário musical específico (notas, acordes, sequência de parâmetros) |
| | Layout mono: aceitou o plugin que recusa mono (F35) e não pegou a escrita no canal 2 em mono (F28) |
| | Gestos de automação sem fim (F33) |

Resumo: **[medido]** o pluginval verifica que o plugin é um **plugin bem comportado**; os testes de áudio
verificam que ele é **o plugin certo**. Nos mutantes, os testes pegaram 91 que o pluginval não pegou, e o
pluginval pegou 5 que os testes não pegaram (p < 0,001). Nos defeitos reais, 11 contra 0 (p < 0,001). Use os
dois, e não conte com nenhum dos dois para estado, layouts de canais e host sem testes próprios.

## 6. Infraestrutura de teste

- **Renderize pelo caminho real.** Crie o processador de verdade (sem editor), mande MIDI e chame
  `processBlock` como um host faria; ver `tests/common/Render.cpp`. **[medido: todos os testes do estudo
  funcionam assim]**
- **Não faça pelo plugin o que é responsabilidade dele.** O helper do estudo limpava o buffer de saída antes de
  cada bloco e escondeu um defeito real (F07). **[medido]**
- **Um processo por teste.** Assim um travamento afeta só aquele teste. **[medido: o executor do estudo]**
- **Grave o valor medido de cada verificação** (no GoogleTest, `RecordProperty`). Permite rever tolerâncias e
  entender falhas sem rodar de novo. **[medido]**
- **Inicialize o JUCE no `main` dos testes** (`juce::ScopedJuceInitialiser_GUI`): os parâmetros do processador
  precisam do gerenciador de mensagens. **[medido]**

## 7. Escolha do framework

Os três servem; as diferenças que importam para áudio (`experiment/frameworks/COMPARACAO.md`): **[medido]**

| Framework | Ponto forte | Cuidado |
| --- | --- | --- |
| GoogleTest | único que grava valores medidos no relatório; cada combinação parametrizada vira um caso na CI | sintaxe de parametrização mais longa |
| Catch2 | comparações absoluta (`WithinAbs`), relativa (`WithinRel`) e em ULPs explícitas | o relatório JUnit não guarda valores medidos |
| doctest | um único cabeçalho, o mais simples de incluir | `Approx` mistura margem relativa e absoluta: `Approx(0).epsilon(5)` nunca falha; use comparação explícita |

## 8. Receita para um plugin novo

1. **Dia 1:** pluginval na CI, rigor 10, semente fixa.
2. **Sanidade** no padrão, no mínimo, no máximo e em patches aleatórios com semente fixa.
3. **Golden master:** um cenário por módulo do sinal, mais os extremos; tolerância 10⁻⁴; referências
   versionadas.
4. **Propriedades** para cada promessa central (altura, filtros, envelopes), em todas as taxas suportadas.
5. **Cenários** da seção 3: mono, blocos de 0 e variados, automação durante a nota, bordas exatas, transições
   do envelope, vozes reaproveitadas, estado salvo e restaurado, saídas e entradas que não são áudio.
6. **Valores esperados** também nos testes com patches aleatórios e extremos (seção 3.1).
7. **Tolerâncias** em unidades naturais, com folga sobre a variação entre plataformas (seção 4).
8. **Ao mudar o som de propósito:** ouvir, regravar as referências e commitar junto com a mudança.

## 9. Limites deste guia

- Baseado num único sintetizador pequeno; efeitos, reverbs e sintetizadores complexos podem exigir outros
  cenários e tolerâncias.
- Os defeitos e mutantes do estudo foram todos **regressões** (introduzidos depois da gravação do golden), o
  que favorece o golden master.
- A detecção foi medida só no Linux.
- 34 defeitos, 13 detectados: as taxas nos defeitos têm intervalos largos; o peso estatístico das
  comparações vem dos mutantes.
- Recomendações marcadas **[inferido]** são conclusões razoáveis, mas não foram testadas diretamente.
