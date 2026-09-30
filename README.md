# Compiladores

## PROJETO – Fase 1: Análise Léxica e Análise Sintática

## Integrantes

* Nome: Carolina Lee — RA: 10440304
* Nome: Enrique Cipolla — RA: 10427834
* Nome: Pedro Henrique Saraiva Arruda — RA: 10437747

### Objetivo

A proposta desta atividade é a implementação das duas primeiras etapas do *front-end* de um compilador para a linguagem **MiniVisualg**: a análise léxica e a análise sintática.

O analisador léxico é responsável por identificar os *tokens* presentes no código-fonte, enquanto o analisador sintático utiliza esses *tokens* para verificar se o programa segue a gramática definida para a linguagem.

As duas etapas funcionam de forma integrada. O analisador sintático solicita os próximos *tokens* por meio da função `nextToken()`, que realiza chamadas ao analisador léxico através da função `obterToken()`.

---

### ETAPA 1 - Gramática Livre de Contexto (MiniVisualg)

A gramática da linguagem foi desenvolvida na primeira etapa do projeto a partir das definições fornecidas no enunciado.

Foram consideradas as principais construções da linguagem:

* **1.1** Palavras reservadas
* **1.2** Especificação de *tokens*
* **1.3** Estrutura do programa
* **1.4** Declaração
* **1.5** Comandos
* **1.6** Expressões

A implementação das Etapas 2 e 3 utiliza essa gramática como base para o reconhecimento dos programas MiniVisualg.

---

# ETAPA 2 - Analisador Léxico

## Como compilar

O projeto foi desenvolvido em C e pode ser compilado utilizando o GCC/MinGW com o mesmo comando especificado no enunciado:

```bash
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
```

As principais opções utilizadas são:

* `-Wall`: habilita os warnings do compilador;
* `-Wno-unused-result`: desabilita o warning relacionado a resultados de funções não utilizados;
* `-g`: adiciona informações para depuração;
* `-Og`: utiliza otimização adequada para execução com debug;
* `-o compilador`: define o nome do executável.

O comando acima também é importante para testar o projeto nas mesmas condições indicadas para a avaliação. O enunciado especifica esse comando e informa que o projeto será testado utilizando MinGW com VSCode.

## Como executar

Depois de compilar, o programa recebe o arquivo-fonte MiniVisualg como argumento:

```bash
./compilador arquivo.txt
```

O nome do arquivo deve ser informado pela linha de comando, conforme especificado no enunciado.

A análise é realizada sobre o arquivo fornecido e os *tokens* reconhecidos são apresentados no terminal e também armazenados em um arquivo de saída `.lex`.

---

## Funcionamento

O analisador léxico percorre o código-fonte e identifica os elementos que formam o programa, produzindo os respectivos *tokens*.

Entre os elementos reconhecidos estão:

* identificadores;
* palavras reservadas;
* números inteiros;
* números reais;
* cadeias de caracteres;
* operadores relacionais;
* operador de atribuição;
* operadores aritméticos;
* delimitadores;
* comentários iniciados por `//`.

O resultado segue o formato especificado no enunciado:

```text
NúmeroDaLinha# NomeToken | Atributo
```

Por exemplo:

```text
11# IDENTIFICADOR | 1
```

Cada token é armazenado em uma estrutura própria contendo as informações necessárias para a análise.

Além de gerar o arquivo de saída, os mesmos tokens são apresentados na tela.

Quando ocorre um erro léxico, o programa informa a mensagem de erro, a linha em que o problema foi encontrado e a sequência considerada inválida. Após o erro, o processamento é encerrado, conforme solicitado pelo enunciado.

Exemplo:

```text
ERRO LEXICO na linha X: "sequência"
```

---

## Decisões de implementação

Durante a implementação foi necessário lidar com algumas diferenças entre as categorias de elementos exigidas pela gramática e os tipos originalmente disponíveis na estrutura de `Token`.

A decisão adotada foi **reaproveitar os campos existentes sempre que possível**, evitando alterações maiores na estrutura fornecida.

| Elemento                               | Representação utilizada         | Justificativa                                                             |
| -------------------------------------- | ------------------------------- | ------------------------------------------------------------------------- |
| Cadeias de caracteres                  | `TOKEN_ID`                      | Reaproveita o mecanismo de armazenamento utilizado para identificadores   |
| `<-`                                   | `TOKEN_OP_REL`  e  `OP_ASSIGN`  | Utiliza o campo `op_code` com um código específico para atribuição        |
| `<>`                                   | `TOKEN_OP_REL`  e  `OP_NE`      | Reaproveita a estrutura dos operadores relacionais                        |
| Operadores aritméticos e delimitadores | `TOKEN_OP_REL`                  | O próprio caractere é armazenado no campo `op_code`                       |
| Palavras reservadas                    | `TOKEN_KEYWORD` + `lexemaAtual` | Permite que o sintático identifique qual palavra reservada foi encontrada |

Essa abordagem não é necessariamente a modelagem mais convencional para um analisador léxico. A escolha foi feita principalmente para manter a estrutura principal fornecida na especificação e evitar alterações desnecessárias no `struct Token` e nos tipos já existentes.

### `lexemaAtual`

Foi criada a variável `lexemaAtual` para armazenar o texto da última palavra reservada reconhecida.

O tipo `TOKEN_KEYWORD` indica apenas que o token é uma palavra reservada, mas o analisador sintático precisa saber qual palavra foi encontrada. Por exemplo, `se`, `enquanto`, `para` e `fimalgoritmo` possuem comportamentos diferentes na gramática.

