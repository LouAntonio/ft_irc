#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// Macros para cores
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

// Struct Request
typedef struct s_Request
{
    std::vector<std::string> args;
    std::string command;
    bool invalidMessage;
} Request;

Request splitRequest(std::string req)
{
    Request request;
    request.invalidMessage = false;
    size_t i = 0;
    size_t j = 0;

    if (req[i] == ' ' || !req[i]) {
        request.invalidMessage = true;
        return (request);
    }
    j = i;
    while (req[i])
    {
        if (req[i] == ' ')
        {
            if (req[i + 1] == ' ') {
                request.invalidMessage = true;
                return (request);
            }
            request.args.push_back(req.substr(j, i - j));
            while (req[i] == ' ')
                i++;
            j = i;
        }
        if (req[i] == ':')
        {
            if (req[i - 1] != ' ') {
                request.invalidMessage = true;
                return (request);
            }
            request.args.push_back(req.substr(i + 1, req.length() - i));
            request.command = request.args[0];
            request.args.erase(request.args.begin());
            return (request);
        }
        i++;
    }

    if (i && req[j])
        request.args.push_back(req.substr(j, i - j));
    request.command = request.args[0];
    request.args.erase(request.args.begin());
    return (request);
}

void printRequest(const std::string& input, const Request& req)
{
    std::cout << CYAN << "========================================" << RESET << std::endl;
    std::cout << YELLOW << "Input: " << RESET << "\"" << input << "\"" << std::endl;
    
    if (req.invalidMessage) {
        std::cout << RED << "❌ MENSAGEM INVÁLIDA!" << RESET << std::endl;
        return;
    }
    
    std::cout << GREEN << "✓ Válida" << RESET << std::endl;
    std::cout << BLUE << "Comando: " << RESET << "\"" << req.command << "\"" << std::endl;
    std::cout << MAGENTA << "Argumentos (" << req.args.size() << "):" << RESET << std::endl;
    
    for (size_t i = 0; i < req.args.size(); i++) {
        std::cout << "  [" << i << "] = \"" << req.args[i] << "\"" << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
    std::cout << GREEN << "╔════════════════════════════════════════╗" << RESET << std::endl;
    std::cout << GREEN << "║   TESTE DE PARSING IRC - _splitRequest ║" << RESET << std::endl;
    std::cout << GREEN << "╚════════════════════════════════════════╝" << RESET << std::endl;
    std::cout << std::endl;

    // Teste 1: Comando simples
    std::cout << CYAN << "TESTE 1: Comando simples (NICK)" << RESET << std::endl;
    printRequest("NICK deezNuts69", splitRequest("NICK deezNuts69"));

    // Teste 2: Comando com múltiplos argumentos
    std::cout << CYAN << "TESTE 2: USER com múltiplos args" << RESET << std::endl;
    printRequest("USER deez 0 * realname", splitRequest("USER deez 0 * realname"));

    // Teste 3: Comando com trailing (dois pontos)
    std::cout << CYAN << "TESTE 3: PRIVMSG com trailing" << RESET << std::endl;
    printRequest("PRIVMSG #channel :Hello world, how :are you?", 
                 splitRequest("PRIVMSG #channel :Hello world, how :are you?"));

    // Teste 4: USER completo com trailing
    std::cout << CYAN << "TESTE 4: USER com nome completo" << RESET << std::endl;
    printRequest("USER deez * * :Deez Nuts", splitRequest("USER deez * * :Deez Nuts"));

    // Teste 5: JOIN
    std::cout << CYAN << "TESTE 5: JOIN canal" << RESET << std::endl;
    printRequest("JOIN #general", splitRequest("JOIN #general"));

    // Teste 6: TOPIC com descrição
    std::cout << CYAN << "TESTE 6: TOPIC com descrição longa" << RESET << std::endl;
    printRequest("TOPIC #general :This is the general channel for everyone!", 
                 splitRequest("TOPIC #general :This is the general channel for everyone!"));

    // Teste 7: PASS
    std::cout << CYAN << "TESTE 7: PASS senha" << RESET << std::endl;
    printRequest("PASS mySecretPassword123", splitRequest("PASS mySecretPassword123"));

    // Teste 8: KICK com razão
    std::cout << CYAN << "TESTE 8: KICK com razão" << RESET << std::endl;
    printRequest("KICK #channel baduser :Breaking the rules", 
                 splitRequest("KICK #channel baduser :Breaking the rules"));

    // Teste 9: QUIT com mensagem
    std::cout << CYAN << "TESTE 9: QUIT com mensagem" << RESET << std::endl;
    printRequest("QUIT :Goodbye everyone!", splitRequest("QUIT :Goodbye everyone!"));

    // Teste 10: MODE
    std::cout << CYAN << "TESTE 10: MODE" << RESET << std::endl;
    printRequest("MODE deezNuts69 +i", splitRequest("MODE deezNuts69 +i"));

    std::cout << RED << "═══════════════════════════════════════" << RESET << std::endl;
    std::cout << RED << "        TESTES DE MENSAGENS INVÁLIDAS" << RESET << std::endl;
    std::cout << RED << "═══════════════════════════════════════" << RESET << std::endl;
    std::cout << std::endl;

    // Teste INVÁLIDO 1: Espaço no início
    std::cout << CYAN << "TESTE INVÁLIDO 1: Espaço no início" << RESET << std::endl;
    printRequest(" NICK test", splitRequest(" NICK test"));

    // Teste INVÁLIDO 2: Dois espaços seguidos
    std::cout << CYAN << "TESTE INVÁLIDO 2: Dois espaços seguidos" << RESET << std::endl;
    printRequest("NICK  test", splitRequest("NICK  test"));

    // Teste INVÁLIDO 3: Dois pontos sem espaço antes
    std::cout << CYAN << "TESTE INVÁLIDO 3: ':' sem espaço antes" << RESET << std::endl;
    printRequest("PRIVMSG#channel:message", splitRequest("PRIVMSG#channel:message"));

    // Teste INVÁLIDO 4: String vazia
    std::cout << CYAN << "TESTE INVÁLIDO 4: String vazia" << RESET << std::endl;
    printRequest("", splitRequest(""));

    std::cout << GREEN << "╔════════════════════════════════════════╗" << RESET << std::endl;
    std::cout << GREEN << "║          TESTES FINALIZADOS!           ║" << RESET << std::endl;
    std::cout << GREEN << "╚════════════════════════════════════════╝" << RESET << std::endl;

    return 0;
}