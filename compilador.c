#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEMA 128
char lexemaAtual[MAX_LEXEMA];
// #define OP_ASSIGN 5
// #define OP_NE     6
FILE *arquivoSaida = NULL;

// esses dois enum tiveram que vir para cima do struct Token, porque o C
// não deixa usar um tipo antes dele existir (dava erro de compilação
// "unknown type name" quando eu deixei embaixo)

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ID,
    TOKEN_NUM_INT,
    TOKEN_NUM_FLOAT,
    TOKEN_OP_REL,
    TOKEN_OP_ARIT,
    TOKEN_OP_LOG,
    TOKEN_OP_ATRIBUTION,
    TOKEN_KEYWORD,
    TOKEN_DELIMITADORES,
    TOKEN_COMENTARIO,
    TOKEN_BOOLEANO,
} TokenNome;


typedef enum {
    TRUE, // verdadeiro
    FALSO, // falso
} Booleano;

typedef enum {
    OP_LT, // <
    OP_LE, // <=
    OP_EQ, // ==
    OP_GT, // >
    OP_GE, // >= 
    OP_DIF, // <>
} OpRelType;

typedef enum {
    OP_SUM, // +
    OP_MINUS, // -
    OP_DIV, // /
    OP_MULT, // * 
    OP_DIV_INT, // divisao inteira (barra invertida), usada no teste CalculadoraBasica
} OpAritType;


typedef enum {
    OP_E, // E
    OP_OU, // OU
} OpLogType;


typedef enum {
    OP_AT, // E
} OpAtribuition;

typedef enum {
    comentario, // E
} Comentario;

typedef enum {
    abreParenteses,
    fechaParenteses,
    abreColchetes,
    fechaColchetes,
    aspas,
    virgula,
    doisPontos,
    pontoVirgula, // ;   (lista de Delimitador do relatorio)
    pontoPonto,   // ..  (limites do vetor)
} Delimitadores;

typedef struct {
    TokenNome type; //Nome do token
    int line; // Para tratamento de erros

    union{
        int table_index; // Índice para Tabela de Símbolos
        int int_value; //Valor literal convertido
        double float_value; //Valor literal convertido
        OpRelType op_rel; //operador relacional específico
        OpAritType op_arit;//operador aritmetico
        OpLogType op_log; // operador logico
        OpAtribuition op_at; // atribuicao
        Delimitadores delimitadores;
        Comentario comentario;
        Booleano booleano;
    } attribute;

} Token;

void erroSintatico(const char *msg); 
void nextToken(); 
void expressao(); 
void expressaoRelacional(); 
void expressaoAritmetica(); 
void termo(); 
void fator(); 
void variavel(); 
void atribuicao();
void leitura(); 
void escrita(); 
void condicional(); 
void repeticaoPara(); 
void repeticaoEnquanto(); 
void chamada(); 
void retorno(); 
void comando(); 
void tipoBase(); 
void tipo(); 
void idLista(); 
void declaracaoLista(); 
void parametro(); 
void parametros(); 
void declaracaoVar(); 
void declaracaoProcedimento(); 
void declaracaoFuncao(); 
void algoritmo(); 
void declaracao();
void codigo();
// prototipos das funcoes novas
void operando();
int ehDelimitador(Delimitadores d);
void operandoSimples();
void condicaoSe();
void condicaoEnquanto();
void chamadaResto();
void declaracoesRotinas();
void imprimirTabelaSimbolos(FILE *saida);


/*
 * TODO LIST - ETAPA 2: ANALISADOR LÉXICO (MINIVISUALG)

 * 1. GERENCIAMENTO DE ARQUIVO E ESTADO GLOBAL
 * [X] Configurar o ponteiro FILE para leitura do código fonte.
 * [X] Criar a variável global de controle de linha atual (inicializada em 1).
 * [X] Implementar as funções de ciclo de vida: abrir arquivo, checar EOF (fim de arquivo) e fechar arquivo.
*/
FILE *fonte = NULL; // Ponteiro para o arquivo de entrada
int linhaAtual = 1; // Contador de linha atual
int pontoPendente = 0; // 1 se um '.' ja foi lido do arquivo mas ainda nao virou token

void iniciarAnalisador(FILE *arquivo) {
    fonte = arquivo;
    linhaAtual = 1;
    pontoPendente = 0;
}

int fimDoArquivo(){
    return(fonte == NULL || feof(fonte)); // retorna true se o arquivo for nulo ou se chegou no final do arquivo, representado pelo feof()
}

void fecharAnalisador(){ 
    if(fonte != NULL) {
        fclose(fonte);
        fonte = NULL;
    }
}

// preciso avisar pro compilador que essas 3 funções aqui embaixo existem
// (mesmo elas sendo definidas só lá na frente, nos itens 4 e 6), porque a
// proximoToken() já usa elas antes disso. se eu não avisar, o compilador
// dá erro de "declarado implicitamente" quando ele finalmente ve a
// função de verdade com um tipo diferente do que ele tinha "chutado"
TokenNome classificarPalavra(const char *lexema);
int inserirTabelaSimbolos(const char *lexema);
void erroLexico(const char *sequencia);

/*
 *
 * 2. LIMPEZA DE ENTRADA (ESPAÇOS E COMENTÁRIOS)
 * [X] Implementar a lógica de leitura avançando caractere por caractere.
 * [X] Descartar espaços em branco, tabulações e quebras de linha (incrementando o contador de linha ao detectar '\n').
 * [X] Descartar comentários: ao identificar '//', ignorar todos os caracteres seguintes até encontrar uma quebra de linha.
 */

int peek() { // A função peek espia o próximo caractere sem consumi-lo do buffer
    int c = fgetc(fonte); // tal que c representa um charactere lido do arquivo fonte e o fgetc() lê o próximo caractere do arquivo fonte e retorna seu valor como um inteiro
    if (c != EOF) { 
        ungetc(c, fonte); // ele devolve o caractere lido para o fluxo
    }
    return c;
}

// le o proximo caractere do arquivo. Se sobrou um '.' lido antes, devolve ele primeiro.
int lerCaractere() {
    if (pontoPendente) {
        pontoPendente = 0;
        return '.';
    }
    return fgetc(fonte);
}

