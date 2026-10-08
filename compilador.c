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
    TOKEN_KEYWORD
} TokenNome;

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
} OpAritType;


typedef enum {
    OP_E, // E
    OP_OU, // OU
} OpLogType;


typedef enum {
    OP_AT, // E
} OpAtribuition;

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


/*
 * TODO LIST - ETAPA 2: ANALISADOR LÉXICO (MINIVISUALG)

 * 1. GERENCIAMENTO DE ARQUIVO E ESTADO GLOBAL
 * [X] Configurar o ponteiro FILE para leitura do código fonte.
 * [X] Criar a variável global de controle de linha atual (inicializada em 1).
 * [X] Implementar as funções de ciclo de vida: abrir arquivo, checar EOF (fim de arquivo) e fechar arquivo.
*/
FILE *fonte = NULL; // Ponteiro para o arquivo de entrada
int linhaAtual = 1; // Contador de linha atual

void iniciarAnalisador(FILE *arquivo) {
    fonte = arquivo;
    linhaAtual = 1;
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

//Limpeza de Entrada
Token proximoToken() {
    Token token;
    int c;

    while ((c = fgetc(fonte)) != EOF) {
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
    if (isalpha(c) || c == '_') {
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

        if (peek() == '.') {
            ehReal = 1;
            lexema[i++] = (char)fgetc(fonte);

            while (i < MAX_LEXEMA - 1 && isdigit(peek())) {
                lexema[i++] = (char)fgetc(fonte);
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
            token.type = TOKEN_OP_REL;
            token.attribute.op_code = OP_ASSIGN;
            return token;
        }
        if (peek() == '=') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_code = OP_LE;
            return token;
        }
        if (peek() == '>') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_code = OP_NE;
            return token;
        }
        token.type = TOKEN_OP_REL;
        token.attribute.op_code = OP_LT;
        return token;
    }

    // Operadores >= e >
    if (c == '>') {
        if (peek() == '=') {
            fgetc(fonte);
            token.type = TOKEN_OP_REL;
            token.attribute.op_code = OP_GE;
            return token;
        }
        token.type = TOKEN_OP_REL;
        token.attribute.op_code = OP_GT;
        return token;
    }

    // Operador =
    if (c == '=') {
        token.type = TOKEN_OP_REL;
        token.attribute.op_code = OP_EQ;
        return token;
    }

    
    // Operadores e Delimitadores em geral
    if (c == '(' || c == ')' || c == ',' || c == ':' || 
        c == '[' || c == ']' || c == '+' || c == '-' || 
        c == '*' || c == '/' || c == '\\') {
        token.type = TOKEN_OP_REL;
        token.attribute.op_code = c; // Guarda o próprio caractere como código!
        return token;
    }

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
    // lista com todas as palavras reservadas do relatório da etapa 1
    static const char *reservadas[] = {
        "algoritmo", "var", "inicio", "fimalgoritmo",
        "caractere", "inteiro", "real", "logico",
        "verdadeiro", "falso",
        "leia", "escreva", "escreval",
        "se", "entao", "senao", "fimse",
        "para", "de", "ate", "passo", "faca", "fimpara",
        "enquanto", "fimenquanto",
        "vetor",
        "procedimento", "fimprocedimento",
        "funcao", "fimfuncao", "retorne",
        "MOD", "E", "OU"
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

// isso aqui ainda não é a tabela de símbolos de verdade que o projeto vai
// precisar depois -- é só um contador provisório pra função de cima ter
// algum número pra colocar no table_index sem dar erro
int inserirTabelaSimbolos(const char *lexema) {
    static int proximoIndice = 0;
    (void)lexema; // por enquanto não uso o lexema pra nada, só pra não sobrar warning de parametro nao usado
    return proximoIndice++;
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
            sprintf(buffer, "%d# OPERADOR_DELIMITADOR | %d", t.line, t.attribute.op_code);
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

//VERIFICA SE O TOKEN ATUAL É UM DELIMITADOR OU OPERADOR ESPECIFICO
int checarDelimitadorOperador(OpRelType opDe){//
}


/*
6. IMPLEMENTACAO DA GRAMATICA - EXPRESSOES
[X] expressao -> expressao_e (OU expressao_e)*
[X] expressao_e -> expressao_rel (E expressao_rel)*
[X] expressao_rel -> expressao_arit (opReal expressao_arit)?
[X] expressao_arit -> termo (('+' | '-') termo)*
[X] termo -> fator (('*' | '/' | '\' | MOD) fator)*
[X] fator -> '(' expressao ')'
           | '-' fator
           | id ('[' expressao ']' | '(' (expressao (',' expressao)*)? ')')?
           | num_int | num_real | cadeia | verdadeiro | falso
*/
// fator -> '(' expressao ')' | '-' fator | id ('[' expressao ']' | '(' (expressao (',' expressao)*)? ')')? | num_int | num_real | cadeia | verdadeiro | falso
void fator() {//
}

// termo -> fator (('*' | '/' | '\' | MOD) fator)*
void termo() {//
}

// expressao_arit -> termo (('+' | '-') termo)*
void expressaoAritmetica() {//
}

// expressao_rel -> expressao_arit (opReal expressao_arit)?
void expressaoRelacional() {//
}

// expressao -> expressao_e (OU expressao_e)*
void expressao() {//
}

/*
5. IMPLEMENTACAO DA GRAMATICA - COMANDOS
[X] comando -> atribuicao | leitura | escrita | condicional
             | repeticao_para | repeticao_enquanto | chamada | retorno
[X] atribuicao -> variavel '<-' expressao
[X] variavel -> id ('[' expressao ']')?
[X] leitura -> leia '(' variavel ')'
[X] escrita -> (escreva | escreval) '(' expressao (',' expressao)* ')'
[X] condicional -> se '(' expressao ')' entao comando* (senao comando*)? fimse
[X] repeticao_para -> para id de expressao ate expressao (passo expressao)? faca comando* fimpara
[X] repeticao_enquanto -> enquanto '(' expressao ')' faca comando* fimenquanto
[X] chamada -> id ('(' (expressao (',' expressao)*)? ')')?
[X] retorno -> retorne expressao
[X] Decidir como diferenciar atribuicao de chamada quando os dois comecam com id (olhar o que vem depois do id: '<-', '[', '(' ou nenhum desses).
*/
// Protótipos das funções de comandos e expressões


// [X] variavel -> id ('[' expressao ']')?
void variavel() {//
}

// [X] atribuicao -> variavel '<-' expressao
void atribuicao() {//
}

// [X] leitura -> leia '(' variavel ')'
void leitura() {//
}

// [X] escrita -> (escreva | escreval) '(' expressao (',' expressao)* ')'
void escrita() {//
}

// [X] condicional -> se '(' expressao ')' entao comando* (senao comando*)? fimse
void condicional() {//
}

// [X] repeticao_para -> para id de expressao ate expressao (passo expressao)? faca comando* fimpara
void repeticaoPara() {//
}

// [X] repeticao_enquanto -> enquanto '(' expressao ')' faca comando* fimenquanto
void repeticaoEnquanto() {//
}

// [X] chamada -> id ('(' (expressao (',' expressao)*)? ')')?
void chamada() {//
}

// [X] retorno -> retorne expressao
void retorno() {//
}

/*
[X] Decisão para diferenciar atribuição de chamada quando ambos começam com ID:
*/
void comando() {//
}

/*
4. IMPLEMENTACAO DA GRAMATICA - DECLARACOES
[X] declaracao_var -> var declaracao_lista+
[X] declaracao_lista -> id_lista ':' tipo
[X] id_lista -> id (',' id)*
[X] tipo -> tipo_base | vetor '[' num_int '..' num_int ']' de tipo_base
[X] tipo_base -> inteiro | real | caractere | logico
[X] declaracao_procedimento -> procedimento id ('(' parametros ')')? inicio comando* fimprocedimento
[X] declaracao_funcao -> funcao id '(' parametros? ')' ':' tipo_base inicio comando* fimfuncao
[X] parametros -> parametro (',' parametro)*
[X] parametro -> id ':' tipo_base
*/
// tipo_base -> inteiro | real | caractere | logico
void tipoBase() { //
}

// tipo -> tipo_base | vetor '[' num_int '..' num_int ']' de tipo_base
void tipo() {//
}

void idLista() {//
}

// [x] declaracao_lista -> id_lista ':' tipo
void declaracaoLista() {//
}

// parametro -> id ':' tipo_base
void parametro() {//
}

// parametros -> parametro (',' parametro)*
void parametros() {//
}

// declaracao_var -> var declaracao_lista+
void declaracaoVar() {//
}

// declaracao_procedimento -> procedimento id ('(' parametros ')')? inicio comando* fimprocedimento
void declaracaoProcedimento() {//
}

// declaracao_funcao -> funcao id '(' parametros? ')' ':' tipo_base inicio comando* fimfuncao
void declaracaoFuncao() { // 'fimfuncao'
}
/*
3. IMPLEMENTACAO DA GRAMATICA - ESTRUTURA GERAL
[X] algoritmo -> algoritmo cadeia declaracao* inicio comando* fimalgoritmo
[ ] declaracao -> declaracao_var | declaracao_procedimento | declaracao_funcao
*/
// algoritmo -> algoritmo cadeia declaracao* inicio comando* fimalgoritmo
void algoritmo() {
    //
}
void declaracao() {
    //
}
/*
7. TRATAMENTO DE ERROS SINTATICOS
[X] Interceptar qualquer token que nao bata com o que a gramatica esperava naquele ponto da analise.
[X] Exibir a mensagem exata "ERRO SINTATICO", informando o token incorreto e a linha do codigo fonte correspondente.
[X] Abortar imediatamente a execucao do programa (exit) apos identificar o erro.
*/
void erroSintatico(const char *msg){
    fprintf(stderr, "ERRO SINTATICO na linha %d: %s\n", tokenAtual.line, msg);

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

    // while (tokenAtual.type != TOKEN_EOF) {
    //     printf("[Parser Lookahead] Linha %d | Type: %d\n", tokenAtual.line, tokenAtual.type);
    //     nextToken();
    // }

    fecharAnalisador();
    printf("compilou\n");
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
        fprintf(stderr, "Nao foi possivel abrir o arquivo: %s\n", argv[1]);
        return 1;
    }

    // Criar arquivo de saída com a extensão .lex
    char nomeSaida[256];
    sprintf(nomeSaida, "%s.lex", argv[1]);
    arquivoSaida = fopen(nomeSaida, "w");

    analisadorSintatico(arq);

    if (arquivoSaida != NULL) {
        fclose(arquivoSaida);
    }

    return 0;
}