# Notas de análise e interpretação

Este arquivo reúne a interpretação dos resultados, para a escrita do artigo. Os números vêm de
`RESULTADOS.md` (gerado por `experiment/analyze.py`) e o método de `experiment/PROTOCOLO.md`; aqui ficam o
porquê de cada resultado, as ressalvas e o posicionamento do estudo. Cada afirmação indica de onde vem.
Quando um número daqui e de `RESULTADOS.md` divergirem, vale o de `RESULTADOS.md`.

Conferido em 2026-11-04 contra os dados brutos (`experiment/results/raw/`), o código-fonte e os patches.

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

- **Condições:** 11 defeitos (F01–F11), 225 mutantes do Mull (M0001–M0225), 8 condições inofensivas (H01–H04,
  build Debug, clang, Windows, macOS) e a referência (código limpo, 5 execuções).
- **Desenho contra viés:** testes congelados na tag `tests-frozen` antes de qualquer defeito existir; defeitos
  escritos por um autor sem acesso aos testes; protocolo fixado antes da execução, com desvios registrados.

## 2. Origem dos defeitos (importante para o texto)

Os defeitos **não são inventados**: o guia (`experiment/GUIA_DEFEITOS.pdf`) exige que cada um reintroduza no
tapSynth um bug real, corrigido em um plugin JUCE open source ou no próprio JUCE, com o link da correção
aberto e conferido pelo autor. As fontes estão em `experiment/faults/INDEX.md`:

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

O autor foi um agente de IA separado, sem acesso aos testes nem ao plano, trabalhando num repositório sem
histórico e sem testes (desvio declarado no PROTOCOLO, seção 10; o colega previsto desistiu). Para o texto:
são **defeitos reais transplantados** para o tapSynth. Isso responde à crítica de "defeitos artificiais", mas
não totalmente: a adaptação ao tapSynth é feita à mão, e Andrews et al. (2005) observaram que defeitos
semeados à mão podem ser mais difíceis de detectar que os reais.

## 3. Defeitos: o que cada um faz e por que foi ou não detectado

Detectados nas tolerâncias padrão: **3 de 11** (RESULTADOS, seções 2 e 3).

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

**Achado principal:** 7 dos 8 defeitos perdidos deixaram **todos os valores medidos idênticos bit a bit** à
referência (conferido comparando as propriedades gravadas de cada teste); o oitavo (F04) mudou um único patch
em cerca de 1%. Nenhuma tolerância, por mais rígida, os pegaria. O limite dos testes é **quais situações eles
criam** (taxas de amostragem, mono, blocos vazios, parâmetros que mudam durante a nota, CPU), não o rigor da
comparação.

Com n = 11, as taxas por defeito têm intervalos largos (27%: 9,7–56,6%) e o Q de Cochran não é significativo
(p = 0,231). Os defeitos servem para a análise qualitativa; o peso estatístico vem dos mutantes.

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
  mudar a detecção.
- Conclusão: não há um compromisso gradual entre detecção e alarmes falsos; há uma zona larga e segura e um
  penhasco. As tolerâncias padrão ficaram na zona segura.

## 7. Respostas às perguntas de pesquisa

| Pergunta | Hipótese do plano | Resultado |
| --- | --- | --- |
| QP1: que estratégia detecta que categoria, com quantos alarmes falsos? | cada estratégia se especializa | Parcial. O temporal detecta muito mais mutantes do ADSR (27/43) que o espectral (4/43) e a sanidade (5/43). Mas o golden detecta 28/43 e quase tudo que os outros detectam; em detecção, o generalista vence os especialistas. Zero alarmes falsos para todas |
| QP2: como as estratégias se comparam ao pluginval? | o pluginval pega estado, host e travamento, poucos defeitos de som | Confirmada. Perde o F02 (som errado, áudio válido); seus 5 exclusivos são erros de memória. Complementar aos testes, não substituto (McNemar: 91 contra 5, p < 0,001) |
| QP3: como a tolerância afeta o compromisso? | o golden detecta mais, com mais alarmes falsos | Metade. Detecta mais (significativo contra todas), mas sem alarmes falsos no padrão. A tolerância quase não importa até o penhasco (seção 6) |
| QP4: projeto vs JUCE? | sem diferença relevante | Bruto: 57% (projeto) contra 76% (JUCE) nos testes. Sem os possivelmente equivalentes: 91,5% contra 100%. A diferença vem do código do projeto que não toca o áudio (medidor, editor), não de testes piores para o projeto |