//Limpeza de Entrada
Token proximoToken() {
    Token token;
    int c;

    while ((c = lerCaractere()) != EOF) {
        if (c == '\n') {
            linhaAtual++;
            continue;
        }

        if (isspace(c)) {
            continue;
        }

        // Ignora comentários no estilo //
        if (c == '/' && peek() == '/') {
            while ((c = fgetc(fonte)) != EOF && c != '\n');
            if (c == '\n') {
                linhaAtual++;
            }
            continue;
        }
        break;
    }

    if (c == EOF) {
        token.type = TOKEN_EOF;
        token.line = linhaAtual;
        return token;
    }

    token.line = linhaAtual;

    // Leitura de Palavras Reservadas e Identificadores
    // id comeca com letra: [a-zA-Z][a-zA-Z0-9_]*
    if (isalpha(c)) {
        char lexema[MAX_LEXEMA];
        int i = 0;
        lexema[i++] = (char)c;

        while (i < MAX_LEXEMA - 1 && (isalnum(peek()) || peek() == '_')) {
            lexema[i++] = (char)fgetc(fonte);
        }
        lexema[i] = '\0';

        // Guarda o texto lido na variável global sem alterar o struct Token
        strcpy(lexemaAtual, lexema);

        token.type = classificarPalavra(lexema);
        if (token.type == TOKEN_ID) {
            token.attribute.table_index = inserirTabelaSimbolos(lexema);
        }
        // guarda qual operador logico / qual booleano foi lido
        else if (token.type == TOKEN_OP_LOG) {
            token.attribute.op_log = (strcmp(lexema, "E") == 0) ? OP_E : OP_OU;
        } else if (token.type == TOKEN_BOOLEANO) {
            token.attribute.booleano = (strcmp(lexema, "verdadeiro") == 0) ? TRUE : FALSO;
        }
        return token;
    }

    // Leitura de Números (Inteiros e Reais)
    if (isdigit(c)) {
        char lexema[MAX_LEXEMA];
        int i = 0;
        int ehReal = 0;
        lexema[i++] = (char)c;

        while (i < MAX_LEXEMA - 1 && isdigit(peek())) {
            lexema[i++] = (char)fgetc(fonte);
        }

        // numero real: [0-9]+\.[0-9]+  (precisa de digito depois do ponto)
        if (peek() == '.') {
            fgetc(fonte); // le o '.'
            if (isdigit(peek())) {
                ehReal = 1;
                lexema[i++] = '.';
                while (i < MAX_LEXEMA - 1 && isdigit(peek())) {
                    lexema[i++] = (char)fgetc(fonte);
                }
            } else {
                // nao e real (ex: 1..3 no vetor). O '.' ja foi lido, entao
                // guardo ele para ser o inicio do proximo token.
                pontoPendente = 1;
            }
        }
        lexema[i] = '\0';

        if (ehReal) {
            token.type = TOKEN_NUM_FLOAT;
            token.attribute.float_value = atof(lexema);
        } else {
            token.type = TOKEN_NUM_INT;
            token.attribute.int_value = atoi(lexema);
        }
        return token;
    }

    // Leitura de Strings entre Aspas "..."
    if (c == '"') {
        char lexema[MAX_LEXEMA];
        int i = 0;

        while ((c = fgetc(fonte)) != EOF && c != '"' && c != '\n') {
            if (i < MAX_LEXEMA - 1) {
                lexema[i++] = (char)c;
            }
        }

        if (c != '"') {
            erroLexico("Cadeia de texto nao fechada");
        }

        lexema[i] = '\0';
        token.type = TOKEN_ID;
        token.attribute.table_index = inserirTabelaSimbolos(lexema);
        return token;
    }

    // Operador <- (Atribuição), <=, <>, <
    if (c == '<') {
        if (peek() == '-') {
            fgetc(fonte);
            // <- e o operador de atribuicao
            token.type = TOKEN_OP_ATRIBUTION;
            token.attribute.op_at = OP_AT;
            return token;
        }
        if (peek() == '=') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_rel = OP_LE;
            return token;
        }
        if (peek() == '>') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_rel = OP_DIF;
            return token;
        }
        token.type = TOKEN_OP_REL;
        token.attribute.op_rel= OP_LT;
        return token;
    }

    // Operadores >= e >
    if (c == '>') {
        if (peek() == '=') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_rel = OP_GE;
            return token;
        }
        token.type = TOKEN_OP_REL;
        token.attribute.op_rel = OP_GT;
        return token;
    }

    // Operador == (relatorio). O '=' simples tambem e aceito como igualdade.
    if (c == '=') {
        if (peek() == '=') {
            fgetc(fonte);
        }
        token.type = TOKEN_OP_REL;
        token.attribute.op_rel = OP_EQ;
        return token;
    }

    
    // delimitador '..' (limites do vetor: vetor[1..3])
    if (c == '.' && peek() == '.') {
        fgetc(fonte);
        token.type = TOKEN_DELIMITADORES;
        token.attribute.delimitadores = pontoPonto;
        return token;
    }

    // Operadores e Delimitadores em geral
    // o atributo guarda qual delimitador foi lido (enum Delimitadores)
    if (c == '(' || c == ')' || c == ',' || c == ':' || 
        c == '[' || c == ']' || c == ';')  {
        token.type = TOKEN_DELIMITADORES;
        switch (c) {
            case '(': token.attribute.delimitadores = abreParenteses;  break;
            case ')': token.attribute.delimitadores = fechaParenteses; break;
            case '[': token.attribute.delimitadores = abreColchetes;   break;
            case ']': token.attribute.delimitadores = fechaColchetes;  break;
            case ',': token.attribute.delimitadores = virgula;         break;
            case ':': token.attribute.delimitadores = doisPontos;      break;
            case ';': token.attribute.delimitadores = pontoVirgula;    break;
        }
        return token;
    }
    // operadores aritmeticos: o tipo e TOKEN_OP_ARIT e o operador fica no atributo
    else if( c == '+'){
        token.type = TOKEN_OP_ARIT;
        token.attribute.op_arit = OP_SUM;
        return token;
    }
    else if(c == '-'){
        token.type = TOKEN_OP_ARIT;
        token.attribute.op_arit = OP_MINUS;
        return token;
    }
    else if( c == '*'){
        token.type = TOKEN_OP_ARIT;
        token.attribute.op_arit = OP_MULT;
        return token;
    }
    else if(c == '/'){
        token.type = TOKEN_OP_ARIT;
        token.attribute.op_arit = OP_DIV;
        return token;
    }
    else if(c == '\\'){   // divisao inteira (aparece no teste CalculadoraBasica)
        token.type = TOKEN_OP_ARIT;
        token.attribute.op_arit = OP_DIV_INT;
        return token;
    }

    // qualquer outro caractere e erro lexico
    char seqInvalida[2] = { (char)c, '\0' };
    erroLexico(seqInvalida);
    return token;
}

