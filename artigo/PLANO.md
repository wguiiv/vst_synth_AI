# Plano do estudo (transcrição)

Transcrição em texto do PDF `Plano do estudo testes automatizados de áudio no tapSynth.pdf` (raiz do
repositório, 11 páginas), feita para quem não consegue abrir o PDF. O texto do plano está reproduzido sem
alterações de conteúdo; só a diagramação mudou (tabelas refeitas em Markdown, nomes de código em `crase`).
As notas marcadas **[Nota de execução]** não fazem parte do plano: dizem onde a execução se afastou dele e
apontam para o registro oficial (`experiment/PROTOCOLO.md`, seção 10, e `experiment/analysis/NOTAS.md`).

O plano foi escrito antes da fase 0 e não foi alterado depois. Ele é a fonte da pergunta-problema, das QP1 a
QP4, das hipóteses e do desenho **originais**. O que mudou na execução está nas notas abaixo e no PROTOCOLO.

---

## Resumo

O estudo mede quais estratégias de teste automatizado detectam quais tipos de defeito de áudio em um plugin
VST3, com quantos alarmes falsos, e compara essas estratégias com o pluginval, o validador genérico mais usado
por desenvolvedores JUCE.

O sistema testado é o tapSynth. Os defeitos são injetados em duas origens de código: o código do próprio
projeto e as classes de DSP do JUCE que ele usa. Isso permite separar o efeito do autor (código que nós
escrevemos) do código de terceiros.

Contribuição: evidência empírica sobre qual teste pega qual defeito de áudio, um conjunto de defeitos e uma
infraestrutura reprodutíveis que outros podem reutilizar, e um guia prático para desenvolvedores.

Entregas:

- Artigo do PFC I com os resultados e a discussão.
- Repositório público e reproduzível: testes, código de análise de áudio, defeitos, scripts e pipeline.
- Guia de recomendação: que testes escrever, com que tolerância, e o que o pluginval cobre ou não.

## Perguntas de pesquisa e hipóteses

**Pergunta geral:** como diferentes estratégias de teste automatizado de áudio se comparam, em detecção de
defeitos e em alarmes falsos, na verificação de um plugin VST3?

| Pergunta | Hipótese |
| --- | --- |
| QP1. Que estratégia detecta que categoria de defeito, e com quantos alarmes falsos? | Cada estratégia se especializa: espectral em altura e filtro, temporal em envelope, sanidade em problemas numéricos e de nível. |
| QP2. Como as estratégias de áudio se comparam ao pluginval? | O pluginval detecta defeitos de estado, host e travamento, mas poucos defeitos de conteúdo sonoro. |
| QP3. Como a tolerância afeta o compromisso entre detecção e alarmes falsos? | O golden master detecta mais defeitos que as outras estratégias, mas com mais alarmes falsos, e o equilíbrio depende da tolerância. |
| QP4. O desempenho muda entre defeitos no código do projeto e no código do JUCE? | Não há diferença relevante; se houver, indica viés de autoria. |

A comparação entre frameworks de C++ deixou de ser pergunta de pesquisa e virou uma análise secundária (seção
"Comparação de frameworks").

> **[Nota de execução]** A situação de cada hipótese depois do experimento está na NOTAS, seção 7 (QP1 e QP2
> confirmadas só em parte, QP3 metade, QP4 sem viés de autoria atribuível).

## Objeto de estudo

O sistema testado é o tapSynth: um plugin sintetizador em JUCE 8.0.9, compilado com CMake, com 5 vozes. Cada
voz soma dois osciladores (seno, dente de serra ou quadrada, com FM), aplica ganho fixo, envelope ADSR de
amplitude e um filtro de variáveis de estado com modulação por envelope e LFO. Depois da soma das vozes vêm um
reverb e um medidor de nível.

Todos os testes tocam notas no `TapSynthAudioProcessor` real e analisam o som produzido. Os defeitos são
injetados em duas origens de código:

| Origem | O que entra | Por que importa |
| --- | --- | --- |
| Código do projeto | `OscData` (nota MIDI → frequência, transposição, FM), `SynthVoice` (soma da modulação e limite do cutoff), fórmulas das formas de onda, `MeterData`, `applyParametersFromJson` | É o código específico do plugin, onde ficam os erros de integração |
| Código do JUCE | `juce::ADSR`, `juce::dsp::Oscillator`, `juce::dsp::StateVariableTPTFilter`, `juce::dsp::Reverb`, `juce::dsp::Gain` | Código que não escrevemos e que milhares de plugins usam; reduz o viés de autoria |

Origem do código: o tapSynth começou como o tutorial tapSynth do The Audio Programmer (Joshua Hodge, 2021; os
cabeçalhos dos arquivos em `Source/Data/` mostram isso). Um colega o estendeu com FM, modulação do filtro, nova
interface e o recurso de IA. Este trabalho contribui com os testes, os defeitos, a pipeline e a avaliação. As
três partes devem ser creditadas no artigo e no README.

Fora do escopo: testes de interface gráfica, a chamada à API do Gemini e outros formatos (AU, AAX). O recurso
de IA entra apenas de forma indireta: a estratégia de sanidade aplica patches aleatórios pelo mesmo caminho que
as respostas da IA usam.

## Abordagens de detecção

São cinco abordagens: quatro estratégias de teste de áudio escritas por nós e o pluginval como linha de base.

| Abordagem | Pergunta que responde | Exemplos de verificação |
| --- | --- | --- |
| Sanidade | O som está quebrado? | Sem NaN ou infinito; pico abaixo de um limite; sem silêncio inesperado; sem DC. Variante com patches aleatórios: N conjuntos de parâmetros aplicados por `applyParametersFromJson`, com as mesmas verificações |
| Espectral | As frequências estão certas? | Frequência fundamental de cada nota em 44,1, 48 e 96 kHz; atenuação do passa-baixa acima do cutoff; aliasing das ondas dente de serra e quadrada (teste de caracterização) |
| Temporal | O som evolui certo no tempo? | O ataque atinge o pico no tempo configurado; o release chega ao silêncio; a voz é liberada após o release |
| Golden master | O som mudou? | Áudio renderizado comparado com referências gravadas, em configurações padrão e extremas, com tolerância variada |
| pluginval (linha de base) | O plugin se comporta bem num host? | Carregamento, salvamento e restauração de estado, parâmetros aleatórios, chamadas de várias threads, taxas de amostragem e tamanhos de bloco variados |

Regras do pluginval: roda sobre o VST3 compilado, não sobre o executável de testes. O nível de rigor (de 1 a
10) é fixado antes do experimento e informado no artigo.

Regra das referências do golden master: são gravadas uma única vez, a partir do código sem defeitos, e
versionadas no repositório. Se o sintetizador tiver qualquer aleatoriedade, a semente é fixada nos testes.

## Defeitos injetados e alterações inofensivas

Cada defeito é aplicado sozinho, um por vez, e todas as abordagens rodam contra ele.

Categorias de defeito (definidas antes de criar os defeitos):

| Categoria | Exemplo |
| --- | --- |
| Altura/frequência | Taxa de amostragem fixa em 44,1 kHz no cálculo do oscilador |
| Nível/ganho | Ganho de saída dobrado |
| Tempo/envelope | O release nunca chega ao fim |
| Filtro | Cutoff em unidade errada; limite do cutoff removido |
| Numérico | Filtro instável na ressonância máxima, gerando NaN |
| Estado/host | Restauração de estado ignora um parâmetro; travamento com bloco de 1 amostra; alocação de memória no callback de áudio |

A categoria estado/host existe para a comparação com o pluginval ser justa: se todos os defeitos fossem de
conteúdo sonoro, o pluginval perderia por construção.

Fontes dos defeitos:

1. Baseados em correções reais de plugins JUCE open source: 10 a 15 patches, criados pelo colega (não por quem
   escreve os testes). Cada patch registra a correção real que o inspirou (link do commit ou issue).