Assim, o sintático consegue consultar o lexema atual para decidir qual regra deve ser aplicada.

---

# ETAPA 3 - Analisador Sintático

## Funcionamento

O analisador sintático utiliza os *tokens* produzidos pelo analisador léxico para verificar se a sequência encontrada corresponde à gramática definida na Etapa 1.

A implementação utiliza **análise descendente**, conforme solicitado no enunciado.

As principais construções tratadas são:

* atribuições, como `idade <- 18`;
* declarações de variáveis;
* chamadas de funções e procedimentos;
* `leia`;
* `escreva` e `escreval`;
* estruturas condicionais:

  * `se`;
  * `entao`;
  * `senao`;
  * `fimse`;
* estruturas de repetição:

  * `enquanto`;
  * `faca`;
  * `fimenquanto`;
  * `para`;
  * `fimpara`.

A integração entre os analisadores ocorre por meio de `nextToken()` e `obterToken()`. O analisador sintático solicita um novo token quando necessário, e o analisador léxico é responsável por reconhecê-lo. Essa é também a forma de interação definida no enunciado.

O fluxo pode ser resumido da seguinte forma:

```text
              Arquivo-fonte
                    |
                    v
          +-------------------+
          | Analisador Léxico |
          |    obterToken()   |
          +-------------------+
                    |
                  token
                    |
                    v
         +--------------------+
         | Analisador Sintático|
         |    nextToken()     |
         +--------------------+
                    |
                    v
          Programa aceito
             ou erro
```

---

## Tratamento de erros

Quando a sequência de *tokens* não corresponde à gramática esperada, o analisador sintático informa um erro contendo a indicação de erro sintático, o token incorreto e a linha correspondente.

Exemplo:

```text
ERRO SINTATICO na linha X: mensagem
```

Após a identificação do erro, o processamento é encerrado, conforme especificado no projeto.

---

## Limitações e decisões não ideais

Algumas decisões de implementação foram tomadas para adaptar a estrutura inicial às necessidades da gramática.

A principal delas foi o reaproveitamento de `TOKEN_OP_REL` para representar operadores e delimitadores que não possuíam uma categoria específica na estrutura original. Embora não seja a representação mais semântica possível, ela permite que o analisador sintático diferencie os elementos utilizando o valor armazenado em `op_code`.

Da mesma forma, cadeias de caracteres utilizam a estrutura disponível para identificadores. Essa escolha reduz alterações na estrutura original, mas faz com que categorias semanticamente diferentes compartilhem o mesmo tipo de token.

Essas decisões foram adotadas como uma forma de manter compatibilidade com a estrutura definida inicialmente, priorizando o funcionamento conjunto das Etapas 2 e 3.

---

## Bugs e limitações conhecidas

Na versão atual, foram identificados alguns pontos que ainda precisam de verificação ou podem ser aprimorados:

1. O tratamento do intervalo de vetor (`..`), utilizado em declarações como:

```text
nomes: vetor[1..3] de caractere
```

deve ser validado nos testes do analisador léxico.

2. A validação de alguns formatos de números reais pode aceitar casos que deveriam ser considerados inválidos.

3. Existem situações específicas de tokens inesperados dentro de `comando()` que podem não gerar o erro sintático esperado.

Esses pontos devem ser considerados na avaliação dos testes da versão entregue.

---

## Testes

Os testes devem considerar as principais construções apresentadas nos exemplos do enunciado, incluindo:

* declaração de variáveis;
* atribuição;
* entrada e saída;
* números inteiros e reais;
* operadores aritméticos;
* operadores relacionais;
* operadores lógicos;
* estruturas condicionais;
* estruturas de repetição;
* vetores;
* procedimentos;
* funções e retorno.

Entre os exemplos fornecidos no enunciado estão `BoasVindas`, `CadastroSimples`, `TextoELogico`, `TesteDeLogica`, `CalculadoraBasica`, `ComparaTextosENumeros`, `ParqueDeDiversoes`, `VerificaChuva`, `ContagemSimples`, `ContagemRegressiva`, `ContadorManual`, `ListaNomes`, `MediaNotas`, exemplos de procedimentos e exemplos de funções.
A tabela abaixo pode ser preenchida com o resultado dos testes efetivamente realizados:

| Teste                       | Etapa            | Resultado   |
| -------------------         | ---------------- | ----------- |
| `RotinasComRetorno`         | Léxico/Sintático | Funciona    |
| `nome_do_algoritmo`         | Léxico/Sintático | Funciona    |
| `CalculadoraBasica`         | Léxico/Sintático | Funciona    |
| `ProcedimentosSemParametros`| Léxico/Sintático | Funciona    |
| `ListaNomes`                | Léxico/Sintático | Funciona    |
| `ContagemSimples`           | Léxico/Sintático | Funciona    |
| `ParqueDeDiversoes`         | Léxico/Sintático | Funciona    |

---

## Conclusão

Nesta fase foram implementados os analisadores léxico e sintático da linguagem MiniVisualg, com funcionamento integrado entre as duas etapas.

O analisador léxico realiza o reconhecimento dos *tokens* e a geração da saída correspondente, enquanto o analisador sintático utiliza esses tokens para verificar a estrutura dos programas de acordo com a gramática definida.

As decisões de implementação que diferem de uma representação mais convencional foram documentadas neste README, principalmente aquelas relacionadas ao reaproveitamento da estrutura de `Token`.