/* 4. CLASSIFICAÇÃO E RETORNO DE TOKENS
 * [X] Criar a função que avalia o lexema recém-extraído e define seu tipo.
 * [X] Garantir que Palavras Reservadas da linguagem (algoritmo, var, inicio, se, enquanto, etc.) tenham prioridade de classificação sobre Identificadores comuns.
 * [X] Preencher e retornar a struct Token com o tipo, linha e atributo correspondente.
 */
TokenNome classificarPalavra(const char *lexema) {
    // E, OU, verdadeiro e falso tem tipo proprio de token
    if (strcmp(lexema, "E") == 0 || strcmp(lexema, "OU") == 0) return TOKEN_OP_LOG;
    if (strcmp(lexema, "verdadeiro") == 0 || strcmp(lexema, "falso") == 0) return TOKEN_BOOLEANO;

    // lista com todas as palavras reservadas do relatório da etapa 1
    static const char *reservadas[] = {
        "algoritmo", "var", "inicio", "fimalgoritmo",
        "caractere", "inteiro", "real", "logico",
        "leia", "escreva", "escreval",
        "se", "entao", "senao", "fimse",
        "para", "de", "ate", "passo", "faca", "fimpara",
        "enquanto", "fimenquanto",
        "vetor",
        "procedimento", "fimprocedimento",
        "funcao", "fimfuncao", "retorne",
        "MOD"
    };
    int total = (int)(sizeof(reservadas) / sizeof(reservadas[0]));

    // comparo o lexema com cada palavra da lista. se bater com alguma,
    // é reservada. isso já garante a prioridade que o item pede, porque
    // só cai em TOKEN_ID se não bateu com nenhuma reservada
    for (int i = 0; i < total; i++) {
        if (strcmp(lexema, reservadas[i]) == 0) {
            return TOKEN_KEYWORD;
        }
    }
    return TOKEN_ID;
}

// Tabela de simbolos: guarda cada identificador uma unica vez.
// Se o lexema ja esta na tabela, devolve o indice que ele ja tinha.
#define MAX_SIMBOLOS 1000
char tabelaSimbolos[MAX_SIMBOLOS][MAX_LEXEMA]; // texto de cada simbolo
int linhaSimbolos[MAX_SIMBOLOS];               // linha da primeira ocorrencia
int totalSimbolos = 0;

int inserirTabelaSimbolos(const char *lexema) {
    for (int i = 0; i < totalSimbolos; i++) {
        if (strcmp(tabelaSimbolos[i], lexema) == 0) {
            return i; // ja existe
        }
    }

    if (totalSimbolos == MAX_SIMBOLOS) {
        fprintf(stderr, "Erro: tabela de simbolos cheia\n");
        exit(1);
    }

    strcpy(tabelaSimbolos[totalSimbolos], lexema);
    linhaSimbolos[totalSimbolos] = linhaAtual;
    totalSimbolos++;
    return totalSimbolos - 1;
}

void imprimirTabelaSimbolos(FILE *saida) {
    printf("\n=== TABELA DE SIMBOLOS ===\n");
    if (saida != NULL) fprintf(saida, "\n=== TABELA DE SIMBOLOS ===\n");

    for (int i = 0; i < totalSimbolos; i++) {
        printf("%d | %s | linha %d\n", i, tabelaSimbolos[i], linhaSimbolos[i]);
        if (saida != NULL) {
            fprintf(saida, "%d | %s | linha %d\n", i, tabelaSimbolos[i], linhaSimbolos[i]);
        }
    }
}


/* 5. FORMATAÇÃO E ARQUIVO DE SAÍDA
 * [X] Formatar a string de saída no padrão exigido: Número da Linha do Átomo# NomeToken | Atributo
 * [X] Imprimir cada token no terminal (stdout) à medida que são reconhecidos.
 * [X] Gravar a mesma saída formatada em um arquivo de texto de log.
 */

void registrar_token(Token t, FILE *arquivo_log) {
    const char *nome_tipo;

    switch (t.type) {
        case TOKEN_KEYWORD: nome_tipo = "PALAVRA_RESERVADA"; break;
        case TOKEN_ID:      nome_tipo = "IDENTIFICADOR";     break;
        case TOKEN_NUM_INT: nome_tipo = "NUMERO";            break;
        default:            nome_tipo = "DESCONHECIDO";      break;
    }

    printf("Número da Linha# %d | %s\n", t.line, nome_tipo);

    if (arquivo_log != NULL) {
        fprintf(arquivo_log, "Número da Linha# %d | %s\n", t.line, nome_tipo);
    }
}


/* 6. TRATAMENTO DE ERROS LÉXICOS
 * [X] Interceptar qualquer caractere lido que não pertença ao alfabeto/regras da linguagem MiniVisualg.
 * [X] Exibir a mensagem exata "ERRO LÉXICO", informando a linha e a sequência incorreta (falta acentuar certinho e formatar igual o item 5 pede).
 * [X] Abortar imediatamente a execução do programa (exit) após a identificação do erro.
 */
void erroLexico(const char *sequencia) {
    fprintf(stderr, "ERRO LEXICO na linha %d: \"%s\"\n", linhaAtual, sequencia);
    fecharAnalisador();
    exit(1);
}



/* TODO LIST - ETAPA 3: ANALISADOR SINTATICO (MINIVISUALG)*/

/* 1. INTEGRACAO COM O ANALISADOR LEXICO
[X] Criar a variavel/estrutura que guarda o "token atual" (o lookahead do parser).
[X] Implementar a funcao nextToken() do lado do sintatico, que chama obterToken() do lexico e atualiza o token atual
[X] Antes de comecar a analise, chamar nextToken() uma vez para carregar o primeiro token do arquivo.
*/
//TOKEN ATUAL
Token tokenAtual;
//OBTEM O TOKEN DE FATO
Token obterToken(void) {
    return proximoToken();
}
// codigo impresso para operadores aritmeticos e delimitadores (o mesmo que o relatorio mostra: codigo ASCII)
int codigoOperadorDelimitador(Token t) {
    if (t.type == TOKEN_OP_ARIT) {
        switch (t.attribute.op_arit) {
            case OP_SUM:     return '+';
            case OP_MINUS:   return '-';
            case OP_DIV:     return '/';
            case OP_MULT:    return '*';
            case OP_DIV_INT: return '\\';
        }
    } else {
        switch (t.attribute.delimitadores) {
            case abreParenteses:  return '(';
            case fechaParenteses: return ')';
            case abreColchetes:   return '[';
            case fechaColchetes:  return ']';
            case aspas:           return '"';
            case virgula:         return ',';
            case doisPontos:      return ':';
            case pontoVirgula:    return ';';
            case pontoPonto:      return '.';
        }
    }
    return 0;
}