2. Mutantes do Mull no código do projeto: gerados automaticamente.
3. Mutantes do Mull nas classes de DSP do JUCE: restritos aos arquivos das classes listadas no objeto de
   estudo.

> **[Nota de execução]** O colega saiu do projeto. Os defeitos foram criados por agentes de IA sem acesso aos
> testes, seguindo o guia `experiment/GUIA_DEFEITOS.pdf` (transcrito em `artigo/GUIA_DEFEITOS.md`), em dois
> lotes: 11 no primeiro e 23 no segundo, este com fontes ampliadas a qualquer software de áudio open source em
> C/C++ (PROTOCOLO, seção 10). Ao todo, 34 correções reais de 12 projetos.

Cada arquivo de defeito tem um cabeçalho com identificador, categoria, origem (projeto ou JUCE) e fonte.
Mutantes que nenhuma abordagem detecta são classificados à parte, porque podem ser equivalentes (não mudam o
comportamento).

Alterações inofensivas (medem alarmes falsos; qualquer falha nelas conta como alarme falso):

- refatoração sem mudança de comportamento;
- build Release em vez de Debug;
- outro sistema operacional (Windows, macOS);
- outro compilador.

## Métricas e análise

Todas as métricas e tolerâncias são fixadas antes da execução, para que os resultados não dependam de escolhas
feitas depois.

| Métrica | Pergunta | Como é medida |
| --- | --- | --- |
| Taxa de detecção | QP1, QP2 | Defeitos detectados ÷ defeitos injetados, por abordagem e por categoria |
| Alarmes falsos | QP1, QP3 | Número de alterações inofensivas que fizeram a abordagem falhar |
| Curva de tolerância | QP3 | Detecção e alarmes falsos de cada estratégia em uma grade de tolerâncias definida antes |
| Detecção por origem | QP4 | Taxa de detecção separada para defeitos no projeto e no JUCE |
| Custo de execução | Discussão | Tempo de execução por abordagem e linhas de código dos testes |

Análise estatística: taxas apresentadas com intervalo de confiança de 95% (Wilson). Como todas as abordagens
rodam sobre os mesmos defeitos, a comparação é pareada: teste Q de Cochran entre as cinco abordagens e testes
de McNemar par a par, com correção para comparações múltiplas.

Apresentação dos resultados:

- matriz defeito × abordagem (detectou ou não);
- taxa de detecção por categoria × abordagem;
- alarmes falsos por abordagem;
- gráfico da curva de tolerância (detecção contra alarmes falsos).

## Arquitetura técnica (resumo)

O plano descreve: o executável `tapSynth_tests` ligando a biblioteca compartilhada `tapSynth` e o GoogleTest
(via FetchContent); uma função de renderização que cria o `TapSynthAudioProcessor` sem editor, chama
`prepareToPlay`, define parâmetros pelo `apvts`, monta um `juce::MidiBuffer` com note-on e note-off e chama
`processBlock` em laço; funções de análise compartilhadas (`rms`, `peak`, `fundamentalHz` com
`juce::dsp::FFT`, `bandEnergyDb`, `envelope`, `maxAbsDiff`, `loadWav`, `saveWav`); a estrutura de pastas
(`tests/common`, `tests/sanity|spectral|temporal|golden`, `tests/references`, `experiment/faults`,
`experiment/harmless`, `experiment/run_experiment.py`, `experiment/results`); um clone local do JUCE 8.0.9
para os defeitos no JUCE (`-DTAPSYNTH_JUCE_DIR`); o Mull rodando no Linux, limitado a `Source/` (sem
`Source/UI/`) e aos arquivos das classes JUCE listadas; a CI no GitHub Actions (`build.yml`, `tests.yml` nos
três sistemas e um `mutation.yml` sob demanda); e o script do experimento, que para cada defeito aplica o
patch, compila, roda os testes e o pluginval, registra no `results.csv` (`defect_id`, `origin`, `category`,
`approach`, `tolerance`, `detected`, `runtime_s`) e desfaz o patch.

