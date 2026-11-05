# Relatório do segundo lote de defeitos (F12–F36)

Entregues 25 patches de 40 pedidos: faltaram 15. Cada patch aplica sozinho sobre o código limpo, o plugin compila e o código volta limpo depois de desfazer. Nenhum teste, validador ou medição de áudio foi usado, e nenhuma chave de API foi escrita em arquivo.

## Entrega

- Patches: `experiment/faults/F12-….patch` a `F36-….patch`, todos com o cabeçalho completo do guia.
- `experiment/faults/INDEX.md`: só as linhas novas (id, categoria, origem, arquivo alvo, fonte), para acrescentar ao INDEX do estudo.
- Origens: 17 patches no projeto (`Source/`, aplicáveis com `git apply` na raiz) e 8 no JUCE 8.0.9 (`modules/…`, aplicáveis com `git -C <JUCE> apply`).

| categoria | defeitos |
| --- | --- |
| altura | F20, F21, F22 |
| nivel | F19, F30 |
| envelope | F12–F17, F24, F25, F26 |
| filtro | F18, F27, F29 |
| numerico | F31 |
| estado | F23, F28, F32–F36 |

As fontes vêm de JUCE, Odin 2, Helm, amsynth, sfizz, ZynAddSubFX, Tracktion (4OSC), Surge XT, Monique e Dexed. Todos os links foram abertos e mostram a correção descrita. No odin2 465bfe2 a página do commit não exibe o diff do `PMOscillator.h`; a confirmação foi feita pela versão `.diff` do mesmo commit. No JUCE 792f992df a página mostra a linha adicionada, e o contexto que a situa em `noteOn()` foi conferido no clone local do histórico do JUCE.

## Adaptações que se afastam do bug original

- **F19:** o bug era em `AudioBlock::multiply` (sem rampa, copiava sem multiplicar). O mesmo atalho foi levado para `juce::dsp::Gain::process`, que faz a mesma operação em outra classe. No tapSynth, o ganho fixo de 0,07 da voz deixa de ser aplicado.
- **F24:** no original, o timestamp MIDI era usado em segundos como se fosse amostras. Aqui a voz escreve a partir da amostra 0 em vez de `startSample`. O sintoma é o mesmo (a posição da nota no bloco se perde), mas o mecanismo é outro.
- **F26:** o original era o arpejador do Helm sem a taxa de amostragem. Aqui é o envelope do filtro que não recebe `setSampleRate`.
- **F31:** o original acrescentou um retorno antecipado para intervalo vazio no FadeNode. Aqui foi retirada a guarda equivalente do `MeterData`, que passa a dividir por `numSamples = 0`.
- **F23:** o original era um cache de coeficientes de filtro sem atualização inicial forçada. Aqui é o cache `lastType` do oscilador. O oscilador nunca é inicializado e o plugin trava na primeira nota.
- **F36:** o original descartava o retorno de `getparreal()`. Aqui é descartado o retorno de `ValueTree::fromXml`, e nenhum parâmetro é restaurado.
- **F14:** usa só metade do commit (a taxa de release em `noteOff`). A outra metade (validar o estado em `setParameters`) ficou de fora para não usar o mesmo commit duas vezes.

## Bugs encontrados e deixados de fora

- **JUCE c3a98d44c (snap-to-zero no filtro):** o código só roda em `process()`, e o tapSynth usa só `processSample()`.
- **amsynth 724b466 (`undenormalise` do freeverb):** a falha dependia de otimização de compilador antigo; no tapSynth o macro faria o mesmo que o atual.
- **zyn c141f77 na entrada do reverb:** antes do reverb os canais esquerdo e direito são idênticos, então não haveria efeito. O bug foi usado no medidor (F30).
- **odin2 824f50a (profundidade da FM linear):** a FM do tapSynth é exponencial, em semitons.
- **JUCE d9c25ec17:** o próprio JUCE reverteu a correção (69e287bc2).
- **Dexed 70c8231 e ADLplug d5d74f0 (atributo não salvo no estado):** o APVTS salva todos os parâmetros automaticamente.
- **helm ad439c3 (damping sem limite):** a faixa do parâmetro e o JSON já limitam o valor.
- **monique 5d1e5c8 (cutoff máximo alto demais):** ficaria parecido demais com o F27.
- **JUCE c1ae3ab7a (LadderFilter) e helm 9eda4c3:** a adaptação seria inventada.
- **Correções de velocity, pitch bend e glide:** o tapSynth não tem esses recursos.

## Pontos para a revisão

- O comando de build do `CLAUDE.md` falha como está escrito: o `CMakeLists.txt` liga `TAPSYNTH_BUILD_TESTS` por padrão e chama `add_subdirectory(tests)`, e a pasta não existe. A compilação foi feita com `-DTAPSYNTH_BUILD_TESTS=OFF`. Os testes não foram procurados nem abertos.
- Os patches F01–F11 não estão disponíveis nesta cópia, então não há como saber se algum repete o mesmo trecho. Os casos mais prováveis são o F27 (limite do cutoff, exemplo do guia) e o F22/F26 (taxa de amostragem não repassada, mesmo tipo do odin2 8cd9fa4).
- Nada foi commitado nem enviado; os arquivos aguardam revisão.