// imprime o token no formato dos testes do relatorio (linha# NOME | atributo)
void imprimirToken(Token t, FILE *saida) {
    if (t.type == TOKEN_EOF) return;

    char buffer[256];

    switch (t.type) {
        case TOKEN_ID:
            sprintf(buffer, "%d# IDENTIFICADOR | %d", t.line, t.attribute.table_index);
            break;
        case TOKEN_KEYWORD:
            sprintf(buffer, "%d# PALAVRA_RESERVADA | 0", t.line);
            break;
        case TOKEN_NUM_INT:
            sprintf(buffer, "%d# NUM_INTEIRO | %d", t.line, t.attribute.int_value);
            break;
        case TOKEN_NUM_FLOAT:
            sprintf(buffer, "%d# NUM_REAL | %.2f", t.line, t.attribute.float_value);
            break;
        case TOKEN_OP_REL:
            sprintf(buffer, "%d# OPERADOR_DELIMITADOR | %d", t.line, t.attribute.op_rel);
            break;
        case TOKEN_OP_ATRIBUTION:
            sprintf(buffer, "%d# OPERADOR_DELIMITADOR | 5", t.line);
            break;
        case TOKEN_OP_ARIT:
        case TOKEN_DELIMITADORES:
            sprintf(buffer, "%d# OPERADOR_DELIMITADOR | %d", t.line, codigoOperadorDelimitador(t));
            break;
        case TOKEN_OP_LOG:      // E, OU e verdadeiro/falso sao palavras reservadas (secao 1.1)
        case TOKEN_BOOLEANO:
            sprintf(buffer, "%d# PALAVRA_RESERVADA | 0", t.line);
            break;
        default:
            sprintf(buffer, "%d# DESCONHECIDO | 0", t.line);
            break;
    }

    // Imprime no terminal
    printf("%s\n", buffer);

    // Escreve no ficheiro .lex se estiver aberto
    if (saida != NULL) {
        fprintf(saida, "%s\n", buffer);
    }
}
//PEGA O PROXIMO TOKEN
void nextToken(void) {
    tokenAtual = obterToken();
    imprimirToken(tokenAtual, arquivoSaida);
}

/*
2. FUNCOES AUXILIARES DE APOIO AO PARSER
[X] Criar uma funcao que confere se o token atual e do tipo esperado e, se for, avanca para o proximo (senao, aciona o erro sintatico).
[X] Criar uma funcao que confere se o token atual e uma palavra reservada especifica (ex: "se", "enquanto", "fimalgoritmo").
[X] Criar uma funcao que confere se o token atual e um delimitador ou operador especifico (ex: '(', ')', ':', ',').
*/
//OLHA SE É O TOKEN ATUAL É O TIPO ESPERADO
int casaToken(TokenNome tipoEsperado){
    if (tokenAtual.type == tipoEsperado){
        nextToken();
        return 1;
    } else {
        char msg[50];
        sprintf(msg, "token do tipo %d", tipoEsperado);
        erroSintatico("NÃO É O TIPO ESPERADO");
        return 0;
    }
}
//VERIFICA SE O TOKEN ATUAL É UMA DAS PALAVRAS RESERVADAS MAPEADAS NA ETAPA 1
int checarPalavraReservada(){
    if(tokenAtual.type == TOKEN_KEYWORD){//SE FOR UMA PALAVRA DO TIPO PALAVRA RESERVADA
        nextToken();//PEGA O PROXIMO TOKEN
        return 1;
    }
    erroSintatico("Não palavra reservada");
    return 0; // CASO CONTRARIO, RETONA
}

// exige uma palavra reservada especifica (ex: "algoritmo", "inicio")
// e avanca. Se nao for ela, da erro sintatico.
void consumirKeyword(const char *palavra) {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, palavra) == 0) {
        nextToken();
    } else {
        char msg[100];
        sprintf(msg, "Esperado '%s'.", palavra);
        erroSintatico(msg);
    }
}

//VERIFICA SE O TOKEN ATUAL É UM DELIMITADOR OU OPERADOR ESPECIFICO
int checarDelimitadorOperador(Delimitadores delimitador) {
    if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == delimitador) {
        nextToken();
        return 1;
    }
    return 0;
}

/*
6. IMPLEMENTACAO DA GRAMATICA - EXPRESSOES
Expressao: id operacoes id
Operacoes: operacaoAritmetica | operacaoRelacional | operacaoLogica
Operação Aritmética: + | - | * | /
Operação Relacional: < | <= | == | > | >= | <>
Atribuição:  id '<-' (inteiro | float | caractere | logico)
Operações Lógicas: E | OU
*/

//Operação Aritmética: + | - | * | /
void operacaoAritmetica(){
    if (tokenAtual.type == TOKEN_OP_ARIT) {
        nextToken();
    } else {
        erroSintatico("Operador aritmetico (+, -, *, /) esperado.");
    }
}
//Operação Relacional: < | <= | == | > | >= | <>
void operacaoRelacional(){
    if (tokenAtual.type == TOKEN_OP_REL) {
        nextToken();
    } else {
        erroSintatico("Operador relacional esperado.");
    }
}
//Atribuição:  id '<-' (inteiro | float | caractere | logico)
void operacaoAtribuicao(){
    // Reconhece o identificador 'id'
    casaToken(TOKEN_ID);

    // Reconhece o operador de atribuição '<-'
    if (tokenAtual.type == TOKEN_OP_ATRIBUTION) {
        nextToken();
    } else {
        erroSintatico("Operador de atribuicao '<-' esperado.");
    }

    // Aceita um valor literal: inteiro, float, caractere (ID) ou booleano
    if (tokenAtual.type == TOKEN_NUM_INT) {
        casaToken(TOKEN_NUM_INT);
    } else if (tokenAtual.type == TOKEN_NUM_FLOAT) {
        casaToken(TOKEN_NUM_FLOAT);
    } else if (tokenAtual.type == TOKEN_BOOLEANO) {
        casaToken(TOKEN_BOOLEANO);
    } else if (tokenAtual.type == TOKEN_ID) { // Para representar 'caractere' ou ID
        casaToken(TOKEN_ID);
    } else {
        erroSintatico("Valor invalido para atribuicao.");
    }
}
//Operações Lógicas: E | OU
void operacaoLogico(){
    if (tokenAtual.type == TOKEN_OP_LOG) {
        nextToken();
    } else {
        erroSintatico("Operador logico esperado.");
    }
}
//Operacoes: operacaoAritmetica | operacaoRelacional | operacaoLogica
// tambem aceita MOD (a regra do se usa 'MOD')
void operacoes(){
   if (tokenAtual.type == TOKEN_OP_ARIT) {
        operacaoAritmetica();
    } else if (tokenAtual.type == TOKEN_OP_REL) {
        operacaoRelacional();
    } else if (tokenAtual.type == TOKEN_OP_LOG) {
        operacaoLogico();
    } else if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "MOD") == 0) {
        nextToken();
    } else {
        erroSintatico("Operacao invalida. Esperado operador aritmetico, relacional ou logico.");
    }
}