> **[Nota de execução]** O que foi de fato construído e como rodou está no PROTOCOLO e no CLAUDE.md. Por
> exemplo, os mutantes rodaram localmente no Linux, não num `mutation.yml`.

## Controle de viés e regras para o Claude Code

A maior ameaça à validade é quem escreve os testes conhecer os defeitos. Estas regras valem para todos,
pessoas e IA:

1. Testes congelados antes dos defeitos. A suíte completa é escrita e marcada com a tag git `tests-frozen`
   antes de qualquer defeito existir. Depois disso, os testes não mudam.
2. Defeitos realistas criados pelo colega, em uma branch ou repositório separado, sem informar a quem escreve
   os testes o que são.
3. O Claude Code que escreve testes não vê `experiment/faults/`. A pasta só entra no repositório depois do
   congelamento.
4. Tolerâncias, grade de tolerâncias e nível do pluginval fixados antes da execução, registrados em commit.
5. Uso de IA declarado na seção de Materiais e Métodos: o que foi gerado com ajuda de IA e como foi revisado.
6. Toda mudança revisada por uma pessoa antes do commit.
7. A chave da API do Gemini nunca entra no repositório.

## Plano de execução (fases)

- **Fase 0, viabilidade (1 a 2 dias):** criar `tapSynth_tests` e renderizar uma nota; compilar com o JUCE
  local; confirmar mutantes do Mull em `Source/` e nas classes JUCE em cabeçalhos; rodar o pluginval e escolher
  o nível de rigor.
- **Fase 1, testes (3 a 4 dias):** renderização e análise em `tests/common/`; testes de sanidade (com patches
  aleatórios), espectrais (44,1, 48 e 96 kHz) e temporais; gravar as referências do golden master; fixar
  tolerâncias e grade em `tests/common/Tolerances.h`; criar a tag `tests-frozen`.
- **Fase 2, defeitos (em paralelo, pelo colega):** levantar 10 a 15 correções reais cobrindo todas as
  categorias; transformá-las em patches com cabeçalho; configurar o escopo do Mull; preparar as alterações
  inofensivas.
- **Fase 3, execução (2 a 3 dias):** `run_experiment.py`; rodar defeitos, mutantes e alterações inofensivas;
  gerar o `results.csv`; rodar a varredura de tolerâncias.
- **Fase 4, análise e escrita:** tabelas, gráfico da curva de tolerância e testes estatísticos; resultados,
  discussão e guia de recomendação; preparar a demonstração da apresentação (defeito de taxa de amostragem
  tocado ao vivo e o teste espectral falhando na CI).

> **[Nota de execução]** A demonstração prevista não funciona: o F01 (o defeito de taxa de amostragem) não foi
> detectado (NOTAS).

## Comparação de frameworks (secundária)

O experimento usa só o GoogleTest. A comparação com Catch2 e doctest é uma análise de recursos, ilustrada com
um teste representativo ("toda nota tem a altura certa") portado para cada framework.

| Necessidade do teste de áudio | GoogleTest | Catch2 | doctest |
| --- | --- | --- | --- |
| Comparação aproximada | `EXPECT_NEAR` (absoluta), `EXPECT_FLOAT_EQ` (baseada em ULP) | `WithinAbs`, `WithinRel`, `WithinULP` | `Approx` (`epsilon` relativo, mais `scale`) |
| Mesmo teste para vários valores | `TEST_P` + `INSTANTIATE_TEST_SUITE_P` | `GENERATE`, casos de teste com template | Laços, `SUBCASE` |
| Verificações de áudio próprias | Macros `MATCHER` do gMock | Classes de matcher próprias | Sem sistema de matchers; usar funções auxiliares |
| Contexto extra quando falha | `SCOPED_TRACE` | `INFO`, `CAPTURE` | `INFO`, `CAPTURE` |
| Relatório legível pela CI | XML via `--gtest_output` | Reporter JUnit | Reporter JUnit |

Ponto relevante para áudio: a tolerância do `Approx` do doctest é relativa, então ±0,5% vale 0,55 Hz em 110 Hz
e 8,8 Hz em 1760 Hz, enquanto ±2 Hz absoluto vale o mesmo em toda a faixa.

