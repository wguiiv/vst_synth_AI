# Protocolo do experimento (fase 3)

Regras fixadas antes de qualquer execução (plano, regra 4). Este arquivo é commitado antes da primeira rodada; mudanças depois disso ficam registradas como desvios na seção 10, com data e motivo.

## 1. Versões congeladas

| Item | Valor |
| --- | --- |
| Testes | tag `tests-frozen` (commit `6d9da97`) |
| Defeitos e alterações inofensivas | commit `4b4bf95` em `fase2-defeitos` (`experiment/faults/`, `experiment/harmless/`) |
| JUCE | 8.0.9 (clone local em `tools/JUCE` para os defeitos no JUCE) |
| pluginval | 1.0.4, `--strictness-level 10 --random-seed 42 --skip-gui-tests --validate-in-process` (`experiment/run_pluginval.sh`) |
| Tolerâncias e grades | `tests/common/Tolerances.h` na tag `tests-frozen` |
| Mull | 0.36, LLVM 19 |

## 2. Ambiente de referência

Linux (Debian, gcc 14.2.0), build Release, taxa de amostragem e blocos definidos pelos próprios testes. Toda condição é comparada com esta referência. Os outros sistemas operacionais rodam no GitHub Actions (seção 4).

## 3. Abordagens e definição de detecção

| Abordagem | O que roda |
| --- | --- |
| sanidade | testes `*Sanity*` |
| espectral | testes `*Spectral*` |
| temporal | testes `*Temporal*` |
| golden | testes `*Golden*` |
| pluginval | `run_pluginval.sh` sobre o VST3 compilado |

`AnalysisSelfTest` verifica as medições em sinais sintéticos e não é uma abordagem; ele roda em toda condição, mas não entra nos resultados. Se falhar numa condição, a condição é registrada como inválida e investigada.

Uma abordagem **detecta** uma condição quando pelo menos um de seus testes falha. Contam como falha: asserção que falha, travamento (crash ou sinal), e tempo esgotado. O pluginval detecta quando termina com código diferente de zero ou estoura o tempo.

Cada teste roda em um processo separado, então um travamento afeta só aquele teste.

Tempos-limite: 60 s por teste (o teste mais lento leva cerca de 9 s na referência) e 15 min por execução do pluginval.

## 4. Condições

Cada condição é aplicada sozinha, sobre o código limpo, e desfeita ao final.

| Tipo | Condições | Onde roda |
| --- | --- | --- |
| referência | código limpo, 5 execuções | Linux local |
| defeito | F01 a F11 (`experiment/faults/`) | Linux local |
| inofensiva | H01 a H04 (`experiment/harmless/`) | Linux local |
| inofensiva | build Debug em vez de Release | Linux local |
| inofensiva | outro compilador: clang 19 em vez de gcc 14 | Linux local |
| inofensiva | outro sistema: Windows e macOS, código limpo, 5 execuções cada | GitHub Actions |
| mutante | mutantes do Mull (seção 7) | Linux local |

Um patch que não aplica ou não compila não é executado: fica registrado como `nao_compila` e sai do denominador.

Qualquer falha numa condição inofensiva conta como alarme falso da abordagem que falhou.

## 5. Referência e testes instáveis

A referência roda 5 vezes no Linux e 5 vezes em cada sistema da CI. Todas devem passar. Os testes estão congelados: se algum falhar de forma intermitente, ele não é corrigido; a instabilidade é registrada e relatada como resultado.

## 6. Varredura de tolerâncias (QP3)

Cada verificação com tolerância grava o valor medido no relatório XML do GoogleTest (`<tolerância>|<rótulo>`). A varredura é calculada a partir desses valores, sem rodar de novo:

1. Uma tolerância por vez percorre sua grade; as demais ficam no valor padrão.
2. Num ponto da grade, um teste falha se algum valor gravado para aquela tolerância viola o ponto, ou se o teste falhou por um motivo que não é uma tolerância.
3. "Motivo que não é uma tolerância": o teste falhou no padrão, mas nenhum valor gravado viola o padrão (por exemplo, asserção sem tolerância, travamento, tempo esgotado). Esse teste conta como falha em todos os pontos da grade.
4. Valores `nan` violam qualquer ponto da grade. Valores `inf` são comparados como na própria asserção do teste: falham num limite "no máximo" e passam num limite "no mínimo".
5. As tolerâncias com grade de um único valor (as de sanidade) não variam.