// o token atual e um operador (inclui MOD)?
int ehOperador() {
    return tokenAtual.type == TOKEN_OP_ARIT ||
           tokenAtual.type == TOKEN_OP_REL  ||
           tokenAtual.type == TOKEN_OP_LOG  ||
           (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "MOD") == 0);
}

// chamada: '(' (expressao (',' expressao)*)? ')'   (o id ja foi consumido)
void chamadaResto() {
    if (ehDelimitador(abreParenteses)) {
        nextToken();
    } else {
        erroSintatico("Esperado '(' na chamada.");
    }
    if (!ehDelimitador(fechaParenteses)) {
        expressao();
        while (ehDelimitador(virgula)) {
            nextToken();
            expressao();
        }
    }
    if (ehDelimitador(fechaParenteses)) {
        nextToken();
    } else {
        erroSintatico("Esperado ')' ao fechar chamada.");
    }
}

// operando: id | inteiro | real | booleano | '-' operando | '(' expressao ')'
// (antes so aceitava id, o que rejeitava "se (x > 0)", "para i de 1 ate 10", "x <- 5")
void operando() {
    if (tokenAtual.type == TOKEN_OP_ARIT && tokenAtual.attribute.op_arit == OP_MINUS) {
        nextToken();
        operando();
    } else if (ehDelimitador(abreParenteses)) {
        nextToken();
        expressao();
        if (ehDelimitador(fechaParenteses)) {
            nextToken();
        } else {
            erroSintatico("Esperado ')' ao fechar expressao.");
        }
    } else if (tokenAtual.type == TOKEN_ID) {
        variavel();
        if (ehDelimitador(abreParenteses)) {   // id(args): chamada de funcao
            chamadaResto();
        }
    } else if (tokenAtual.type == TOKEN_NUM_INT ||
               tokenAtual.type == TOKEN_NUM_FLOAT ||
               tokenAtual.type == TOKEN_BOOLEANO) {
        nextToken();
    } else {
        erroSintatico("Operando esperado (id, numero ou booleano).");
    }
}

//Expressao: operando (operacoes operando)*
// versao mais geral que "id operacoes id" do relatorio: aceita um operando sozinho
// (escreva(x), x <- 5) e tambem varias operacoes seguidas (a + b * c)
void expressao(){
    operando();
    while (ehOperador()) {
        operacoes();
        operando();
    }
}

// Operando de condicao do relatorio: (id | inteiro | real)
void operandoSimples() {
    if (tokenAtual.type == TOKEN_ID || tokenAtual.type == TOKEN_NUM_INT || tokenAtual.type == TOKEN_NUM_FLOAT) {
        nextToken();
    } else {
        erroSintatico("Esperado identificador, inteiro ou real.");
    }
}

// Condicional: se ( (id|inteiro|real) (operadorRelacional | 'MOD') (id|inteiro|real) ) ...
void condicaoSe() {
    operandoSimples();
    if (tokenAtual.type == TOKEN_OP_REL ||
        (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "MOD") == 0)) {
        nextToken();
    } else {
        erroSintatico("Esperado operador relacional ou 'MOD'.");
    }
    operandoSimples();
}

// Repeticao_enquanto: enquanto ( id operadorRelacional inteiro ) ...
void condicaoEnquanto() {
    casaToken(TOKEN_ID);
    operacaoRelacional();
    casaToken(TOKEN_NUM_INT);
}

/*
 * 5. IMPLEMENTAÇÃO DA GRAMÁTICA - COMANDOS
 */

// Função auxiliar para verificar se o token atual é um delimitador específico
int ehDelimitador(Delimitadores d) {
    return (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == d);
}

// [X] variavel -> id ('[' expressao ']')?
void variavel() {
    casaToken(TOKEN_ID);
    if (ehDelimitador(abreColchetes)) {
        nextToken(); // consome '['
        expressao();
        if (ehDelimitador(fechaColchetes)) {
            nextToken(); // consome ']'
        } else {
            erroSintatico("Esperado ']' apos indice do vetor.");
        }
    }
}

// [X] atribuicao -> variavel '<-' expressao
void atribuicao() {
    // a variavel ja foi consumida por comando() (atribuicao e chamada comecam com id)
    // so aceita o token de atribuicao <-
    if (tokenAtual.type == TOKEN_OP_ATRIBUTION) {
        nextToken(); // consome '<-'
    } else {
        erroSintatico("Operador de atribuicao '<-' esperado.");
    }
    expressao();
}

// [X] leitura -> leia '(' variavel ')'
void leitura() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "leia") == 0) {
        nextToken(); // consome 'leia'
    } else {
        erroSintatico("Esperado 'leia'.");
    }

    if (ehDelimitador(abreParenteses)) {
        nextToken(); // consome '('
    } else {
        erroSintatico("Esperado '(' apos 'leia'.");
    }

    casaToken(TOKEN_ID); // Leitura: leia ( id )

    if (ehDelimitador(fechaParenteses)) {
        nextToken(); // consome ')'
    } else {
        erroSintatico("Esperado ')' apos variavel em 'leia'.");
    }
}

// [X] escrita -> (escreva | escreval) '(' expressao (',' expressao)* ')'
void escrita() {
    if (tokenAtual.type == TOKEN_KEYWORD && 
       (strcmp(lexemaAtual, "escreva") == 0 || strcmp(lexemaAtual, "escreval") == 0)) {
        nextToken(); // consome 'escreva' ou 'escreval'
    } else {
        erroSintatico("Esperado 'escreva' ou 'escreval'.");
    }

    if (ehDelimitador(abreParenteses)) {
        nextToken(); // consome '('
    } else {
        erroSintatico("Esperado '(' em comando de escrita.");
    }

    expressao();

    while (ehDelimitador(virgula)) {
        nextToken(); // consome ','
        expressao();
    }

    if (ehDelimitador(fechaParenteses)) {
        nextToken(); // consome ')'
    } else {
        erroSintatico("Esperado ')' ao fechar comando de escrita.");
    }
}

