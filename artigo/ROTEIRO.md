# Roteiro do artigo (TCC / PFC I)

Plano de escrita do artigo, seção por seção. O ângulo e a conclusão estão em
`experiment/analysis/NOTAS.md`, seção 0; os números, em `experiment/analysis/RESULTADOS.md`; as regras da
universidade, em `artigo/universidade/REGRAS.md`. Este arquivo diz **o que vai em cada seção, quanto espaço
ocupa e de onde vem cada coisa**.

## 1. Fio condutor (resumo da NOTAS, seção 0)

Testar software de áudio é difícil, e o estudo mostra concretamente por quê. Foram mineradas 34 correções
reais de *bugs* em 12 projetos de áudio de código aberto, reintroduzidas no tapSynth por agentes que nunca
viram os testes, além de 225 mutantes. Resultado:

1. as ferramentas são complementares: o pluginval verifica o *plugin* como programa (pega travamento, NaN e
   erros de memória), os testes de propriedade verificam o som, e o *golden master* serve de rede contra
   regressões;
2. combinadas, ainda deixam passar 21 de 34 defeitos;
3. o que limita a detecção são as **situações** que os testes criam e os **valores esperados** (oráculo), não
   o rigor das tolerâncias;
4. daí saem recomendações práticas (`experiment/GUIA_RECOMENDACAO.md`).

Tudo no artigo deve servir a esse fio. Números soltos sem interpretação, não.

## 2. Formato e orçamento de páginas

Máximo de 30 páginas, contando tudo (modelo da universidade: entre 15 e 30). Mirar em 26 a 28, para sobrar
folga na formatação final (Arial 12, espaço 1,5, margens 3/3/2/2 cm; ver REGRAS.md).

| Seção | Páginas | Observação |
| --- | --- | --- |
| Título, autor, Resumo, palavras-chave | 1 | resumo em parágrafo único |
| 1 INTRODUÇÃO | 3 a 4 | no máximo 15% do artigo (aula) |
| 2 PROCEDIMENTO EXPERIMENTAL | 7 a 8 | 2.1 MATERIAIS (2,5 a 3) e 2.2 MÉTODOS (4,5 a 5) |
| 3 RESULTADOS E DISCUSSÕES | 10 a 12 | a maior seção |
| 4 CONSIDERAÇÕES FINAIS | 1,5 a 2 | responde a pergunta-problema |
| AGRADECIMENTOS | até 0,3 | opcional, só o autor escreve |
| REFERÊNCIAS | 1,5 a 2 | ABNT NBR 6023 |

Se faltar espaço, cortar nesta ordem: comparação de frameworks (pode virar um parágrafo), detalhes dos
mutantes equivalentes, tabela de custo (vira uma frase).

## 3. Seção por seção

### Título

Sugestões (decisão do autor): "TESTES AUTOMATIZADOS DE ÁUDIO EM *PLUGINS* VST3: UM ESTUDO DE CASO COM DEFEITOS
REAIS" ou "POR QUE É DIFÍCIL TESTAR *SOFTWARE* DE ÁUDIO: UM ESTUDO DE CASO COM DEFEITOS REAIS EM UM *PLUGIN*
VST3". O título deve refletir o fio condutor e o tipo de estudo.

### Resumo e palavras-chave

- Um parágrafo: contexto (uma frase), objetivo, método (tapSynth, quatro estratégias e pluginval; 34 defeitos
  reais de 12 projetos e 225 mutantes; 8 condições inofensivas), resultados principais (13 de 34; pluginval 2;
  zero alarmes falsos; 16 dos 21 perdidos sem nenhum valor medido alterado; tolerância decidiu 1) e conclusão
  (complementares; situações e valores esperados, não rigor).
- A ABNT NBR 6028 recomenda de 100 a 250 palavras para artigos; conferir com o orientador.
- Palavras-chave (3 a 5), por exemplo: Teste de *software*. *Plugins* de áudio. Processamento de sinais de
  áudio. Teste de mutação. Oráculo de teste.
- Escrever por último.

### 1 INTRODUÇÃO (3 a 4 páginas)

Funil, do geral ao problema (aula), **sem antecipar resultados nem conclusões** (modelo):

