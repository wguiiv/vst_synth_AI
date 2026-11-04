# Comparação de frameworks: GoogleTest, Catch2 e doctest

Análise secundária do plano ("Comparação de frameworks"). O experimento usa só o GoogleTest; aqui, um teste
representativo, "toda nota tem a altura certa" (`SpectralPitch` em `tests/spectral/SpectralTest.cpp`), foi
portado para os três frameworks, para comparar como cada um expressa as necessidades de um teste de áudio.

Versões: GoogleTest 1.15.2, Catch2 3.7.1, doctest 2.4.11. As afirmações sobre o comportamento de cada framework
foram conferidas no código-fonte dessas versões (os arquivos estão indicados), não de memória.

## O teste portado

As três versões fazem o mesmo: para cada taxa de amostragem (44,1, 48 e 96 kHz) e cada forma de onda (seno,
dente de serra, quadrada), tocam 9 notas e verificam se a fundamental medida está a no máximo 5 cents da nota
certa (`tol::pitch`). São 81 verificações. A parte que não depende do framework (renderizar a nota e medir a
fundamental) fica em `PitchCommon.h` e reutiliza `tests/common/Render.cpp` e `Analysis.cpp` sem alteração; os
testes congelados em `tests/` não foram tocados.

| Arquivo | Linhas de código |
| --- | --- |
| `PitchCommon.h` (comum aos três) | 33 |
| `gtest_pitch.cpp` | 25 |
| `catch2_pitch.cpp` | 24 |
| `doctest_pitch.cpp` | 23 (mais 4 da demonstração do `Approx`, abaixo) |

Linhas sem brancos e comentários, contadas como em `analyze.py`.

### Equivalência

No código limpo, as três versões passam nas 81 verificações e medem os mesmos valores que o teste original
gravou na referência (`experiment/results/raw/baseline/run1`). A maior diferença relativa entre um valor das
versões e o original é 4,2 × 10⁻⁶, e vem só do número de casas com que cada framework imprime o valor
(o doctest imprime 6 algarismos significativos). Ou seja, a comparação abaixo é do framework, não da medição.

## Como cada framework expressa o teste

GoogleTest: um teste parametrizado por valor; cada combinação vira um caso de teste separado.

```cpp
class Pitch : public ::testing::TestWithParam<std::tuple<double, int>> {};

TEST_P (Pitch, EveryNoteHasTheRightFundamental)
{
    const auto [sampleRate, wave] = GetParam();
    for (const auto note : pitch::notes)
    {
        SCOPED_TRACE ("note " + std::to_string (note));
        const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
        RecordProperty ("spectral.pitch|note" + std::to_string (note), std::to_string (c));
        EXPECT_NEAR (c, 0.0, pitch::toleranceCents);
    }
}

INSTANTIATE_TEST_SUITE_P (RatesAndWaves, Pitch,
    ::testing::Combine (::testing::ValuesIn (pitch::sampleRates), ::testing::ValuesIn (pitch::waves)));
```

Catch2: `GENERATE` executa o mesmo caso de teste uma vez por combinação (produto cartesiano dos geradores).

```cpp
TEST_CASE ("Every note has the right fundamental", "[spectral][pitch]")
{
    const auto sampleRate = GENERATE (from_range (std::begin (pitch::sampleRates), std::end (pitch::sampleRates)));
    const auto wave = GENERATE (from_range (std::begin (pitch::waves), std::end (pitch::waves)));
    for (const auto note : pitch::notes)
    {
        CAPTURE (sampleRate, wave, note);
        const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
        CHECK_THAT (c, WithinAbs (0.0, pitch::toleranceCents));
    }
}
```

doctest: não há testes parametrizados por valor, então as combinações são laços.

```cpp
TEST_CASE ("Every note has the right fundamental")
{
    for (const auto sampleRate : pitch::sampleRates)
        for (const auto wave : pitch::waves)
            for (const auto note : pitch::notes)
            {
                CAPTURE (sampleRate);
                CAPTURE (wave);
                CAPTURE (note);
                const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
                CHECK (std::abs (c) <= pitch::toleranceCents);
            }
}
```

Os três precisam de uma função `main` própria, porque o processador do JUCE exige o
`juce::ScopedJuceInitialiser_GUI` antes dos testes; ela tem 2 linhas no Catch2 e 3 no GoogleTest e no doctest.

## Comparação de recursos

| Necessidade do teste de áudio | GoogleTest | Catch2 | doctest |
| --- | --- | --- | --- |
| Comparação aproximada | `EXPECT_NEAR` (absoluta); `EXPECT_FLOAT_EQ` / `EXPECT_DOUBLE_EQ` (até 4 ULPs) | `WithinAbs` (absoluta), `WithinRel` (relativa; padrão 100 × épsilon de `double`), `WithinULP`, e `Approx` | `Approx`: margem ε × (escala + max(\|a\|, \|b\|)), com ε = 100 × épsilon de `float` e escala 1 por padrão |
| Mesmo teste para vários valores | `TEST_P` + `INSTANTIATE_TEST_SUITE_P` (com `Combine`) | `GENERATE` | Laços, ou `SUBCASE`; `TEST_CASE_TEMPLATE` só varia tipos |
| Verificações de áudio próprias | `AssertionResult` (usado no estudo em `within()`) ou `MATCHER` do gMock | Classes de matcher | Funções auxiliares |
| Contexto quando falha | `SCOPED_TRACE` | `INFO`, `CAPTURE` | `INFO`, `CAPTURE` |
| Gravar o valor medido no relatório | `RecordProperty`, que vira `<property>` no XML | Não há; o relatório JUnit só grava as propriedades `random-seed` e `filters` | Não há; o relatório JUnit não tem propriedades |
| Relatório para a CI | XML (`--gtest_output`): 9 casos de teste, um por combinação, e as 81 medições | JUnit: 1 caso de teste | JUnit: 1 caso de teste |
| Forma de distribuição | Biblioteca compilada | Biblioteca compilada (v3) | Um único cabeçalho |