Para cada abordagem e cada ponto da grade, calcula-se a taxa de detecção nos defeitos e mutantes e o número de alarmes falsos nas condições inofensivas.

## 7. Mutantes

- Escopo: os arquivos de `Source/` sem `Source/UI/` e sem `PluginEditor.*`; e no JUCE, os arquivos de `juce::ADSR`, `juce::dsp::Oscillator`, `juce::dsp::StateVariableTPTFilter`, `juce::dsp::Reverb` (e o `juce::Reverb` que ele envolve) e `juce::dsp::Gain`.
- Operadores: o conjunto padrão do Mull 0.36, fixado em `mull.yml` e commitado antes da execução.
- Execução: cada mutante é ativado sozinho no executável de testes instrumentado, e os testes rodam com o mesmo relatório XML das outras condições, de modo que a varredura de tolerâncias vale também para os mutantes.
- Quantidade: todos os mutantes, se a execução completa couber em 24 h nesta máquina (2 núcleos). Se não couber, uma amostra aleatória simples com semente 42, estratificada por origem (projeto e JUCE), com o tamanho decidido a partir da contagem total e registrado aqui antes da execução.
- pluginval nos mutantes: só se um VST3 instrumentado pelo Mull permitir ativar um mutante por execução. Isso é testado antes; se não funcionar, os mutantes comparam apenas as quatro estratégias, e a limitação é relatada.
- Mutantes que nenhuma abordagem detecta são classificados à parte como possivelmente equivalentes. As taxas são relatadas com e sem eles.

### 7.1 Decisões fixadas antes da execução dos mutantes (2026-11-03)

- **Configuração:** `experiment/mull.yml`. Os operadores são o grupo `cxx_default` (incremento, aritmética, comparação e limites). Uma lista vazia no Mull 0.36 dá exatamente esse grupo; isso foi conferido num programa de teste.
- **Build:** um único build instrumentado do código congelado, com clang 19, Release, `-g -grecord-command-line -ffp-contract=off` e o plugin do Mull (`run_experiment.py mutants`). Sem `-ffp-contract=off`, o clang junta `a * b + c` numa só operação (`fmuladd`) que o Mull 0.36 não muta, e boa parte da aritmética das fórmulas de DSP ficaria sem mutantes. Em x86-64 sem FMA, como no build de referência com gcc, essa operação vira uma multiplicação e uma soma de qualquer forma, então a saída não muda.
- **Ativação:** o Mull compila todos os mutantes nos binários, e cada um é ativado por uma variável de ambiente com o seu id. Os testes e o pluginval rodam com essa variável, sem recompilar. Verificado antes da execução: o mutante M0044 derruba o pluginval (falha de segmentação) e o M0075 faz o golden falhar; sem mutante ativo, tudo passa.
- **Verificação do build instrumentado:** sem mutante ativo, os 80 testes e o pluginval passam (`experiment/results/mutants-check/`).
- **Contagem:** 225 mutantes, 94 no projeto e 131 no JUCE. A lista numerada (M0001 a M0225) fica em `experiment/results/mutants.csv`.
- **Quantidade:** cerca de 3 min por mutante na execução de teste, 11 h no total, dentro do limite de 24 h. Todos os mutantes são executados, sem amostra.
- **Tempos-limite:** o código instrumentado roda cerca de 16 vezes mais devagar, porque o Mull consulta a variável de ambiente a cada chamada de função mutada. Nos mutantes, cada teste tem como limite 10 vezes o seu tempo na verificação sem mutante, com mínimo de 60 s (a mesma regra do executor do próprio Mull). O pluginval mantém 15 min.
- **Testes pulados:** num mutante, quando um teste de uma abordagem trava ou estoura o tempo, os demais testes dessa abordagem não rodam. Pela regra 6.3, esse teste já falha em todos os pontos da grade, então o resultado da abordagem não muda; a regra só evita que um mutante que trava consuma um tempo-limite por teste.
- **Limitação conhecida do Mull 0.36:** em código de template, quando há sobrecargas de `operator*` ou `operator+` visíveis no namespace, o detector de mutantes inválidos do Mull descarta os mutantes de `*` e `+`. É o caso do JUCE (`StateVariableTPTFilter`, `Oscillator`, `Gain`): esses arquivos ficam só com mutantes de divisão, comparação e incremento. Desligar o detector traz esses mutantes de volta, mas também cria mutantes inválidos (por exemplo, um `++` sobre uma soma), então o padrão do Mull foi mantido. Os mutantes do JUCE vêm sobretudo de `juce_Reverb.h` (78) e `juce_ADSR.h` (43), e isso é relatado na QP4.