// [X] condicional -> se '(' expressao ')' entao comando* (senao comando*)? fimse
void condicional() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "se") == 0) {
        nextToken(); // consome 'se'
    } else {
        erroSintatico("Esperado 'se'.");
    }

    if (ehDelimitador(abreParenteses)) {
        nextToken(); // consome '('
    } else {
        erroSintatico("Esperado '(' apos 'se'.");
    }

    condicaoSe(); // (id|inteiro|real) (opRel|MOD) (id|inteiro|real)

    if (ehDelimitador(fechaParenteses)) {
        nextToken(); // consome ')'
    } else {
        erroSintatico("Esperado ')' apos expressao condicional.");
    }

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "entao") == 0) {
        nextToken(); // consome 'entao'
    } else {
        erroSintatico("Esperado 'entao'.");
    }

    codigo(); // Bloco de comandos no ramo 'entao'

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "senao") == 0) {
        nextToken(); // consome 'senao'
        codigo();    // Bloco de comandos no ramo 'senao'
    }

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "fimse") == 0) {
        nextToken(); // consome 'fimse'
    } else {
        erroSintatico("Esperado 'fimse'.");
    }
}

// [X] repeticao_para -> para id de expressao ate expressao (passo expressao)? faca comando* fimpara
void repeticaoPara() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "para") == 0) {
        nextToken(); // consome 'para'
    } else {
        erroSintatico("Esperado 'para'.");
    }

    casaToken(TOKEN_ID);

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "de") == 0) {
        nextToken(); // consome 'de'
    } else {
        erroSintatico("Esperado 'de' no laco 'para'.");
    }

    casaToken(TOKEN_NUM_INT); // para id de inteiro ate inteiro

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "ate") == 0) {
        nextToken(); // consome 'ate'
    } else {
        erroSintatico("Esperado 'ate' no laco 'para'.");
    }

    casaToken(TOKEN_NUM_INT);

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "passo") == 0) {
        nextToken(); // consome 'passo'
        casaToken(TOKEN_NUM_INT); // passo: 'passo' inteiro
    }

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "faca") == 0) {
        nextToken(); // consome 'faca'
    } else {
        erroSintatico("Esperado 'faca' no laco 'para'.");
    }

    codigo();

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "fimpara") == 0) {
        nextToken(); // consome 'fimpara'
    } else {
        erroSintatico("Esperado 'fimpara'.");
    }
}

// [X] repeticao_enquanto -> enquanto '(' expressao ')' faca comando* fimenquanto
void repeticaoEnquanto() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "enquanto") == 0) {
        nextToken(); // consome 'enquanto'
    } else {
        erroSintatico("Esperado 'enquanto'.");
    }

    if (ehDelimitador(abreParenteses)) {
        nextToken(); // consome '('
    } else {
        erroSintatico("Esperado '(' apos 'enquanto'.");
    }

    condicaoEnquanto(); // id operadorRelacional inteiro

    if (ehDelimitador(fechaParenteses)) {
        nextToken(); // consome ')'
    } else {
        erroSintatico("Esperado ')' apos expressao do 'enquanto'.");
    }

    // a regra do enquanto nao tem 'faca'; aceita-se opcionalmente (VisuAlg)
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "faca") == 0) {
        nextToken(); // consome 'faca'
    }

    codigo();

    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "fimenquanto") == 0) {
        nextToken(); // consome 'fimenquanto'
    } else {
        erroSintatico("Esperado 'fimenquanto'.");
    }
}

// [X] retorno -> retorne expressao
void retorno() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "retorne") == 0) {
        nextToken(); // consome 'retorne'
    } else {
        erroSintatico("Esperado 'retorne'.");
    }

    // Retorne: logico | id operacaoAritmetica id
    if (tokenAtual.type == TOKEN_BOOLEANO) {
        nextToken();
    } else {
        casaToken(TOKEN_ID);
        operacaoAritmetica();
        casaToken(TOKEN_ID);
    }
}

// [X] comando -> leitura | escrita | condicional | repeticaoPara | repeticaoEnquanto | retorno | atribuicao
void comando() {
    if (tokenAtual.type == TOKEN_KEYWORD) {
        if (strcmp(lexemaAtual, "leia") == 0) {
            leitura();
        } else if (strcmp(lexemaAtual, "escreva") == 0 || strcmp(lexemaAtual, "escreval") == 0) {
            escrita();
        } else if (strcmp(lexemaAtual, "se") == 0) {
            condicional();
        } else if (strcmp(lexemaAtual, "para") == 0) {
            repeticaoPara();
        } else if (strcmp(lexemaAtual, "enquanto") == 0) {
            repeticaoEnquanto();
        } else if (strcmp(lexemaAtual, "retorne") == 0) {
            retorno();
        } else {
            erroSintatico("Comando nao reconhecido.");
        }
    } else if (tokenAtual.type == TOKEN_ID) {
        // atribuicao e chamada comecam com id. Consome a variavel e decide pelo proximo token.
        variavel();
        if (tokenAtual.type == TOKEN_OP_ATRIBUTION) {
            atribuicao();
        } else if (ehDelimitador(abreParenteses)) {
            chamadaResto();
        }
        // senao: chamada de procedimento sem parenteses (ex: linha_decorativa)
    } else {
        erroSintatico("Inicio de comando invalido.");
    }
}

/*
 [X] Decisão para diferenciar atribuição de chamada quando ambos começam com ID:
     A função codigo() consome uma sequência de comandos até encontrar palavras
     que sinalizam o encerramento de um bloco (ex: fimse, fimalgoritmo, etc.) ou o EOF.
*/
void codigo() {
    while (tokenAtual.type != TOKEN_EOF) {
        if (tokenAtual.type == TOKEN_KEYWORD) {
            // Se encontrar palavra que encerra bloco de comandos atual, interrompe o loop
            if (strcmp(lexemaAtual, "fimalgoritmo") == 0 ||
                strcmp(lexemaAtual, "senao") == 0 ||
                strcmp(lexemaAtual, "fimse") == 0 ||
                strcmp(lexemaAtual, "fimpara") == 0 ||
                strcmp(lexemaAtual, "fimenquanto") == 0 ||
                strcmp(lexemaAtual, "fimprocedimento") == 0 ||
                strcmp(lexemaAtual, "fimfuncao") == 0) {
                break;
            }
        }
        comando();
    }
}

