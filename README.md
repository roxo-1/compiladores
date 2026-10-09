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
algoritmo, var, inicio, fimalgoritmo,
caractere, inteiro, real, logico,
verdadeiro, falso,
leia, escreva, escreval
se, entao, senao, fimse
para, de, ate, passo, faca, fimpara,
enquanto, fimenquanto,
vetor,
procedimento, fimprocedimento,
funcao, fimfuncao, retorne
MOD, E, OU

* **1.2** Estrutura geral do algoritmo
algoritmo       ::= "algoritmo" nomeAlgoritmo declaracoesRotinas
                    "var" declaracaoVariaveis declaracoesRotinas
                    "inicio" codigo "fimalgoritmo"

nomeAlgoritmo   ::= TOKEN_ID

declaracoesRotinas ::= (procedimento | funcao)*

declaracaoVariaveis ::= (idLista ":" tipo)+

idLista         ::= id ("," id)*

codigo          ::= comando*

* **1.3** Tipos e declarações
tipo ::= tipoBase
       | "vetor" "[" limite ".." limite "]" "de" tipoBase

tipoBase ::= "inteiro"
           | "real"
           | "caractere"
           | "logico"

limite ::= TOKEN_NUM_INT
         | TOKEN_NUM_FLOAT
         | id

* **1.4** Expressões e operadores
  expressao ::= operando (operacoes operando)*

operando ::= "-" operando
           | "(" expressao ")"
           | id chamadaOpcional
           | TOKEN_NUM_INT
           | TOKEN_NUM_FLOAT
           | TOKEN_BOOLEANO

chamadaOpcional ::= chamadaResto
                  | ε

chamadaResto ::= "(" argumentosOpcional ")"

argumentosOpcional ::= expressao ("," expressao)*
                     | ε

operacoes ::= operacaoAritmetica
            | operacaoRelacional
            | operacaoLogica
            | "MOD"

operacaoAritmetica ::= "+" | "-" | "*" | "/"

operacaoRelacional ::= "<" | "<=" | "==" | ">" | ">=" | "<>"

operacaoLogica ::= "E" | "OU"

* **1.5** Comandos
 comando ::= leitura
          | escrita
          | condicional
          | repeticaoPara
          | repeticaoEnquanto
          | retorno
          | atribuicao
          | chamadaProcedimento

variavel ::= id ("[" expressao "]")?

atribuicao ::= variavel "<-" expressao

leitura ::= "leia" "(" id ")"

escrita ::= ("escreva" | "escreval")
            "(" expressao ("," expressao)* ")"

condicional ::= "se" "(" condicaoSe ")" "entao"
                codigo
                ("senao" codigo)?
                "fimse"

condicaoSe ::= operandoSimples
               (operacaoRelacional | "MOD")
               operandoSimples

operandoSimples ::= id | TOKEN_NUM_INT | TOKEN_NUM_FLOAT

* **1.6** Estruturas de repetição e retorno
repeticaoPara ::= "para" id "de" TOKEN_NUM_INT
                  "ate" TOKEN_NUM_INT
                  ("passo" TOKEN_NUM_INT)?
                  "faca" codigo "fimpara"

repeticaoEnquanto ::= "enquanto" "(" condicaoEnquanto ")"
                      ("faca")?
                      codigo "fimenquanto"

condicaoEnquanto ::= id operacaoRelacional TOKEN_NUM_INT

retorno ::= "retorne" (TOKEN_BOOLEANO | id operacaoAritmetica id)

* **1.7** Procedimentos e funções
procedimento ::= "procedimento" id
                 ("(" parametros ")")?
                 ("inicio")?
                 codigo
                 "fimprocedimento"

funcao ::= "funcao" id "(" parametros ")"
           ":" tipo
           "inicio" codigo
           "fimfuncao"

parametros ::= parametro ("," parametro)*

parametro ::= id ":" tipo

* **1.8** Chamadas e identificadores:
chamadaProcedimento ::= id
                      | id "(" argumentosOpcional ")"

id ::= TOKEN_ID

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
As  variáveis globais que foram adicionadas ao código e o que elas fazem:
* lexemaAtual armazena a cadeia de caracteres até o último token, permitindo que o analisador sintático consulte o código original sem desvios.
* arquivoSaida é apenas o ponteiro usado para gerar o arquivo de saída
* tokenAtual é a variável que armazena o token retornado pela última chamda da função obterToken().
* TOKEN_OP_ARIT para operações aritméticas,
* TOKEN_OP_LOG para operações lógicas,
* TOKEN_OP_ATRIBUTION para atribuição,
* TOKEN_DELIMITADORES para delimitadores,
* TOKEN_COMENTARIO para comentário,
* TOKEN_BOOLEANO para booleano.
* Os tokens extras foram adicionados para a aplicação correta da gramática
  
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

4. Contador de palavras reservadas não funciona como deveria, conta a quantidade de palavras reservadas no arquivo txt

Esses pontos devem ser considerados na avaliação dos testes da versão entregue.

---
## Como executar o código

* Como compilar:
´´´
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
´´´

* Como rodar, passando o arquivo fonte MiniVisualg como argumento:
´´´´
./compilador <nome_do_arquivo>.txt
´´´
---
## Testes
Os testes realizados foram os anexos fornecidos pela professora:
* nome_do_algoritmo, que valida a estrutura do algoritmo, declaração de variável, comentários e espaços em branco ignorados, identificação de id's e manipulação da tabela de simbolos, reconhecimento de palavras reservadas.
* CalculadoraBasica, que valida atribuição e operações aritméticas


---
## Conclusão

Nesta fase foram implementados os analisadores léxico e sintático da linguagem MiniVisualg, com funcionamento integrado entre as duas etapas.

O analisador léxico realiza o reconhecimento dos *tokens* e a geração da saída correspondente, enquanto o analisador sintático utiliza esses tokens para verificar a estrutura dos programas de acordo com a gramática definida.

As decisões de implementação que diferem de uma representação mais convencional foram documentadas neste README, principalmente aquelas relacionadas ao reaproveitamento da estrutura de `Token`.