Estatística (RESULTADOS, seção 7): nos mutantes, Q de Cochran p < 0,001; todos os pares diferem após Holm,
exceto sanidade × temporal (20 contra 28, p = 0,312), que detectam quantidades parecidas de mutantes
diferentes. Nos defeitos, nada é significativo (n = 11).

## 8. Ressalvas de interpretação (escrever na discussão)

1. **O desenho favorece o golden master.** As referências foram gravadas do código correto e todo defeito ou
   mutante entrou depois: todos são **regressões**, o ponto forte do golden. Um bug que já existe quando a
   referência é gravada fica protegido pelo golden; só os testes de propriedade (82% juntos) o pegariam. Logo,
   "o golden detecta 96%" vale para regressões.
2. **Os resultados são destes testes, não das estratégias em geral.** Um teste espectral escrito de outro jeito
   poderia detectar mais.
3. **Parte dos resultados é esperada** (comparação amostra a amostra detecta quase toda mudança; o pluginval
   não sabe como o sintetizador deve soar; teste não pega bug em código que não executa). As contribuições
   menos óbvias são: zero alarmes falsos entre plataformas e compiladores com 10⁻⁴; o penhasco; cobertura de
   situações acima de rigor; o papel do pluginval como verificação de memória; a medida de quanto os testes de
   propriedade perdem (1 em 6 regressões).
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
- **Lacuna de prática:** BOGDAN, A.; VERANO MERINO, M.; MALAVOLTA, I. The Ecosystem of Open-Source Music
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
- 11 defeitos, 3 detectados: sem poder estatístico nos defeitos.
- Defeitos reais transplantados por um agente de IA (não pelo colega previsto); adaptação manual ao tapSynth.
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
  `GUIA_DEFEITOS.pdf`; revisados pelo autor.
- **Decisões de desenho, revisão, interpretação e escrita:** do autor.
- Conferir as regras da universidade e seguir as diretrizes de Baltes et al. ao descrever o uso.

## 13. Pendências

- **Demonstração da apresentação:** a planejada (defeito de taxa de amostragem com o teste espectral falhando)
  não funciona, porque o F01 não foi detectado. Sugestão: F02 (dry/wet trocados), que se ouve na hora e mostra
  a diferença entre testes e pluginval (os quatro testes falham, o pluginval passa); ou F05 (NaN, todos falham).
- Guia de recomendação: feito, em `experiment/GUIA_RECOMENDACAO.md`.
- Escrita: resultados, discussão, limitações.
- LICENSE, nomes no README, conferir as referências "a conferir", merge para `master`.

## 14. Arquivos para a escrita

| Arquivo | Conteúdo |
| --- | --- |
| `experiment/analysis/RESULTADOS.md` | todas as tabelas (fonte dos números) |
| `experiment/analysis/NOTAS.md` | este arquivo |
| `experiment/analysis/curva_tolerancia.svg` | gráfico da QP3 |
| `experiment/PROTOCOLO.md` | método e desvios |
| `experiment/faults/INDEX.md` | defeitos e fontes |
| `experiment/frameworks/COMPARACAO.md` | comparação de frameworks |
| `experiment/GUIA_RECOMENDACAO.md` | guia prático para desenvolvedores (entrega do plano) |
| Plano do estudo (PDF) | perguntas, hipóteses, desenho |