Fontes no código: `googletest/include/gtest/internal/gtest-internal.h` (`kMaxUlps = 4`);
`catch2/matchers/catch_matchers_floating_point.cpp` (`WithinRel` e sua margem);
`catch2/reporters/catch_reporter_junit.cpp` (propriedades gravadas); `doctest/doctest.h` (`Approx`, `JUnitReporter`).

## Medidas

Um build e três execuções por framework, no servidor do estudo (2 núcleos, prioridade mínima), por isso os tempos
de compilação são indicativos.

| | GoogleTest | Catch2 | doctest |
| --- | --- | --- | --- |
| Compilação do arquivo de teste | 4,9 s | 4,2 s | 7,8 s |
| Resto do executável (auxiliares e link) | 77,0 s | 75,5 s | 81,6 s |
| Tamanho do executável | 6,5 MB | 7,1 MB | 6,1 MB |
| Execução das 81 verificações | 0,86–1,24 s | 0,88–1,82 s | 0,90–1,02 s |

O custo é dominado pelo JUCE: o link com otimização em tempo de link leva mais de um minuto em qualquer framework,
e a execução é quase toda a renderização do áudio. A diferença entre os frameworks é pequena perto disso.

## O que importa para testes de áudio

1. **Gravar o valor medido.** A varredura de tolerâncias do estudo (QP3) foi calculada sem rodar os testes de
   novo, a partir dos valores gravados com `RecordProperty`. Só o GoogleTest tem esse recurso. No Catch2 e no
   doctest, os valores só aparecem pedindo o relatório de todas as verificações que passaram (`-s` com o
   relatório XML de cada framework), como texto da expressão expandida (por exemplo
   `-0.03428626749760934 is within 5.0 of 0.0`), que precisa ser interpretado; ou o teste teria de gravar um arquivo
   próprio. Foi assim que a equivalência acima foi conferida.

2. **O `Approx` do doctest não serve para tolerância absoluta perto de zero.** A margem
   ε × (escala + max(|a|, |b|)) cresce com o próprio valor medido. Escrever "erro de altura igual a 0, com 5 cents
   de folga" como `Approx (0.0).epsilon (5.0)` dá uma margem de 5 × (1 + |erro|), maior que qualquer erro: a
   verificação nunca falha. A demonstração em `doctest_pitch.cpp` (suíte `demonstration`) passa com um erro de
   1000 cents. Por isso a versão doctest usa uma comparação simples. Longe de zero, o `Approx` funciona como
   tolerância relativa: com ε = 0,005, a margem é 0,56 Hz em 110 Hz e 8,8 Hz em 1760 Hz, enquanto uma tolerância
   absoluta de 2 Hz vale o mesmo em toda a faixa.

3. **Relativa ou absoluta depende da grandeza.** Altura em cents é uma razão: 5 cents correspondem a uma
   diferença relativa de 0,289% na frequência, em qualquer nota. Por isso o teste mede o erro em cents e compara
   com uma tolerância absoluta em cents; o mesmo critério em Hz seria um `WithinRel` do Catch2 com 0,00289.
   Já tempos de envelope (ms) e diferenças de amostra (golden master) são grandezas em que a tolerância absoluta é
   a natural. Um framework que oferece as duas formas explicitamente (Catch2) deixa essa escolha mais visível.

4. **Granularidade do relatório.** No GoogleTest, cada combinação de taxa e forma de onda é um caso de teste no
   relatório da CI, então uma falha mostra na hora qual combinação quebrou. No Catch2 e no doctest, as 9
   combinações são um único caso; a combinação aparece só na mensagem de falha (via `CAPTURE`).

5. **Para o resto, os três servem.** Todos têm contexto na falha, relatório JUnit para a CI e precisam do mesmo
   cuidado com a inicialização do JUCE. O doctest é o mais simples de incluir (um cabeçalho); o Catch2 tem o
   conjunto mais completo de comparações aproximadas; o GoogleTest é o único que guarda medições no relatório.

## Como reproduzir

```bash
cmake -B build-frameworks -G Ninja -DCMAKE_BUILD_TYPE=Release -DTAPSYNTH_BUILD_FRAMEWORK_COMPARISON=ON
cmake --build build-frameworks --target pitch_gtest pitch_catch2 pitch_doctest
build-frameworks/experiment/frameworks/pitch_gtest
build-frameworks/experiment/frameworks/pitch_catch2
build-frameworks/experiment/frameworks/pitch_doctest --test-suite-exclude=demonstration
build-frameworks/experiment/frameworks/pitch_doctest --test-suite=demonstration     # o Approx que nunca falha
```

A opção `TAPSYNTH_BUILD_FRAMEWORK_COMPARISON` fica desligada por padrão e não muda o build do plugin nem dos
testes do estudo.