1. **Contexto e abertura com impacto:** *plugins* de áudio rodam dentro de programas de terceiros (*hosts*), em
   tempo real, em muitas configurações; um erro pode ser um som errado que só alguém ouvindo percebe. Usar só
   afirmações com referência (Valentan e Wotawa, 2021, marcado [VERIFICAR] para a afirmação sobre teste manual;
   Andrei, Verano Merino e Malavolta, 2025, mineração de 299 repositórios de *plugins* VST; Wang et al., 2022,
   automação de testes e qualidade em outro domínio). Nada de estatística inventada.
2. **Problema:** como testar automaticamente o som? Não há uma saída "certa" óbvia para comparar (problema do
   oráculo; Barr et al., 2015, a conferir), e o *plugin* roda em muitas situações. O validador mais usado
   (pluginval) verifica o comportamento do *plugin* como programa, não o som.
3. **Lacuna:** não foi encontrado estudo empírico comparando estratégias de teste de áudio entre si e com um
   validador; o trabalho mais próximo (Valentan; Wotawa, 2021) avalia geração de entradas para comparação de
   saídas (NOTAS, seção 10). Um parágrafo, não uma revisão.
4. **Pergunta-problema** (do plano, literal): "como diferentes estratégias de teste automatizado de áudio se
   comparam, em detecção de defeitos e em alarmes falsos, na verificação de um *plugin* VST3?"
5. **Objetivos:** geral (comparar as estratégias sanidade, espectral, temporal e *golden master* e o pluginval
   no tapSynth, em detecção e alarmes falsos); específicos = QP1 a QP4 reescritas como objetivos, mais a
   entrega do guia de recomendação.
6. **Hipóteses** (do plano, uma frase cada; a aula permite): QP1 especialização; QP2 pluginval pega estado,
   host e travamento, poucos defeitos de som; QP3 *golden* detecta mais com mais alarmes falsos; QP4 sem
   diferença entre origens.
7. **Justificativa:** acadêmica (lacuna), profissional (guia prático para desenvolvedores; o pluginval é o
   padrão da comunidade JUCE), social (repositório aberto e reprodutível), pessoal [AUTOR].
8. **Método em poucas linhas:** estudo de caso exploratório, de natureza aplicada, abordagem mista, com
   procedimento experimental controlado.

### 2 PROCEDIMENTO EXPERIMENTAL (7 a 8 páginas)

Abrir com a classificação da pesquisa (aula): natureza **aplicada**; abordagem **mista** (quantitativa nas
taxas e testes estatísticos, qualitativa na análise de cada defeito); objetivos **exploratórios e
descritivos**; procedimento **estudo de caso** com **experimento controlado** (Runeson; Höst, 2009, a
conferir). Verbos no passado, detalhe suficiente para replicar, citar o repositório.

#### 2.1 MATERIAIS (2,5 a 3 páginas)

- **Objeto:** tapSynth, sintetizador VST3 em JUCE 8.0.9 (C++17, CMake), 5 vozes, dois osciladores com FM,
  envelope ADSR, filtro de variáveis de estado com envelope e LFO, *reverb*, medidor (NOTAS, seção 1;
  `CLAUDE.md`, Architecture). **Fig. 1:** diagrama do caminho do sinal [FIGURA: a criar]. A função de IA do
  *plugin* (texto → parâmetros via Gemini) não é objeto do estudo; os testes não usam o editor, só a entrada
  JSON nos *patches* aleatórios.
- **Estratégias de teste:** **Tab. 1** = tabela da NOTAS, seção 1 (estratégia, nº de testes, o que verifica).
  Explicar na primeira menção: amostra, taxa de amostragem, FFT, cents, dB, *golden master*. Importante: aqui
  "teste de propriedade" significa verificar propriedades esperadas do som (altura, atenuação, tempos), não
  *property-based testing* com gerador de entradas (o único uso de entradas aleatórias são os 200 *patches* da
  sanidade). Todos os testes renderizam o processador real (`tests/common/Render.h`).
- **pluginval** 1.0.4, rigor 10, semente 42, sem testes de interface (`experiment/run_pluginval.sh`).
- **Ferramentas e ambiente:** GoogleTest; Mull 0.36 com LLVM 19 (mutantes); gcc 14.2 (referência) e clang;
  Linux (VPS com 2 núcleos AMD EPYC, 7 GB de RAM) para detecção; GitHub Actions (Windows e macOS) só para
  alarmes falsos; análise em Python 3.13, só com a biblioteca padrão (`experiment/analyze.py`).