> **[Nota de execução]** O resultado está em `experiment/frameworks/COMPARACAO.md`.

## Pendências do repositório (do plano)

Verificar a licença do tutorial original e adicionar um LICENSE; criar um README que credite o tutorial, o
colega e este trabalho; tirar a chave da API do código-fonte; atualizar o CLAUDE.md; confirmar o trabalho
anterior de `tapsynth_core` e GoogleTest.

## Limitações e questões em aberto

O estudo é um estudo de caso: os resultados valem para sintetizadores subtrativos simples em JUCE e são um
primeiro ponto de dados, não regras gerais.

Limitações conhecidas:

- Um único plugin; outros tipos (compressores, reverbs, sintetizadores complexos) podem se comportar diferente.
- Resultados ligados ao JUCE 8.0.9.
- Os defeitos injetados podem não representar todos os defeitos reais; mitigado pelas correções reais e pela
  mutação automática.
- Um único framework no experimento.
- Viés de autoria; mitigado pelo congelamento dos testes, pelos defeitos criados pelo colega e pela comparação
  entre origens (QP4).

Questões em aberto (no momento do plano): datas de entrega do artigo e da apresentação; validar tema,
perguntas e escopo com o orientador; nível de rigor do pluginval; valores das tolerâncias e da grade; número de
patches aleatórios na variante de sanidade; se o Mull gera mutantes nas classes JUCE em cabeçalhos.

> **[Nota de execução]** As questões técnicas foram resolvidas na execução (pluginval no rigor 10, semente 42;
> tolerâncias em `tests/common/Tolerances.h`; 200 patches aleatórios; o Mull gerou mutantes no JUCE). Ver o
> PROTOCOLO. Datas e validação com o orientador: perguntar ao autor.

## Referências (como estão no plano)

O plano diz: "As três primeiras foram verificadas nas fontes; as demais vieram de memória e precisam ser
conferidas antes de entrar no artigo."

Verificadas no plano:

- VALENTAN, A.; WOTAWA, F. On the Automation of Audio Plugin Testing. QRS 2021, p. 270–278. IEEE Xplore —
  trabalho mais próximo; base do desenho com mutação.
- BOGDAN, A.; VERANO MERINO, M.; MALAVOLTA, I. [título exato a copiar da página do artigo; estudo de mineração
  sobre plugins VST open source]. MSR 2025, p. 513–525. dblp — lacuna nas práticas de teste.
- BALTES, S. et al. Guidelines for Empirical Studies in Software Engineering involving Large Language Models.
  arXiv — base para declarar o uso de IA.

A verificar:

- BARR, E. T. et al. The Oracle Problem in Software Testing: A Survey. IEEE TSE, 2015.
- JIA, Y.; HARMAN, M. An Analysis and Survey of the Development of Mutation Testing. IEEE TSE, 2011.
- DENISOV, A.; PANKEVICH, S. Mull it over: mutation testing based on LLVM. ICSTW, 2018.
- RUNESON, P.; HÖST, M. Guidelines for conducting and reporting case study research in software engineering.
  Empirical Software Engineering, 2009.
- pluginval (Tracktion), repositório no GitHub, citado como software com data de acesso.

> **[Nota de execução]** Correções já conhecidas (NOTAS, seção 10): o artigo do MSR 2025 é de **ANDREI, B.**;
> VERANO MERINO, M.; MALAVOLTA, I., com o título "The Ecosystem of Open-Source Music Production Software: A
> Mining Study on the Development Practices of VST Plugins on GitHub" ("BOGDAN, A." no plano é engano). Em
> Valentan e Wotawa, conferir as iniciais (a busca indica Stephan Valentan). A NOTAS, seção 10, também traz
> referências que não estão no plano (Wang et al., 2022; Andrews et al., 2005; Just et al., 2014; Madeyski et
> al., 2014; Kanewala e Bieman). Quando o plano e a NOTAS divergirem, vale a NOTAS. Toda referência ainda não
> conferida leva [VERIFICAR] no artigo.