/*
4. IMPLEMENTACAO DA GRAMATICA - DECLARACOES
tipo: inteiro | real | logico
Procedimentos: procedimento id abreParenteses  (id doisPontos tipo | virgula)+ fechaParenteses codigo fimprocedimento
Funções: funcao id abreParenteses (id doisPontos tipo | virgula)+ fechaParenteses doisPontos tipo inicio codigo fimfuncao

*/
void consumir(TokenNome tipoEsperado) {
    if (tokenAtual.type == tipoEsperado) {
        // nextToken() em vez de proximoToken(), para o token tambem ser impresso no .lex
        nextToken();
    } else {
        erroSintatico("Token inesperado.");
    }
}
// limite do vetor: caractere (uma letra) | inteiro | real. Devolve qual dos tres foi lido.
TokenNome limiteVetor() {
    TokenNome tipoLimite = tokenAtual.type;
    if (tipoLimite == TOKEN_NUM_INT || tipoLimite == TOKEN_NUM_FLOAT) {
        nextToken();
    } else if (tipoLimite == TOKEN_ID && strlen(lexemaAtual) == 1) {
        nextToken(); // caractere: [a-zA-Z]
    } else {
        erroSintatico("Limite de vetor invalido (esperado caractere, inteiro ou real).");
    }
    return tipoLimite;
}

// tipo: tipoBase | vetor '[' limite '..' limite ']' de tipoBase   (limites do mesmo tipo)
void tipo() {
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "vetor") == 0) {
        nextToken(); // consome 'vetor'
        if (ehDelimitador(abreColchetes)) nextToken(); else erroSintatico("Esperado '[' apos 'vetor'.");
        TokenNome t1 = limiteVetor();
        if (ehDelimitador(pontoPonto)) nextToken(); else erroSintatico("Esperado '..' nos limites do vetor.");
        TokenNome t2 = limiteVetor();
        if (t1 != t2) erroSintatico("Os dois limites do vetor devem ser do mesmo tipo.");
        if (ehDelimitador(fechaColchetes)) nextToken(); else erroSintatico("Esperado ']' apos limites do vetor.");
        consumirKeyword("de");
        tipoBase();
    } else {
        tipoBase();
    }
}

//tipoBase: inteiro | real | logico | caractere
void tipoBase() {
    if (tokenAtual.type == TOKEN_KEYWORD && 
       (strcmp(lexemaAtual, "inteiro") == 0 || 
        strcmp(lexemaAtual, "real") == 0 || 
        strcmp(lexemaAtual, "caractere") == 0 || 
        strcmp(lexemaAtual, "logico") == 0)) {
        nextToken(); // consome o tipo da variável
    } else {
        erroSintatico("Esperado um tipo valido (inteiro, real, caractere ou logico).");
    }
}

// Procedimentos: procedimento id abreParenteses  (id doisPontos tipo | virgula)+ fechaParenteses codigo fimprocedimento
void procedimento() {
    consumirKeyword("procedimento"); // confere a palavra exata
    consumir(TOKEN_ID);      // id do procedimento

    // sem parametros nao ha parenteses (teste ProcedimentosSemParametros)
    if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == abreParenteses) {
        consumir(TOKEN_DELIMITADORES);

        // Lista de parâmetros: (id doisPontos tipo | virgula)+
        do {
            if (tokenAtual.type == TOKEN_ID) {
                consumir(TOKEN_ID);

                // doisPontos
                if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == doisPontos) {
                    consumir(TOKEN_DELIMITADORES);
                    tipo();
                } else {
                    erroSintatico("Esperado ':' apos o identificador do parametro.");
                }
            } else if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == virgula) {
                consumir(TOKEN_DELIMITADORES);
            } else {
                erroSintatico("Parametro invalido na declaracao do procedimento.");
            }
        } while (tokenAtual.type == TOKEN_ID ||
                (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == virgula));

        // fechaParenteses
        if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == fechaParenteses) {
            consumir(TOKEN_DELIMITADORES);
        } else {
            erroSintatico("Esperado ')' na declaracao do procedimento.");
        }
    }

    // o teste do relatorio usa 'inicio' no procedimento (a regra nao tem), entao e opcional
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "inicio") == 0) {
        nextToken();
    }

    // Corpo do procedimento
    codigo();

    // fimprocedimento
    consumirKeyword("fimprocedimento"); // confere a palavra exata
}