- **Amostra e como foi determinada:**
  - 34 defeitos reais em dois lotes (**Qd. 1**: fontes, a partir de NOTAS, seção 2, e `faults/INDEX.md`):
    critérios de inclusão (bug real corrigido, link conferido, equivalente possível no tapSynth), quantidade
    pedida e entregue (lote 2: 40 pedidos, 25 entregues, 2 excluídos por duplicar F01 e F05);
  - 225 mutantes do Mull (94 no projeto, 131 no JUCE; operadores de comparação, aritméticos e de incremento;
    PROTOCOLO, seção 7);
  - 8 condições inofensivas (4 refatorações, Debug, clang, Windows, macOS) e a referência (5 execuções).

#### 2.2 MÉTODOS (4,5 a 5 páginas)

- **Desenho contra viés** (PROTOCOLO): testes congelados na tag `tests-frozen` antes de qualquer defeito;
  defeitos escritos por agentes de IA sem acesso aos testes, seguindo um guia; protocolo fixado antes;
  desvios registrados (PROTOCOLO, seção 10). **Dois lotes:** dizer que o segundo foi acrescentado para
  ampliar a amostra, com regras fixadas antes de sua criação, e que a decisão veio depois do lote 1 (vai
  também nas limitações). Não esconder nem dramatizar.
- **Variáveis:** independentes (abordagem; tipo de condição; origem projeto/JUCE; tolerância); dependentes
  (detecção, alarme falso, custo em tempo e linhas); controle (mesmo *commit*, build Release, semente fixa,
  Linux).
- **Execução** (PROTOCOLO, seção 4): aplicar o *patch*, compilar, rodar os 80 testes (um processo por teste)
  e o pluginval; definição de detecção (falha nas tolerâncias padrão em pelo menos uma execução; travamento e
  tempo esgotado contam); valores medidos gravados no relatório XML.
- **Varredura de tolerâncias** (PROTOCOLO, seção 6): grade por tolerância, recalculada a partir dos valores
  gravados.
- **Mutantes equivalentes** (PROTOCOLO, seção 7): os não detectados por nenhuma abordagem são "possivelmente
  equivalentes"; taxas com e sem eles (Madeyski et al., 2014).
- **Estatística:** intervalo de Wilson 95%; Q de Cochran entre as cinco abordagens; McNemar exato par a par
  com correção de Holm; significância 0,05. Explicar em uma frase cada teste, para a banca.
- **Aspectos éticos:** sem participantes humanos; código de terceiros usado conforme as licenças abertas;
  declaração de uso de IA conforme NOTAS, seção 12, e Baltes et al. (testes, executor e análise com o Claude
  Code; defeitos por agentes separados; redação com o Claude; tudo revisado pelo autor).
- **Análise secundária** (um parágrafo): comparação de *frameworks* (`frameworks/COMPARACAO.md`).

### 3 RESULTADOS E DISCUSSÕES (10 a 12 páginas)

Seguir o roteiro da aula: relembrar objetivos e hipóteses → resultados com tabelas e figuras, explicando o
que cada um significa → comparação com a literatura → implicações → limitações → trabalhos futuros (os dois
últimos podem ficar na discussão ou nas considerações finais, sem repetir).

Ordem sugerida das subseções (seções terciárias, títulos ligados ao tema):

1. **Validade das execuções e alarmes falsos** (0,5 a 1 p.): todas as execuções válidas; zero alarmes falsos
   nas tolerâncias padrão; diferenças entre plataformas (NOTAS, seção 5). **Tab.:** RESULTADOS, seção 6
   (resumida).
2. **Detecção por abordagem** (2 p.; QP1, QP2, QP4): **Tab. 2:** RESULTADOS, seção 2 (defeitos: total e por
   lote; mutantes: todos e sem os possivelmente equivalentes), com IC. **Fig. 2** opcional: barras de
   detecção por abordagem [FIGURA: a criar a partir de RESULTADOS, seção 2]. Explicar o que cada número
   significa; não comparar os lotes como "melhora".
3. **Por que os defeitos passaram** (2,5 a 3 p.; o centro do artigo): **Tab. 3:** os cinco grupos da NOTAS,
   seção 3 ("Achado principal"), com exemplos concretos (F01, F08, F14, F16, F17, F36). Mostrar as duas
   dificuldades: situações e oráculo. Os 5 detectados só pelo *golden* entram aqui, como exemplos de oráculo.