## 8. Saídas

`experiment/results/results.csv`, uma linha por condição × abordagem × ponto de tolerância:

| Coluna | Conteúdo |
| --- | --- |
| `defect_id` | `F01`…, `H01`…, `debug`, `clang`, `windows`, `macos`, `baseline`, ou o id do mutante |
| `kind` | `referencia`, `defeito`, `inofensiva`, `mutante` |
| `origin` | `projeto` ou `juce` (vazio para as condições sem patch) |
| `category` | categoria do defeito (vazio fora dos defeitos) |
| `approach` | `sanidade`, `espectral`, `temporal`, `golden`, `pluginval` |
| `tolerance` | `padrao`, ou `<nome>=<valor>` na varredura |
| `run` | número da execução (referência e outros sistemas) |
| `detected` | 0 ou 1 |
| `status` | `ok`, `nao_compila`, `invalida` |
| `runtime_s` | tempo da abordagem nessa condição |

Os relatórios XML brutos ficam em `experiment/results/raw/`, um diretório por condição.

## 9. Análise (fixada agora, executada na fase 4)

- Taxa de detecção por abordagem e por categoria, com intervalo de confiança de 95% (Wilson).
- Alarmes falsos por abordagem.
- Comparação pareada: teste Q de Cochran entre as cinco abordagens e McNemar par a par, com correção de Holm.
- Detecção por origem (projeto e JUCE), para a QP4.
- Curva de tolerância: detecção contra alarmes falsos, por abordagem.
- Custo: tempo por abordagem e linhas de código dos testes.

## 10. Desvios do plano

- Os defeitos foram criados por um agente de IA sem acesso aos testes, e não pelo colega previsto no plano. Os patches seguem o guia em `experiment/GUIA_DEFEITOS.pdf` e foram revisados pelo autor do estudo.
- São 11 defeitos e 4 refatorações inofensivas.
- Nos mutantes, o tempo-limite por teste deixa de ser 60 s fixos e passa a ser 10 vezes o tempo do teste no build instrumentado sem mutante, com mínimo de 60 s (seção 7.1), porque a instrumentação do Mull deixa os testes até 16 vezes mais lentos.
- 2026-11-03, durante a execução dos mutantes: o M0038 troca `i++` por `i--` no laço que cria as 5 vozes, e o laço passa a alocar vozes sem fim. Sem limite de memória, o processo encheu a RAM da máquina (7,8 GB, sem swap), e o OOM killer do kernel derrubou também o executor, depois de 37 mutantes. A partir do M0038, todo processo de teste e do pluginval roda com o espaço de endereçamento limitado a 4 GiB (`RLIMIT_AS`). Assim, a alocação falha dentro do próprio teste (`std::bad_alloc`, que o GoogleTest registra como falha). Verificado antes de retomar: sem mutante ativo, os 80 testes e o pluginval passam mesmo com limite de 1 GiB, então o limite não muda o resultado de nenhum mutante que não esgote a memória, e os 37 já executados continuam válidos.
- 2026-11-03, durante a execução dos mutantes: por volta das 12:03, no M0051, a Hostinger desligou a VPS por suspeita de malware. A verificação da máquina não encontrou sinal de invasão; a causa provável é um falso positivo do scanner da Hostinger diante da própria execução (binários instrumentados, processos que travam de propósito, CPU alta por horas). Os 50 mutantes concluídos foram mantidos e o M0051 recomeçou do zero. A partir daí, o executor roda com prioridade mínima (`nice -n 19`) e preso a um núcleo (`taskset -c 0`). Os testes já rodavam um de cada vez, e os tempos-limite são 10 vezes o tempo de cada teste, então isso não muda os resultados; só os tempos registrados (`runtime_s`) dos mutantes a partir do M0051 podem ficar um pouco maiores.