//Funções: funcao id abreParenteses (id doisPontos tipo | virgula)+ fechaParenteses doisPontos tipo inicio codigo fimfuncao
void funcao() {
    consumirKeyword("funcao"); // confere a palavra exata
    consumir(TOKEN_ID);      // id da função

    // abreParenteses
    if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == abreParenteses) {
        consumir(TOKEN_DELIMITADORES);
    } else {
        erroSintatico("Esperado '(' na declaracao da funcao.");
    }

    // Lista de parâmetros
    do {
        if (tokenAtual.type == TOKEN_ID) {
            consumir(TOKEN_ID);
            
            if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == doisPontos) {
                consumir(TOKEN_DELIMITADORES);
                tipo();
            } else {
                erroSintatico("Esperado ':' apos o identificador do parametro.");
            }
        } else if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == virgula) {
            consumir(TOKEN_DELIMITADORES);
        } else {
            erroSintatico("Parametro invalido na declaracao da funcao.");
        }
    } while (tokenAtual.type == TOKEN_ID || 
            (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == virgula));

    // fechaParenteses
    if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == fechaParenteses) {
        consumir(TOKEN_DELIMITADORES);
    } else {
        erroSintatico("Esperado ')' na declaracao da funcao.");
    }

    // doisPontos para o tipo de retorno
    if (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == doisPontos) {
        consumir(TOKEN_DELIMITADORES);
        tipo(); // Tipo de retorno da função
    } else {
        erroSintatico("Esperado ':' indicando o tipo de retorno da funcao.");
    }

    // Bloco de inicio e codigo
    consumirKeyword("inicio"); // confere a palavra exata
    codigo();

    // fimfuncao
    consumirKeyword("fimfuncao"); // confere a palavra exata
}
// procedimentos e funcoes: o relatorio nao diz onde ficam; os testes mostram antes do 'var'.
// Aceita antes e depois do bloco var.
void declaracoesRotinas() {
    while (tokenAtual.type == TOKEN_KEYWORD &&
           (strcmp(lexemaAtual, "procedimento") == 0 || strcmp(lexemaAtual, "funcao") == 0)) {
        if (strcmp(lexemaAtual, "procedimento") == 0) {
            procedimento();
        } else {
            funcao();
        }
    }
}
/*
3. IMPLEMENTACAO DA GRAMATICA - ESTRUTURA GERAL
algoritmoAux: 'algoritmo' aspas cadeia aspas var inicio fimalgoritmo

*/
//algoritmo: 'algoritmo' aspas cadeia aspas var inicio fimalgoritmo
// algoritmo: 'algoritmo' TOKEN_ID (que já é a string entre aspas) [var] inicio codigo fimalgoritmo
void algoritmo() {
    // 1. Reconhece a palavra-chave 'algoritmo'
    consumirKeyword("algoritmo"); // confere a palavra exata

    // 2. Reconhece o nome do algoritmo
    // Como o Scanner lê "nome_do_algoritmo" entre aspas e já retorna TOKEN_ID,
    // apenas consumimos esse TOKEN_ID diretamente.
    if (tokenAtual.type == TOKEN_ID) {
        nextToken();
    } else {
        erroSintatico("Esperado nome do algoritmo (cadeia de texto entre aspas).");
    }

    declaracoesRotinas();

    // 3. Bloco var (OBRIGATORIO no relatorio: var: 'var' id_lista)
    consumirKeyword("var");
    if (tokenAtual.type != TOKEN_ID) {
        erroSintatico("Esperado identificador apos 'var'.");
    }

    // Lê as declarações de variáveis até encontrar 'inicio'
    while (tokenAtual.type == TOKEN_ID) {
        idLista();

        if (ehDelimitador(doisPontos)) {
            nextToken(); // consome ':'
            tipo();
        } else {
            erroSintatico("Esperado ':' apos a lista de identificadores.");
        }
    }

    declaracoesRotinas();

    // 4. Bloco inicio
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "inicio") == 0) {
        nextToken(); // consome 'inicio'
    } else {
        erroSintatico("Esperado 'inicio' no algoritmo.");
    }

    // 5. Corpo de comandos
    codigo();

    // 6. Bloco fimalgoritmo
    if (tokenAtual.type == TOKEN_KEYWORD && strcmp(lexemaAtual, "fimalgoritmo") == 0) {
        nextToken(); // consome 'fimalgoritmo'
    } else {
        erroSintatico("Esperado 'fimalgoritmo'.");
    }
}
// [X] idLista -> id (',' id)*
void idLista() {
    // Reconhece o primeiro identificador
    if (tokenAtual.type == TOKEN_ID) {
        nextToken();
    } else {
        erroSintatico("Esperado identificador na lista de variaveis.");
    }

    // Enquanto houver vírgula, consome a vírgula e o próximo identificador
    while (tokenAtual.type == TOKEN_DELIMITADORES && tokenAtual.attribute.delimitadores == virgula) {
        nextToken(); // consome ','
        if (tokenAtual.type == TOKEN_ID) {
            nextToken(); // consome o próximo ID
        } else {
            erroSintatico("Esperado identificador apos ',' na lista de variaveis.");
        }
    }
}
/*
7. TRATAMENTO DE ERROS SINTATICOS
[X] Interceptar qualquer token que nao bata com o que a gramatica esperava naquele ponto da analise.
[X] Exibir a mensagem exata "ERRO SINTATICO", informando o token incorreto e a linha do codigo fonte correspondente.
[X] Abortar imediatamente a execucao do programa (exit) apos identificar o erro.
*/
// mostra o token encontrado (o item 7 pede o token incorreto)
void erroSintatico(const char *msg){
    const char *nomeTipo;
    switch (tokenAtual.type) {
        case TOKEN_EOF:            nomeTipo = "fim de arquivo";        break;
        case TOKEN_ID:             nomeTipo = "identificador";         break;
        case TOKEN_NUM_INT:        nomeTipo = "numero inteiro";        break;
        case TOKEN_NUM_FLOAT:      nomeTipo = "numero real";           break;
        case TOKEN_OP_REL:         nomeTipo = "operador relacional";   break;
        case TOKEN_OP_ARIT:        nomeTipo = "operador aritmetico";   break;
        case TOKEN_OP_LOG:         nomeTipo = "operador logico";       break;
        case TOKEN_OP_ATRIBUTION:  nomeTipo = "operador de atribuicao"; break;
        case TOKEN_KEYWORD:        nomeTipo = "palavra reservada";     break;
        case TOKEN_DELIMITADORES:  nomeTipo = "delimitador";           break;
        case TOKEN_BOOLEANO:       nomeTipo = "booleano";              break;
        default:                   nomeTipo = "desconhecido";          break;
    }

    if (tokenAtual.type == TOKEN_KEYWORD) {
        fprintf(stderr, "ERRO SINTATICO na linha %d: %s (token encontrado: %s '%s')\n",
                tokenAtual.line, msg, nomeTipo, lexemaAtual);
    } else {
        fprintf(stderr, "ERRO SINTATICO na linha %d: %s (token encontrado: %s)\n",
                tokenAtual.line, msg, nomeTipo);
    }

    fecharAnalisador();
    
    exit(1);
}

/*
8. INTEGRACAO FINAL E ENTREGA
[ ] Garantir que o analisador lexico e o sintatico rodem juntos, no mesmo programa (o enunciado exige a entrega dos dois funcionando em conjunto).
[ ] Testar contra os proprios exemplos do Anexo I fornecidos pela professora.
[ ] Utilizar os nomes de modulos sugeridos no documento (nextToken / obterToken).
*/

//FUNÇÃO INICIAL PARA CONSEGUIR TESTAR/RODAR DEPOIS
void analisadorSintatico(FILE *arq) {
    iniciarAnalisador(arq);

    nextToken(); // Carrega o primeiro token (lookahead)
    algoritmo();

    // nada pode vir depois de 'fimalgoritmo'
    if (tokenAtual.type != TOKEN_EOF) {
        erroSintatico("Token inesperado apos 'fimalgoritmo'.");
    }

    // while (tokenAtual.type != TOKEN_EOF) {
    //     printf("[Parser Lookahead] Linha %d | Type: %d\n", tokenAtual.line, tokenAtual.type);
    //     nextToken();
    // }

    fecharAnalisador();
    printf("compilou\n");

    // imprime a tabela de simbolos ao final
    imprimirTabelaSimbolos(arquivoSaida);
}

// esse main() aqui é só pra eu conseguir testar se o lexico tá funcionando.
// ainda não é a versão final (falta formatar do jeito que o item 5 pede
// e salvar num arquivo de log)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return 1;
    }

    FILE *arq = fopen(argv[1], "r");
    if (arq == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o arquivo de entrada: %s\n", argv[1]);
        return 1;
    }

    // Tenta criar o arquivo .lex, mas se o sistema negar o acesso, 
    // ele NAO trava e continua imprimindo apenas no terminal (stdout)
    char nomeSaida[256];
    sprintf(nomeSaida, "%s.lex", argv[1]);
    arquivoSaida = fopen(nomeSaida, "w");

    if (arquivoSaida == NULL) {
        // Apenas avisa no terminal sem dar crash/Access Denied
        fprintf(stderr, "[AVISO] Nao foi possivel criar '%s' (Acesso Negado no disco). Rodando apenas no terminal.\n\n", nomeSaida);
    }

    analisadorSintatico(arq);

    if (arquivoSaida != NULL) {
        fclose(arquivoSaida);
    }

    return 0;
}