4. **Complementaridade das ferramentas** (1,5 a 2 p.; QP2): pluginval contra testes (McNemar, RESULTADOS,
   seção 7; só os pares relevantes); os 5 erros de memória; o F02; o F36 e a verificação de estado do
   pluginval; o papel do *golden* e por que o desenho o favorece (NOTAS, seção 8, ressalva 1).
5. **Tolerâncias** (1 a 1,5 p.; QP3): **Fig. 3:** `experiment/analysis/curva_tolerancia.svg`; curva plana, o
   penhasco, o F17 em 10⁻⁵ (NOTAS, seção 6).
6. **Custo** (0,5 p.): RESULTADOS, seção 9, em uma tabela pequena ou em texto.
7. **Discussão** (2 a 3 p.):
   - comparação com a literatura (NOTAS, seção 10): Valentan e Wotawa (2021); Andrei, Verano Merino e
     Malavolta (2025); mutantes como substitutos de defeitos reais (Andrews et al., 2005; Just et al., 2014);
     oráculo (Barr et al., 2015, a conferir; Kanewala e Bieman, revisão sistemática no arXiv);
   - "os testes eram ruins?": **Qd. 2** = tabela da NOTAS, seção 0 (lacunas por tipo), com a resposta honesta;
   - situação das hipóteses (NOTAS, seção 7): QP1 e QP2 só em parte, QP3 metade, QP4 sem viés de autoria
     atribuível. Dizer claramente;
   - implicações práticas: o guia (`experiment/GUIA_RECOMENDACAO.md`), em forma de lista curta;
   - limitações e ameaças à validade (NOTAS, seções 8 e 11).

### 4 CONSIDERAÇÕES FINAIS (1,5 a 2 páginas)

- Responder a pergunta-problema diretamente (o modelo exige).
- O que foi alcançado e a contribuição: evidência sobre onde e por que defeitos reais de áudio escapam; um
  conjunto de defeitos e uma infraestrutura reprodutíveis; o guia.
- Limitações em poucas linhas (sem repetir a discussão).
- Trabalhos futuros: medir uma suíte construída a partir das situações e valores esperados identificados;
  outros *plugins* (efeitos, sintetizadores maiores); detecção em Windows e macOS; ferramentas de tempo real.
- Base: rascunho na NOTAS, seção 0. Nada de número novo.

### AGRADECIMENTOS e dados do autor

Só o autor escreve: [AUTOR: agradecimentos], [AUTOR: nome, titulação e e-mail na nota de rodapé],
[AUTOR: orientador, se o modelo da instituição pedir].

### REFERÊNCIAS

- ABNT NBR 6023, uma única ordem. Como as citações são autor-data (NBR 10520/2024), usar **ordem
  alfabética**.
- Fontes permitidas: NOTAS, seção 10, e o plano do estudo. As marcadas "a conferir" levam [VERIFICAR].
  Ferramentas e repositórios (JUCE, pluginval, Mull, GoogleTest, o repositório do estudo) entram como
  documentos *on-line*, com "Disponível em:" e "Acesso em:" ([AUTOR: data de acesso]).

## 4. Tabelas e figuras previstas

| Nº | Conteúdo | Fonte no repositório | Seção |
| --- | --- | --- | --- |
| Fig. 1 | caminho do sinal do tapSynth | `CLAUDE.md` (Architecture); a criar | 2.1 |
| Tab. 1 | estratégias, nº de testes, o que verificam | NOTAS, seção 1 | 2.1 |
| Qd. 1 | defeitos por fonte, lote e categoria | NOTAS, seção 2; `faults/INDEX.md` | 2.1 |
| Tab. 2 | detecção por abordagem, com IC | RESULTADOS, seção 2 | 3 |
| Fig. 2 | barras de detecção (opcional) | RESULTADOS, seção 2; a criar | 3 |
| Tab. 3 | por que os 21 defeitos passaram | NOTAS, seção 3 | 3 |
| Tab. 4 | comparações pareadas relevantes | RESULTADOS, seção 7 | 3 |
| Fig. 3 | curva de tolerância | `analysis/curva_tolerancia.svg` | 3 |
| Tab. 5 | alarmes falsos (resumida) | RESULTADOS, seção 6 | 3 |
| Qd. 2 | lacunas por tipo e o que as resolveria | NOTAS, seção 0 | 3 |

Todas com título acima, numeração crescente e "Fonte: Do autor (2026)" abaixo.