### Emenda: segundo lote de defeitos (2026-11-05, antes de existir qualquer defeito do lote)

Motivo: com 11 defeitos, os intervalos de confiança são largos e nenhuma diferença entre abordagens é
significativa nos defeitos (RESULTADOS, seção 7). Um segundo lote aumenta a amostra de defeitos reais. Decidido
antes de qualquer patch do lote existir:

- **Autor:** um novo agente de IA, em sessão nova, num repositório sem testes e sem histórico, como no primeiro
  lote. Ele não tem acesso aos testes, aos resultados, a este repositório nem aos patches F01–F11. Segue o mesmo
  guia (`GUIA_DEFEITOS.pdf`), com um adendo: ids a partir de F12; meta de 40 defeitos, entregando os que
  encontrar sem forçar adaptações; sem alterações inofensivas; sem reutilizar as 11 correções do primeiro
  lote (recebe só a lista dos links).
- **Fontes ampliadas:** além do JUCE e de plugins JUCE, vale a correção publicada de qualquer software de áudio
  open source em C/C++ (plugins, sintetizadores, bibliotecas de DSP, frameworks de áudio), desde que o bug
  tenha equivalente no código do tapSynth ou nas classes do JUCE que ele usa. Continua valendo: todo defeito
  reintroduz uma correção real, com o link aberto e conferido.
- **Quem escreveu os testes não escreve, não escolhe nem altera os patches;** só os executa.
- **Todo patch entregue entra na análise.** Nenhum é descartado depois de executado. Um patch só pode ser excluído
  antes da execução, e só se não aplicar ou não compilar; a exclusão é registrada aqui. O número entregue é
  o número final; não há lote adicional depois de ver os resultados.
- **Revisão pelo autor do estudo:** só formato, cabeçalho, link da fonte e se aplica e compila; nenhuma mudança
  de conteúdo.
- **Execução:** igual à do primeiro lote (seção 4): Linux local, build Release sobre `tests-frozen`, mesmas
  abordagens e tolerâncias. As condições inofensivas não são repetidas.
- **Análise:** os defeitos são relatados em três conjuntos: lote 1 (F01–F11, como já relatado), lote 2 e todos
  juntos. As conclusões sobre defeitos usam o conjunto completo; o lote 1 continua relatado separadamente.

**Entrega e exclusões (2026-11-05, antes de executar qualquer defeito do lote 2):** o agente entregou 25 patches
(F12–F36) de 40 pedidos, com o relatório `experiment/faults/RELATORIO-F12-F36.md` (bugs considerados e deixados
de fora, e adaptações que se afastam do original). Os 25 links de fonte foram abertos e conferidos contra os
patches. Dois patches foram excluídos antes da execução, por um motivo que a regra acima não previa: a mudança
no código do tapSynth é idêntica, linha por linha, à de um defeito do lote 1, embora venha de outro bug real.
Executá-los repetiria a mesma condição e contaria o mesmo resultado duas vezes.

- F22 (fonte: odin2 465bfe2) é idêntico ao F01: remove `fmOsc.prepare (spec);`.
- F27 (fonte: sfizz 54d2b73) é idêntico ao F05: remove o limite do cutoff modulado.

A exclusão não favorece nenhum resultado: o F05 foi detectado e o F01 não. Os dois patches ficam em
`experiment/faults/excluidos/`, fora da execução. O lote 2 analisado tem 23 defeitos; o total, 34. Os demais 23
aplicam sobre o código limpo (conferido com `git apply --check`). O F31 se sobrepõe em parte ao F11 (os dois
retiram a guarda de bloco vazio do medidor, mas o F31 mantém a de zero canais) e foi mantido, porque a mudança
é diferente.

### Emendas antes da primeira execução

- 2026-11-03, regra 6.4: a versão anterior dizia que `inf` viola qualquer ponto da grade. Uma atenuação de `inf` dB é um resultado válido para um limite "no mínimo" e passa na asserção do teste, então a regra faria o código limpo falhar na varredura. Corrigido antes de qualquer execução.
